#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <vector>
#include "ChronoMessages.pb.h"

class World
{
public:
    World(int sizeX, int sizeY);
    ~World();
    
    // Adds a vehicle to the specified section.
    void addVehicle(int sectionX, int sectionY, ChronoMessages::VehicleMessage);
    
    // Updates the status of a vehicle.
    void updateVehicle(int sectionX, int sectionY, ChronoMessages::VehicleMessage);
    
    // Removes the vehicle with the id from the world.
    void removeVehicle(int sectionX, int sectionY, ChronoMessages::VehicleMessage message);
    
    // The number of vehicles in the world.
    int numVehicles();
    
    std::map<int, ChronoMessages::VehicleMessage>& getSection(int sectionX, int sectionY);
private:
    // Each section of the world has a mapping of the vehicles within it.
    std::vector<std::vector<std::map<int, ChronoMessages::VehicleMessage>>> sectionGrid;
    
    int numVehicles_;
    // Possibly add a map for world objects?
};

#endif // WORLD_H
