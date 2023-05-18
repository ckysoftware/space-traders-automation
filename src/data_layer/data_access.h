#pragma once

#include <cpprest/http_client.h>

#include <vector>

#include "schema.h"

namespace dal
{
    static web::json::value NULL_JSON_BODY = web::json::value();

    class DataAccessLayer
    {
    public:
        DataAccessLayer(std::string baseURI);
        std::vector<schema::Ship> getShips();

        schema::ExtractResponse mine(const std::string &shipSymbol);
        schema::Cargo getShipCargo(const std::string &shipSymbol);
        schema::SellResponse sell(const std::string &shipSymbol, const std::string &tradeSymbol, int);
        schema::NavResponse navigate(const std::string &shipSymbol, const std::string &destinationSymbol);
        bool deliverContract(
            const std::string &contractId,
            const std::string &shipSymbol,
            const std::string &tradeSymbol,
            int units);
        bool dock(const std::string &shipSymbol);
        bool orbit(const std::string &shipSymbol);
        bool refuel(const std::string &shipSymbol);

    private:
        web::http::client::http_client client;
        bool checkAndThrowError(const web::json::value &response);
        void log(const std::string &message);
        web::json::value sendRequest(web::http::http_request &request, const web::json::value &body = NULL_JSON_BODY);
    };
};