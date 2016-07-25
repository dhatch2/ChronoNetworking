#include "World.h"

World::World(int sizeX, int sizeY)
{
    numVehicles_ = 0;
    for(int i = 0; i < sizeX; i++) {
        std::vector<std::map <int, ChronoMessages::VehicleMessage>> column;
        sectionGrid.push_back(column);
        for(int j = 0; j < sizeY; j++) {
            std::map<int, ChronoMessages::VehicleMessage> section;
            column.push_back(section);
        }
    }
}

World::~World()
{
}

void World::addVehicle(int sectionX, int sectionY, ChronoMessages::VehicleMessage message) {
    numVehicles_++;
    sectionGrid[sectionX][sectionY].insert(std::pair<int, ChronoMessages::VehicleMessage>(message.vehicleid(), message));
}

void World::updateVehicle(int sectionX, int sectionY, ChronoMessages::VehicleMessage message) {
    sectionGrid[sectionX][sectionY][message.vehicleid()] = message;
}

void World::removeVehicle(int sectionX, int sectionY, ChronoMessages::VehicleMessage message) {
    sectionGrid[sectionX][sectionY].erase(message.vehicleid());
    numVehicles_--;
}

int World::numVehicles() {
    return numVehicles_;
}