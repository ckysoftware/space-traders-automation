#pragma once

#include <cpprest/json.h>

#include <exception>

namespace error
{
    enum ErrorCode
    {
        RATE_LIMITED = 429,
        EXTRACT_COOLDOWN = 4000,
        NAVIGATE_INSUFFICIENT_FUEL = 4203,
        NAVIGATE_SAME_LOCATION = 4204,
        EXTRACT_INVALID_WAYPOINT = 4205,
        IN_TRANSIT = 4214,
        FULL_CARGO = 4228,
    };

    class BaseException : public std::exception
    {
    public:
        BaseException(web::json::value &error);
        virtual const char *what() const throw();
        web::json::value getData() const;
        int getErrorCode() const;

    protected:
        std::string message;
        int code;
        web::json::value data;
    };

    class ExtractCooldownException : public BaseException
    {
    public:
        ExtractCooldownException(web::json::value &error);
        int getCooldown() const;

    private:
        int cooldown;
    };

    class InTransitException : public BaseException
    {
    public:
        InTransitException(web::json::value &error);
        std::string getDepartureSymbol() const;
        std::string getDestinationSymbol() const;
        int getSecondsToArrival() const;

    private:
        std::string departureSymbol;
        std::string destinationSymbol;
        int secondsToArrival;
    };

    class FullCargoException : public BaseException
    {
    public:
        FullCargoException(web::json::value &error);
    };

    class ExtractInvalidWaypointException : public BaseException
    {
    public:
        ExtractInvalidWaypointException(web::json::value &error);
    };

    class NavigateSameLocationException : public BaseException
    {
    public:
        NavigateSameLocationException(web::json::value &error);
    };

    class NavigateInsufficientFuelException : public BaseException
    {
    public:
        NavigateInsufficientFuelException(web::json::value &error);

        int fuelRequired;
        int fuelAvailable;
    };
}