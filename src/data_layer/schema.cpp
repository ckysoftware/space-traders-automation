#include "spdlog/spdlog.h"
#include <fmt/core.h>

#include "schema.h"

#include <iomanip>

using namespace schema;
using namespace web;

CargoItem::CargoItem(json::value json)
{
    symbol = json.at(U("symbol")).as_string();
    name = json.at(U("name")).as_string();
    description = json.at(U("description")).as_string();
    units = json.at(U("units")).as_integer();
}

Cargo::Cargo(json::value json)
{
    capacity = json.at(U("capacity")).as_integer();
    units = json.at(U("units")).as_integer();
    inventory = std::vector<CargoItem>();
    int unitSum = 0;
    for (auto item : json.at(U("inventory")).as_array())
    {
        CargoItem inventoryItem(item);
        inventory.push_back(inventoryItem);
        unitSum += inventoryItem.units;
    }
    if (units != unitSum)
    {
        spdlog::warn(fmt::format("Cargo: Cargo unit sum does not match. Unit={} / Sum={}", units, unitSum));
    }
}

bool Cargo::isFull()
{
    if (units > capacity)
    {
        spdlog::warn(fmt::format("Cargo: Cargo is over capacity. Unit={} / Capacity={}", units, capacity));
    }
    return units >= capacity;
}

bool Cargo::isEmpty()
{
    return units == 0;
}

std::string Cargo::printStat()
{
    return std::to_string(units) + "/" + std::to_string(capacity) + " (" + std::to_string((int)((float)units / (float)capacity * 100)) + "%)";
}

Fuel::Fuel(json::value json)
{
    current = json.at(U("current")).as_integer();
    capacity = json.at(U("capacity")).as_integer();
    consumedAmount = json.at(U("consumed")).at(U("amount")).as_integer();
    timestamp = json.at(U("consumed")).at(U("timestamp")).as_string();
}

std::string Fuel::printStat()
{
    return std::to_string(current) + "/" + std::to_string(capacity) + " (" + std::to_string((int)((float)current / (float)capacity * 100)) + "%)";
}

ShipBasic::ShipBasic(json::value json)
{
    symbol = json.at(U("symbol")).as_string();
    name = json.at(U("registration")).at(U("name")).as_string();
    role = json.at(U("registration")).at(U("role")).as_string();
}

Ship::Ship(web::json::value json) : ShipBasic(json), cargo(json.at(U("cargo"))), fuel(json.at(U("fuel")))
{
}

Yield::Yield(json::value json)
{
    symbol = json.at(U("symbol")).as_string();
    units = json.at(U("units")).as_integer();
}

std::string Yield::printStat()
{
    return std::to_string(units) + "x " + symbol;
}

NavRouteWaypoint::NavRouteWaypoint(json::value json)
{
    symbol = json.at(U("symbol")).as_string();
    type = json.at(U("type")).as_string();
    systemSymbol = json.at(U("systemSymbol")).as_string();
    x = json.at(U("x")).as_integer();
    y = json.at(U("y")).as_integer();
}

NavRoute::NavRoute(json::value json)
    : departure(NavRouteWaypoint(json.at(U("departure")))), destination(NavRouteWaypoint(json.at(U("destination"))))
{
    arrival = json.at(U("arrival")).as_string();
    departureTime = json.at(U("departureTime")).as_string();
}

int NavRoute::getETA()
{
    // return the ETA in seconds
    std::tm tm = {};
    std::istringstream ss(arrival);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    std::time_t t = std::mktime(&tm);
    return (int)std::ceil(std::difftime(t, std::time(nullptr)));
}

Nav::Nav(json::value json) : route(json.at(U("route")))
{
    systemSymbol = json.at(U("systemSymbol")).as_string();
    waypointSymbol = json.at(U("waypointSymbol")).as_string();
    status = json.at(U("status")).as_string();
    flightMode = json.at(U("flightMode")).as_string();
}

ExtractResponse::ExtractResponse(web::json::value json) : cargo(json.at(U("cargo"))), yield(json.at(U("extraction")).at(U("yield")))
{
    cooldownSeconds = json.at(U("cooldown")).at(U("remainingSeconds")).as_integer();
}

SellResponse::SellResponse(web::json::value json) : cargo(json.at(U("cargo")))
{
    tradeSymbol = json.at(U("transaction")).at(U("tradeSymbol")).as_string();
    units = json.at(U("transaction")).at(U("units")).as_integer();
    totalPrice = json.at(U("transaction")).at(U("totalPrice")).as_integer();
    pricePerUnit = json.at(U("transaction")).at(U("pricePerUnit")).as_integer();
}

NavResponse::NavResponse(json::value json) : nav(json.at(U("nav"))), fuel(json.at(U("fuel")))
{
}