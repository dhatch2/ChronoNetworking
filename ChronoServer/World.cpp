#include "World.h"
#include <iostream>

World::World(int sizeX, int sizeY)
{
    numVehicles_ = 0;
    for(int i = 0; i < sizeX; i++) {
        std::vector<std::map <int, ChronoMessages::VehicleMessage>> column;
        std::vector<ChronoMessages::SectionMessage> messageColumn;
        for(int j = 0; j < sizeY; j++) {
            std::map<int, ChronoMessages::VehicleMessage> section;
            column.push_back(section);
            
            ChronoMessages::SectionMessage message;
            messageColumn.push_back(message);
        }
        sectionGrid.push_back(column);
        sectionMessages.push_back(messageColumn);
    }
}

World::~World()
{
}

void World::addVehicle(int sectionX, int sectionY, ChronoMessages::VehicleMessage* message) {
    numVehicles_++;
    sectionGrid[sectionX][sectionY].insert(std::pair<int, ChronoMessages::VehicleMessage>(message->vehicleid(), *message));
    ChronoMessages::VehicleMessage* newMessage = sectionMessages[sectionX][sectionY].add_message();
    *newMessage = *message;
    std::cout << "Vehicle added" << std::endl;
    //std::cout << message.DebugString() << std::endl;
}

void World::updateVehicle(int sectionX, int sectionY, ChronoMessages::VehicleMessage message) {
    //std::cout << message.DebugString() << std::endl;
    sectionGrid[sectionX][sectionY][message.vehicleid()] = message;
    //std::cout << "Section size: " << sectionGrid[sectionX][sectionY].size() << std::endl;
}

void World::removeVehicle(int sectionX, int sectionY, int id) {
    sectionGrid[sectionX][sectionY].erase(id);
    numVehicles_--;
}

int World::numVehicles() {
    return numVehicles_;
}

std::map<int, ChronoMessages::VehicleMessage>& World::getSection(int sectionX, int sectionY) {
    return sectionGrid[sectionX][sectionY];
}