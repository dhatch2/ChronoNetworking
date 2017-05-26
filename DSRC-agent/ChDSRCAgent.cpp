#include <iostream>
#include <utility>
#include "core/ChVector.h"
#include "ChDSRCAgent.h"
#define MAX_REACHABLE_DISTANCE 10.0

static int vehicleCount = 0;
static std::map<int, ChWheeledVehicle*> vehicleMap;

ChDSRCAgent::ChDSRCAgent(ChWheeledVehicle* veh) {
    vehicle = veh;
    m_vehicleNumber = vehicleCount;
    vehicleMap.insert(std::pair<int, ChWheeledVehicle*>(m_vehicleNumber, vehicle));
    vehicleCount++;
}

bool ChDSRCAgent::canReach(int vehicleNum) {
    return (vehicleMap[vehicleNum]->GetVehiclePos() - vehicle->GetVehiclePos()).Length() <= MAX_REACHABLE_DISTANCE;
}

int ChDSRCAgent::vehicleNumber() {
    return m_vehicleNumber;
}
