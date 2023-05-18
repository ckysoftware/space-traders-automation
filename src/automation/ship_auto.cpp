#include <ctime>
#include <chrono>
#include <thread>
#include <unordered_set>

#include "ship_auto.h"
#include "../data_layer/error.h"

using namespace schema;
using namespace automation::ship;

// ----------------------------------------------------------------
// Hardcoded constants for the time being
const std::string AsteroidFieldWaypoint = "X1-ZA40-99095A";
const std::string contractItem = "IRON_ORE";
const std::string contractWaypoint = "X1-ZA40-15970B";
const std::string contractID = "clhmbbth30031s60d5ai3plj9";
const std::unordered_set<std::string> notForSale = {"ANTIMATTER", contractItem};
// ----------------------------------------------------------------

ShipAutomator::ShipAutomator(schema::Ship &ship, dal::DataAccessLayer &DALInstance)
{
    p_ship = &ship;
    p_DALInstance = &DALInstance;
    status = TO_MINE;
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
        log("Yield = " + response.yield.printStat() + ". CD = " + std::to_string(response.cooldownSeconds) + ". Cargo = " + response.cargo.printStat());

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
                log("Found not for sale item " + item.symbol + ", skipping...");
                continue;
            }

            SellResponse response = p_DALInstance->sell(p_ship->symbol, item.symbol, item.units);
            log("Sold " + std::to_string(response.units) + "x " + response.tradeSymbol + " for " + std::to_string(response.totalPrice) + " at " + std::to_string(response.pricePerUnit) + " each.");
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
    log("Navigating to " + targetWaypoint + "...");
    try
    {
        NavResponse response = p_DALInstance->navigate(p_ship->symbol, targetWaypoint);
        int ETA = response.nav.route.getETA();
        log("Fuel left: " + response.fuel.printStat() + ". ETA: " + std::to_string(ETA) + " seconds.");
        sleep(ETA);
        log("Navigated to " + targetWaypoint + ".");
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
    log("Delivering contract " + contractID + "...");
    try
    {
        updateCargo();
        for (auto &item : p_ship->cargo.inventory)
        {
            if (item.symbol == contractItem)
            {
                log("Found " + std::to_string(item.units) + "x " + contractItem + ", delivering...");
                p_DALInstance->deliverContract(contractID, p_ship->symbol, item.symbol, item.units);
                log("Delivered " + std::to_string(item.units) + "x " + item.symbol + " to fulfill contract " + contractID + ".");
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
    log("Sleeping for " + std::to_string(seconds) + " seconds...");
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    log("Waking up from sleep after " + std::to_string(seconds) + " seconds.");
}

void ShipAutomator::log(const std::string &message)
{
    time_t curTime = std::time(NULL);
    auto timeObject = std::localtime(&curTime);
    std::string timeString = std::to_string(timeObject->tm_year + 1900) + "-" + std::to_string(timeObject->tm_mon + 1) + "-" + std::to_string(timeObject->tm_mday) + " " + std::to_string((timeObject->tm_hour + 8) % 24) + ":" + std::to_string(timeObject->tm_min) + ":" + std::to_string(timeObject->tm_sec);
    std::cout << timeString << " " << p_ship->symbol << ": " << message << std::endl;
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
