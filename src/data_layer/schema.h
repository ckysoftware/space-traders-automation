#pragma once

#include <cpprest/json.h>

namespace schema
{
    class CargoItem
    {
    public:
        CargoItem(web::json::value json);

        std::string symbol;
        std::string name;
        std::string description;
        int units;
    };

    class Cargo
    {
    public:
        Cargo(web::json::value json);

        bool isFull();
        bool isEmpty();
        int capacity;
        int units;
        std::vector<CargoItem> inventory;
        std::string printStat();
    };

    class Fuel
    {
    public:
        Fuel(web::json::value json);

        int current;
        int capacity;
        int consumedAmount;
        std::string timestamp;
        std::string printStat();
    };

    class ShipBasic
    {
    public:
        ShipBasic(web::json::value json);

        std::string symbol;
        std::string name;
        std::string role;
    };

    class Ship : public ShipBasic
    {
    public:
        Ship(web::json::value json);

        Cargo cargo;
        Fuel fuel;
    };

    class Yield
    {
    public:
        Yield(web::json::value json);

        std::string symbol;
        int units;
        std::string printStat();
    };

    class NavRouteWaypoint
    {
    public:
        NavRouteWaypoint(web::json::value json);

        std::string symbol;
        std::string type;
        std::string systemSymbol;
        int x;
        int y;
    };

    class NavRoute
    {
    public:
        NavRoute(web::json::value json);
        int getETA();

        NavRouteWaypoint departure;
        NavRouteWaypoint destination;
        std::string arrival;
        std::string departureTime;
    };

    class Nav
    {
    public:
        Nav(web::json::value json);

        std::string systemSymbol;
        std::string waypointSymbol;
        NavRoute route;
        std::string status;
        std::string flightMode;
    };

    class ExtractResponse
    {
    public:
        ExtractResponse(web::json::value json);

        Yield yield;
        int cooldownSeconds;
        Cargo cargo;
    };

    class SellResponse
    {
    public:
        SellResponse(web::json::value json);

        std::string tradeSymbol;
        int units;
        int totalPrice;
        int pricePerUnit;
        Cargo cargo;
    };

    class NavResponse
    {
    public:
        NavResponse(web::json::value json);

        Fuel fuel;
        Nav nav;
    };
}