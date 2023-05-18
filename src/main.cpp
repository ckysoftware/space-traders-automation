// https://github.com/Microsoft/cpprestsdk/wiki/Getting-Started-Tutorial

#include <string>
#include <iostream>
#include <thread>
#include <vector>

#include "data_layer/schema.h"
#include "data_layer/data_access.h"
#include "automation/ship_auto.h"

using namespace schema;
using namespace web;
using namespace web::http;

void printShip(std::vector<Ship> ships)
{
    int shipNum = 1;
    for (auto ship : ships)
    {
        std::cout << shipNum << U(": Symbol=") << ship.symbol;
        std::cout << ", Name=" << ship.name;
        std::cout << ", Role=" << ship.role << std::endl;
        shipNum++;
    }
}

int main()
{
    std::cout << "***** started *****" << std::endl;

    const std::string baseURI = "https://api.spacetraders.io/v2/";
    dal::DataAccessLayer DALInstance(baseURI);

    std::cout << "***** getting ship *****" << std::endl;
    std::vector<Ship> ships = DALInstance.getShips();
    printShip(ships);

    std::vector<automation::ship::ShipAutomator> shipAutomators;
    for (auto &ship : ships)
    {
        // if (ship.symbol == "GET_RICH_QUICK-1") {
        //     continue;
        // }

        shipAutomators.emplace_back(ship, DALInstance);
        std::cout << "Creating ship automator for " << ship.symbol << std::endl;
    }

    std::vector<std::thread> automationThreads;
    for (auto &shipAutomator : shipAutomators)
    {
        std::cout << "Starting thread for " << shipAutomator.getShipSymbol() << std::endl;
        automationThreads.emplace_back(&automation::ship::ShipAutomator::start, &shipAutomator);
    }

    // automation::ship::ShipAutomator shipAutomator(ships[2], DALInstance);
    // shipAutomator.start();

    for (auto &automationThread : automationThreads)
    {
        automationThread.join();
    }

    std::cout << "***** ended *****" << std::endl;
    return 0;
}