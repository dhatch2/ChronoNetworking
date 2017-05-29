#include <iostream>
#include <utility>
#include "core/ChVector.h"
#include "ChDSRCAgent.h"
#include "ChronoMessages.pb.h"
#include "MessageConversions.h"
#include <google/protobuf/message.h>
#include <boost/asio.hpp>
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

void ChDSRCAgent::broadcastMessage(boost::asio::streambuf buffer) {
    ChronoMessages::DSRCHeader header;
    header.set_vehicleid(m_vehicleNumber);
    header.set_timestamp(time(0));
    header.set_chtime(vehicle->GetChTime());
    messageFromVector(header.mutable_vehiclepos(), vehicle->GetVehiclePos());
}
