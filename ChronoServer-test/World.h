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

#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <vector>
#include <memory>
#include "ChronoMessages.pb.h"

class World
{
public:
    World(int sizeX, int sizeY);
    ~World();

    // Adds a vehicle to the specified section.
    void addVehicle(int sectionX, int sectionY, std::shared_ptr<ChronoMessages::VehicleMessage> message);

    // Updates the status of a vehicle.
    void updateVehicle(int sectionX, int sectionY, std::shared_ptr<ChronoMessages::VehicleMessage> message);

    // Removes the vehicle with the id from the world.
    void removeVehicle(int sectionX, int sectionY, int id);

    // The number of vehicles in the world.
    int numVehicles();

    std::map<int, std::shared_ptr<ChronoMessages::VehicleMessage>>& getSection(int sectionX, int sectionY);
private:
    // Each section of the world has a mapping of the vehicles within it.
    std::vector<std::vector<std::map<int, std::shared_ptr<ChronoMessages::VehicleMessage>>>> sectionGrid;

    int numVehicles_;
    // Possibly add a map for world objects?
};

#endif // WORLD_H
