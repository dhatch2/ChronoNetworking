// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2014 projectchrono.org
// All right reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Dylan Hatch
// =============================================================================
//
//	World object. Used for storing vehicle and world object information.
//
// =============================================================================

#include "World.h"
#include <iostream>

World::World(int sizeX, int sizeY)
{
    numVehicles_ = 0;
    for(int i = 0; i < sizeX; i++) {
        std::vector<std::map <int, std::shared_ptr<ChronoMessages::VehicleMessage>>> column;
        for(int j = 0; j < sizeY; j++) {
            std::map<int, std::shared_ptr<ChronoMessages::VehicleMessage>> section;
            column.push_back(section);
        }
        sectionGrid.push_back(column);
    }
}

World::~World()
{
}

void World::addVehicle(int sectionX, int sectionY, std::shared_ptr<ChronoMessages::VehicleMessage> message) {
    numVehicles_++;
    sectionGrid[sectionX][sectionY].insert(std::pair<int, std::shared_ptr<ChronoMessages::VehicleMessage>>(message->vehicleid(), message));
    //std::cout << "Vehicle added" << std::endl;
}

void World::updateVehicle(int sectionX, int sectionY, std::shared_ptr<ChronoMessages::VehicleMessage> message) {
    if(message->IsInitialized()) sectionGrid[sectionX][sectionY][message->vehicleid()]->MergeFrom(*message);
}

void World::removeVehicle(int sectionX, int sectionY, int id) {
    sectionGrid[sectionX][sectionY].erase(id);
    numVehicles_--;
}

int World::numVehicles() {
    return numVehicles_;
}

std::map<int, std::shared_ptr<ChronoMessages::VehicleMessage>>& World::getSection(int sectionX, int sectionY) {
    std::map<int, std::shared_ptr<ChronoMessages::VehicleMessage>>& copy(sectionGrid[sectionX][sectionY]);
    return copy;
}
