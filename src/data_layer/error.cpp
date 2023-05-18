#include <cpprest/json.h>

#include "error.h"

using namespace web;
using namespace error;

BaseException::BaseException(json::value &error)
    : message(error.at(U("message")).as_string()), data(error.at(U("data")))
{
    code = error.at(U("code")).as_integer();
}

const char *BaseException::what() const throw()
{
    return message.c_str();
}
json::value BaseException::getData() const
{
    return data;
}
int BaseException::getErrorCode() const
{
    return code;
}

ExtractCooldownException::ExtractCooldownException(json::value &error) : BaseException(error)
{
    cooldown = data.at(U("cooldown")).at(U("remainingSeconds")).as_integer();
}

int ExtractCooldownException::getCooldown() const
{
    return cooldown;
}

InTransitException::InTransitException(json::value &error) : BaseException(error)
{
    departureSymbol = data.at(U("departureSymbol")).as_string();
    destinationSymbol = data.at(U("destinationSymbol")).as_string();
    secondsToArrival = data.at(U("secondsToArrival")).as_integer();
}

std::string InTransitException::getDepartureSymbol() const
{
    return departureSymbol;
}

std::string InTransitException::getDestinationSymbol() const
{
    return destinationSymbol;
}

int InTransitException::getSecondsToArrival() const
{
    return secondsToArrival;
}

FullCargoException::FullCargoException(json::value &error) : BaseException(error)
{
}

ExtractInvalidWaypointException::ExtractInvalidWaypointException(json::value &error) : BaseException(error)
{
}

NavigateSameLocationException::NavigateSameLocationException(json::value &error) : BaseException(error)
{
}

NavigateInsufficientFuelException::NavigateInsufficientFuelException(json::value &error) : BaseException(error)
{
    fuelRequired = error.at(U("data")).at(U("fuelRequired")).as_integer();
    fuelAvailable = error.at(U("data")).at(U("fuelAvailable")).as_integer();
}