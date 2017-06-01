#include <iostream>
#include "core/ChVector.h"
#include "ChDSRCAgent.h"
#include "ChronoMessages.pb.h"
#include "MessageConversions.h"
#include <google/protobuf/message.h>
#include <boost/asio.hpp>

#define MAX_REACHABLE_DISTANCE 10.0
#define HEADER_SIZE 32

static int vehicleCount = 0;
static std::map<int, ChDSRCAgent*> vehicleMap;

ChDSRCAgent::ChDSRCAgent(ChWheeledVehicle* veh) {
    vehicle = veh;
    m_vehicleNumber = vehicleCount;
    vehicleMap.insert(std::pair<int, ChDSRCAgent*>(m_vehicleNumber, this));
    vehicleCount++;
}

bool ChDSRCAgent::canReach(int vehicleNum) {
    return (vehicleMap[vehicleNum]->vehicle->GetVehiclePos() - vehicle->GetVehiclePos()).Length() <= MAX_REACHABLE_DISTANCE;
}

int ChDSRCAgent::vehicleNumber() {
    return m_vehicleNumber;
}

// Broadcasts a message. All vehicles within range will be able to recieve it.
void ChDSRCAgent::broadcastMessage(std::vector<uint8_t> buffer) {
    ChronoMessages::DSRCMessage message;
    message.set_vehicleid(m_vehicleNumber);
    message.set_timestamp(time(0));
    message.set_chtime(vehicle->GetChTime());
    messageFromVector(message.mutable_vehiclepos(), vehicle->GetVehiclePos());
    message.set_buffer(buffer.data(), buffer.size());

    auto sendBuf = std::make_shared<boost::asio::streambuf>();
    std::ostream stream(sendBuf.get());
    message.SerializeToOstream(&stream);
    stream.flush();
    for (std::pair<int, ChDSRCAgent*> agentPair : vehicleMap)
        if (canReach(agentPair.first) && agentPair.first != m_vehicleNumber)
            agentPair.second->incomingMessages.push(sendBuf);
}

// Pops message from the end of the message queue. If there are no messages, a vector of size 0 is returned.
std::vector<uint8_t> ChDSRCAgent::popMessage() {
    if (incomingMessages.size() != 0) {
        auto buffer = incomingMessages.front();
        incomingMessages.pop();
        ChronoMessages::DSRCMessage message;
        std::istream stream(buffer.get());
        buffer->commit(buffer->size());
        message.ParseFromIstream(&stream);
        message.CheckInitialized();
        std::vector<uint8_t> messageVector(message.buffer().size());
        messageVector.assign(message.buffer().begin(), message.buffer().end());
        return messageVector;
    }
    return *(new std::vector<uint8_t>());
}
