#include "spdlog/spdlog.h"
#include <fmt/core.h>

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
    json::array jsonArray = response.as_array();
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
    return extractShips(response.at(U("data")));
}

ExtractResponse DataAccessLayer::mine(const std::string &shipSymbol)
{
    log(fmt::format("Mining with {}...", shipSymbol));

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/extract"));

    json::value response = sendRequest(request);
    checkAndThrowError(response);

    log(fmt::format("Mined with {}.", shipSymbol));
    return ExtractResponse(response.at(U("data")));
}

Cargo DataAccessLayer::getShipCargo(const std::string &shipSymbol)
{
    log(fmt::format("Getting ship cargo for {}...", shipSymbol));

    http_request request(methods::GET);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/cargo"));

    json::value response = sendRequest(request);
    checkAndThrowError(response);

    log(fmt::format("Got ship cargo for {}.", shipSymbol));
    return Cargo(response.at(U("data")));
}

SellResponse DataAccessLayer::sell(const std::string &shipSymbol, const std::string &tradeSymbol, int unit)
{
    log(fmt::format("Selling cargo for {}, {}x {}...", shipSymbol, unit, tradeSymbol));

    json::value payload;
    payload["symbol"] = json::value::string(tradeSymbol);
    payload["units"] = json::value::number(unit);

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/sell"));

    json::value response = sendRequest(request, payload);
    checkAndThrowError(response);

    log(fmt::format("Sold cargo for {}, {}x {}.", shipSymbol, unit, tradeSymbol));
    return SellResponse(response.at(U("data")));
}

NavResponse DataAccessLayer::navigate(const std::string &shipSymbol, const std::string &destinationSymbol)
{
    log(fmt::format("Navigating {} to {}...", shipSymbol, destinationSymbol));

    json::value payload;
    payload["waypointSymbol"] = json::value::string(destinationSymbol);

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/navigate"));

    json::value response = sendRequest(request, payload);
    checkAndThrowError(response);

    log(fmt::format("Navigated {} to {}.", shipSymbol, destinationSymbol));
    return NavResponse(response.at(U("data")));
}

bool DataAccessLayer::deliverContract(
    const std::string &contractId,
    const std::string &shipSymbol,
    const std::string &tradeSymbol,
    int unit)
{
    log(fmt::format("Delivering contract {}, {}x {} by {}...", contractId, unit, tradeSymbol, shipSymbol));

    json::value payload;
    payload["shipSymbol"] = json::value::string(shipSymbol);
    payload["tradeSymbol"] = json::value::string(tradeSymbol);
    payload["units"] = json::value::number(unit);

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/contracts/" + contractId + "/deliver"));

    json::value response = sendRequest(request, payload);
    checkAndThrowError(response);

    log(fmt::format("Delivered contract {}, {}x {} by {}.", contractId, unit, tradeSymbol, shipSymbol));
    return true;
}

bool DataAccessLayer::dock(const std::string &shipSymbol)
{
    log(fmt::format("Docking {}...", shipSymbol));

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/dock"));

    json::value response = sendRequest(request);
    checkAndThrowError(response);

    log(fmt::format("Docked {}.", shipSymbol));
    return true;
}

bool DataAccessLayer::orbit(const std::string &shipSymbol)
{
    log(fmt::format("Orbiting {}...", shipSymbol));

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/orbit"));

    json::value response = sendRequest(request);
    checkAndThrowError(response);

    log(fmt::format("Orbited {}.", shipSymbol));
    return true;
}

bool DataAccessLayer::refuel(const std::string &shipSymbol)
{
    log(fmt::format("Refueling {}...", shipSymbol));

    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + U(ACCESS_TOKEN));
    request.set_request_uri(U("/my/ships/" + shipSymbol + "/refuel"));

    json::value response = sendRequest(request);
    checkAndThrowError(response);

    log(fmt::format("Refueled {}.", shipSymbol));
    return true;
}

bool DataAccessLayer::checkAndThrowError(const json::value &response)
{
    log(fmt::format("Checking for error in response = {}...", response.serialize()));

    if (response.has_field(U("error")))
    {
        json::value error = response.at(U("error"));
        log(fmt::format("Error found in response = {}.", error.serialize()));

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
        log("Throwing base error", spdlog::level::critical);
        throw BaseException(error);
    }

    log(fmt::format("No error found in response = {}.", response.serialize()));
    return true;
}

void DataAccessLayer::log(const std::string &message, spdlog::level::level_enum level)
{
    std::string formattedMessage = fmt::format("DAL: {}", message);
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
                log(fmt::format("Rate limited response = {}.", response.serialize()));
                int retrymsec = (int)(error.at(U("data")).at(U("retryAfter")).as_double() * 1000.0);
                log(fmt::format("Sleeping for {} milliseconds...", retrymsec));
                std::this_thread::sleep_for(std::chrono::milliseconds(retrymsec));
                log(fmt::format("Waking up from sleep after {} milliseconds...", retrymsec));
                continue;
            }
        }
        return response;
    }
}