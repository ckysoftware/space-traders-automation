#include <ctime>
#include <thread>

#include "data_access.h"
#include "schema.h"
#include "error.h"

using namespace dal;
using namespace schema;
using namespace error;
using namespace web;
using namespace web::http;

const std::string ACCESS_TOKEN = std::getenv("ACCESS_TOKEN");

DataAccessLayer::DataAccessLayer(std::string baseURI) : client(U(baseURI))
{
}

std::vector<Ship> extractShips(json::value &response)
{
    json::array jsonArray = response.at(U("data")).as_array();
    std::vector<Ship> ships;
    for (auto jsonShip : jsonArray)
    {
        Ship ship(jsonShip);
        ships.push_back(ship);
    }
    return ships;
}

std::vector<Ship> DataAccessLayer::getShips()
{
    http_request request(methods::GET);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships"));

    json::value response = sendRequest(request);
    return extractShips(response);
}

ExtractResponse DataAccessLayer::mine(const std::string &shipSymbol)
{
    log("Mining with " + shipSymbol + "...");

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/extract"));

    json::value response = sendRequest(request);
    // json::value response = client.request(request).get().extract_json().get();
    checkAndThrowError(response);

    log("Mined with " + shipSymbol + ".");
    return ExtractResponse(response.at(U("data")));
}

Cargo DataAccessLayer::getShipCargo(const std::string &shipSymbol)
{
    log("Getting ship cargo for " + shipSymbol + "...");

    http_request request(methods::GET);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/cargo"));

    json::value response = sendRequest(request);
    // json::value response = client.request(request).get().extract_json().get();
    checkAndThrowError(response);

    log("Got ship cargo for " + shipSymbol + ".");
    return Cargo(response.at(U("data")));
}

SellResponse DataAccessLayer::sell(const std::string &shipSymbol, const std::string &tradeSymbol, int unit)
{
    log("Selling cargo for " + shipSymbol + ", " + std::to_string(unit) + "x " + tradeSymbol + "...");
    json::value payload;
    payload["symbol"] = json::value::string(tradeSymbol);
    payload["units"] = json::value::number(unit);

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/sell"));
    // request.set_body(payload);

    json::value response = sendRequest(request, payload);
    // json::value response = client.request(request).get().extract_json().get();
    checkAndThrowError(response);

    log("Sold cargo for " + shipSymbol + ", " + std::to_string(unit) + "x " + tradeSymbol + ".");
    return SellResponse(response.at(U("data")));
}

NavResponse DataAccessLayer::navigate(const std::string &shipSymbol, const std::string &destinationSymbol)
{
    log("Navigating " + shipSymbol + " to " + destinationSymbol + "...");

    json::value payload;
    payload["waypointSymbol"] = json::value::string(destinationSymbol);

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/navigate"));
    // request.set_body(payload);

    json::value response = sendRequest(request, payload);
    // json::value response = client.request(request).get().extract_json().get();
    checkAndThrowError(response);

    log("Navigated " + shipSymbol + " to " + destinationSymbol + ".");
    return NavResponse(response.at(U("data")));
}

bool DataAccessLayer::deliverContract(
    const std::string &contractId,
    const std::string &shipSymbol,
    const std::string &tradeSymbol,
    int unit)
{
    log("Delivering contract " + contractId + " by " + shipSymbol + ", " + std::to_string(unit) + "x " + tradeSymbol + "...");

    json::value payload;
    payload["shipSymbol"] = json::value::string(shipSymbol);
    payload["tradeSymbol"] = json::value::string(tradeSymbol);
    payload["units"] = json::value::number(unit);

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/contracts/" + contractId + "/deliver"));
    // request.set_body(payload);

    json::value response = sendRequest(request, payload);
    // json::value response = client.request(request).get().extract_json().get();
    checkAndThrowError(response);

    log("Delivered contract " + contractId + " by " + shipSymbol + ", " + std::to_string(unit) + "x " + tradeSymbol + ".");
    return true;
}

bool DataAccessLayer::dock(const std::string &shipSymbol)
{
    log("Docking " + shipSymbol + "...");

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/dock"));

    json::value response = sendRequest(request);
    // json::value response = client.request(request).get().extract_json().get();
    checkAndThrowError(response);

    log("Docked " + shipSymbol + ".");
    return true;
}

bool DataAccessLayer::orbit(const std::string &shipSymbol)
{
    log("Orbiting " + shipSymbol + "...");

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/orbit"));

    json::value response = sendRequest(request);
    // json::value response = client.request(request).get().extract_json().get();
    checkAndThrowError(response);

    log("Orbited " + shipSymbol + ".");
    return true;
}

bool DataAccessLayer::refuel(const std::string &shipSymbol)
{
    log("Refueling " + shipSymbol + "...");

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/refuel"));

    json::value response = sendRequest(request);
    // json::value response = client.request(request).get().extract_json().get();
    checkAndThrowError(response);

    log("Refueled " + shipSymbol + ".");
    return true;
}

bool DataAccessLayer::checkAndThrowError(const json::value &response)
{
    log("Checking for error in response = " + response.serialize() + "...");

    if (response.has_field(U("error")))
    {
        json::value error = response.at(U("error"));
        log("Error found in response = " + error.serialize() + ".");

        int errorCode = error.at(U("code")).as_integer();
        switch (errorCode)
        {
        case ErrorCode::EXTRACT_COOLDOWN:
            log("Throwing extract cooldown error");
            throw ExtractCooldownException(error);
        case ErrorCode::IN_TRANSIT:
            log("Throwing in transit error");
            throw InTransitException(error);
        case ErrorCode::FULL_CARGO:
            log("Throwing full cargo error");
            throw FullCargoException(error);
        case ErrorCode::EXTRACT_INVALID_WAYPOINT:
            log("Throwing invalid waypoint error");
            throw ExtractInvalidWaypointException(error);
        case ErrorCode::NAVIGATE_SAME_LOCATION:
            log("Throwing same location error");
            throw NavigateSameLocationException(error);
        case ErrorCode::NAVIGATE_INSUFFICIENT_FUEL:
            log("Throwing insufficient fuel error");
            throw NavigateInsufficientFuelException(error);
        }
        log("Throwing base error");
        throw BaseException(error);
    }

    log("No error found in response = " + response.serialize() + ".");
    return true;
}

void DataAccessLayer::log(const std::string &message)
{
    time_t curTime = std::time(NULL);
    auto timeObject = std::localtime(&curTime);
    std::string timeString = std::to_string(timeObject->tm_year + 1900) + "-" + std::to_string(timeObject->tm_mon + 1) + "-" + std::to_string(timeObject->tm_mday) + " " + std::to_string((timeObject->tm_hour + 8) % 24) + ":" + std::to_string(timeObject->tm_min) + ":" + std::to_string(timeObject->tm_sec);
    std::cout << timeString << " DAL: " << message << std::endl;
}

json::value DataAccessLayer::sendRequest(http_request &request, const web::json::value &body)
{
    /* Due to a bug in the cpprestsdk library, a request.body's stream will be consumed
    after the first request. This is a workaround to reinitialize the stream. Otherwise,
    an error will be thrown when the second request is sent occassionally.
    */

    while (true)
    {
        if (body.is_null())
        {
            request.set_body("");
        }
        else
        {
            request.set_body(body);
        }
        json::value response = client.request(request).get().extract_json().get();

        if (response.has_field(U("error")))
        {
            json::value error = response.at(U("error"));
            int errorCode = error.at(U("code")).as_integer();
            if (errorCode == ErrorCode::RATE_LIMITED)
            {
                log("Rate limited response = " + response.serialize() + ".");
                int retrymsec = (int)(error.at(U("data")).at(U("retryAfter")).as_double() * 1000.0);
                log("Sleeping for " + std::to_string(retrymsec) + " milliseconds...");
                std::this_thread::sleep_for(std::chrono::milliseconds(retrymsec));
                log("Waking up from sleep after " + std::to_string(retrymsec) + " milliseconds...");
                continue;
            }
        }
        return response;
    }
}