#pragma once

#include "spdlog/spdlog.h"

#include "../data_layer/schema.h"
#include "../data_layer/data_access.h"
#include "../data_layer/error.h"

namespace automation
{
    namespace ship
    {
        enum Status
        {
            TO_MINE,
            FULL,
            IN_ORBIT,
            IN_DOCK,
            TO_SELL,
            TO_NAVIGATE,
            TO_DELIVER,
            TEMP_IN_TRANSIT,
            TEMP_ON_EXTRACT_CD,
        };

        class ShipAutomator
        {
        public:
            ShipAutomator(schema::Ship &ship, dal::DataAccessLayer &DALInstance);
            void start();
            std::string getShipSymbol();

        private:
            bool mine();
            bool dock();
            void updateCargo();
            void updateCargo(schema::Cargo cargo);
            bool orbit();
            bool sell();
            bool navigate();
            bool refuel();
            bool deliverContract();

            void sleep(int seconds);
            void log(const std::string &message, spdlog::level::level_enum level = spdlog::level::info);
            void setTargetWaypoint(const std::string waypointSymbol);

            void handleInTransitError(const error::InTransitException &e);
            void handleExtractCooldownError(const error::ExtractCooldownException &e);
            void handleFullCargoError(const error::FullCargoException &e);
            void handleExtractInvalidWaypointError(const error::ExtractInvalidWaypointException &e);
            void handleNavigateSameLocationError(const error::NavigateSameLocationException &e);
            void handleNavigateInsufficientFuelError(const error::NavigateInsufficientFuelException &e);
            // TODO make dock, orbit retry until successful
            // TODO use strategy class for functional state transition
            // TODO make a full set of status including sth like full_cargo_to_deliver
            schema::Ship *p_ship;
            dal::DataAccessLayer *p_DALInstance;
            Status status;
            bool toDeliver;
            std::string targetWaypoint;
            // TODO implement a queue of planned actions using double linked list?
        };
    }
}