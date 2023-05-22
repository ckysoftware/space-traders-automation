#include <ctime>
#include <chrono>
#include <thread>
#include <unordered_set>

#include "spdlog/spdlog.h"
#include <fmt/core.h>

#include "ship_auto.h"
#include "../data_layer/error.h"

using namespace schema;
using namespace automation::ship;

// ----------------------------------------------------------------
// Hardcoded constants for the time being
const std::string AsteroidFieldWaypoint = "X1-VS75-67965Z";
const std::string contractItem = "PLATINUM_ORE";
const std::string contractWaypoint = "X1-VS75-70500X";
const std::string contractID = "clhw9qowb0139s60dm28j6y4p";
const std::unordered_set<std::string> notForSale = {"ANTIMATTER", contractItem};
// ----------------------------------------------------------------

ShipAutomator::ShipAutomator(schema::Ship &ship, dal::DataAccessLayer &DALInstance)
{
    p_ship = &ship;
    p_DALInstance = &DALInstance;
    status = TO_MINE;
    toDeliver = false;
    targetWaypoint = "";
}

void ShipAutomator::start()
{
    log("Starting ship automator...");

    while (true)
    {
        switch (status)
        {
        case TO_MINE:
            if (orbit())
            {
                status = IN_ORBIT;
            }
            break;
        case IN_ORBIT:
            if (mine())
            {
                status = FULL;
            }
            break;
        case TO_NAVIGATE:
            if (navigate())
            {
                targetWaypoint = "";
                if (toDeliver)
                {
                    status = TO_DELIVER;
                }
                else
                {
                    status = TO_MINE;
                }
            }
            break;
        case TO_DELIVER:
            while (!dock())
            {
                sleep(1);
            }
            if (deliverContract())
            {
                setTargetWaypoint(AsteroidFieldWaypoint);
            }
            break;
        case FULL:
            if (dock())
            {
                status = TO_SELL;
            }
            break;
        case TO_SELL:
            if (sell())
            {
                if (toDeliver)
                {
                    setTargetWaypoint(contractWaypoint);
                }
                else
                {
                    status = TO_MINE;
                }
            }
            break;
        case TEMP_IN_TRANSIT:
            status = TO_MINE;
            break;
        case TEMP_ON_EXTRACT_CD:
            status = TO_MINE;
            break;
        }
    }
}

std::string ShipAutomator::getShipSymbol()
{
    return p_ship->symbol;
}

void ShipAutomator::updateCargo(Cargo cargo)
{
    p_ship->cargo = cargo;
}

void ShipAutomator::updateCargo()
{
    // fetch current cargo information from the database
    p_ship->cargo = p_DALInstance->getShipCargo(p_ship->symbol);
}

bool ShipAutomator::mine()
{
    // return true if the cargo is full
    log("Mining...");

    try
    {
        ExtractResponse response = p_DALInstance->mine(p_ship->symbol);
        log(fmt::format("Yield = {}, CD = {}, Cargo = {}", response.yield.printStat(), response.cooldownSeconds, response.cargo.printStat()));

        updateCargo(response.cargo);
        if (p_ship->cargo.isFull())
        {
            return true;
        }
        else
        {
            Status prevStatus = status;
            status = TEMP_ON_EXTRACT_CD;
            sleep(response.cooldownSeconds);
            status = prevStatus;
        }
    }
    catch (error::ExtractCooldownException &e)
    {
        handleExtractCooldownError(e);
    }
    catch (error::InTransitException &e)
    {
        handleInTransitError(e);
    }
    catch (error::FullCargoException &e)
    {
        handleFullCargoError(e);
        return true;
    }
    catch (error::ExtractInvalidWaypointException &e)
    {
        handleExtractInvalidWaypointError(e);
    }
    return false;
}

bool ShipAutomator::sell()
{
    log("Selling...");

    try
    {
        updateCargo();
        for (auto &item : p_ship->cargo.inventory)
        {
            if (notForSale.count(item.symbol) > 0)
            {
                log(fmt::format("Found not for sale item {}, skipping...", item.symbol));
                continue;
            }

            SellResponse response = p_DALInstance->sell(p_ship->symbol, item.symbol, item.units);
            log(fmt::format("Sold {}x {} for {}@{}.", response.units, response.tradeSymbol, response.totalPrice, response.pricePerUnit));
            // updateCargo(response.cargo);  // will invalidate iterators, let's see if we need to update the cargo each time later
        }
        updateCargo();
    }
    catch (error::InTransitException &e)
    {
        handleInTransitError(e);
        return false;
    }
    if (p_ship->cargo.isFull())
    {
        toDeliver = true;
    } else {
        if (toDeliver) {
            log("toDeliver is true but cargo is not full, this should not happen.", spdlog::level::warn);
        }
        toDeliver = false;
    }
    return true;
}

bool ShipAutomator::dock()
{
    log("Docking...");
    try
    {
        p_DALInstance->dock(p_ship->symbol);
        log("Docked.");
        return true;
    }
    catch (error::InTransitException &e)
    {
        handleInTransitError(e);
    }
    return false;
}

bool ShipAutomator::orbit()
{
    log("Orbiting...");
    try
    {
        p_DALInstance->orbit(p_ship->symbol);
        log("Orbited.");
        return true;
    }
    catch (error::InTransitException &e)
    {
        handleInTransitError(e);
    }
    return false;
}

bool ShipAutomator::refuel()
{
    log("Refueling...");
    try
    {
        p_DALInstance->refuel(p_ship->symbol);
        log("Refueled.");
        return true;
    }
    catch (error::InTransitException &e)
    {
        handleInTransitError(e);
    }
    return false;
}

bool ShipAutomator::navigate()
{
    log(fmt::format("Navigating to {}...", targetWaypoint));
    try
    {
        NavResponse response = p_DALInstance->navigate(p_ship->symbol, targetWaypoint);
        int ETA = response.nav.route.getETA();
        log(fmt::format("Fuel left: {}. ETA: {} seconds.", response.fuel.printStat(), ETA));
        sleep(ETA);
        log(fmt::format("Navgiated to {}.", targetWaypoint));
        return true;
    }
    catch (error::InTransitException &e)
    {
        handleInTransitError(e);
    }
    catch (error::NavigateSameLocationException &e)
    {
        handleNavigateSameLocationError(e);
        return true;
    }
    catch (error::NavigateInsufficientFuelException &e)
    {
        handleNavigateInsufficientFuelError(e);
    }
    return false;
}

bool ShipAutomator::deliverContract()
{
    log(fmt::format("Delivering contract {}...", contractID));
    try
    {
        updateCargo();
        for (auto &item : p_ship->cargo.inventory)
        {
            if (item.symbol == contractItem)
            {
                log(fmt::format("Delivering {}x {}...", item.units, item.symbol));
                p_DALInstance->deliverContract(contractID, p_ship->symbol, item.symbol, item.units);
                log(fmt::format("Delivered {}x {}.", item.units, item.symbol));
                // updateCargo(response.cargo);  // will invalidate iterators, let's see if we need to update the cargo each time later
            }
        }
        updateCargo();
    }
    catch (error::InTransitException &e)
    {
        handleInTransitError(e);
        return false;
    }

    toDeliver = false;
    return true;
}

void ShipAutomator::setTargetWaypoint(std::string waypointSymbol)
{
    targetWaypoint = waypointSymbol;
    status = TO_NAVIGATE;
}

void ShipAutomator::sleep(int seconds)
{
    log(fmt::format("Sleeping for {} seconds...", seconds));
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    log(fmt::format("Waking up from sleep after {} seconds.", seconds));
}

void ShipAutomator::log(const std::string &message, spdlog::level::level_enum level)
{
    std::string formattedMessage = fmt::format("{}: {}", p_ship->symbol, message);
    switch (level)
    {
    case spdlog::level::trace:
        spdlog::trace(formattedMessage);
        break;
    case spdlog::level::debug:
        spdlog::debug(formattedMessage);
        break;
    case spdlog::level::info:
        spdlog::info(formattedMessage);
        break;
    case spdlog::level::warn:
        spdlog::warn(formattedMessage);
        break;
    case spdlog::level::err:
        spdlog::error(formattedMessage);
        break;
    case spdlog::level::critical:
        spdlog::critical(formattedMessage);
        break;
    }
}

void ShipAutomator::handleInTransitError(const error::InTransitException &e)
{
    Status prevStatus = status;
    log(e.what());
    status = TEMP_IN_TRANSIT;
    sleep(e.getSecondsToArrival());
    status = prevStatus;
}

void ShipAutomator::handleExtractCooldownError(const error::ExtractCooldownException &e)
{
    Status prevStatus = status;
    log(e.what());
    status = TEMP_ON_EXTRACT_CD;
    sleep(e.getCooldown());
    status = prevStatus;
}

void ShipAutomator::handleFullCargoError(const error::FullCargoException &e)
{
    log(e.what());
}

void ShipAutomator::handleExtractInvalidWaypointError(const error::ExtractInvalidWaypointException &e)
{
    log(e.what());
    setTargetWaypoint(AsteroidFieldWaypoint);
}

void ShipAutomator::handleNavigateSameLocationError(const error::NavigateSameLocationException &e)
{
    log(e.what());
}

void ShipAutomator::handleNavigateInsufficientFuelError(const error::NavigateInsufficientFuelException &e)
{
    log(e.what());
    // TODO no error handling here
    while (!dock())
    {
        sleep(1);
    }
    while (!refuel())
    {
        sleep(1);
    }
}
