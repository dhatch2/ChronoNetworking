#include "World.h"
#include <iostream>

World::World(int sizeX, int sizeY)
{
    numVehicles_ = 0;
    for(int i = 0; i < sizeX; i++) {
        std::vector<std::map <int, ChronoMessages::VehicleMessage>> column;
        for(int j = 0; j < sizeY; j++) {
            std::map<int, ChronoMessages::VehicleMessage> section;
            column.push_back(section);
        }
        sectionGrid.push_back(column);
    }
}

World::~World()
{
}

void World::addVehicle(int sectionX, int sectionY, ChronoMessages::VehicleMessage message) {
    numVehicles_++;
    sectionGrid[sectionX][sectionY].insert(std::pair<int, ChronoMessages::VehicleMessage>(message.vehicleid(), message));
    std::cout << "Vehicle added" << std::endl;
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

std::map<int, ChronoMessages::VehicleMessage>& World::getSection(int sectionX, int sectionY) {
    return sectionGrid[sectionX][sectionY];
}