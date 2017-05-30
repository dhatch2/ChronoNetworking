#include <iostream>
#include "core/ChVector.h"
#include "ChDSRCAgent.h"
#include "ChronoMessages.pb.h"
#include "MessageConversions.h"
#include <google/protobuf/message.h>
#include <boost/asio.hpp>

#define MAX_REACHABLE_DISTANCE 10.0
#define HEADER_SIZE 352

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

void ChDSRCAgent::broadcastMessage(std::vector<uint8_t> buffer) {
    ChronoMessages::DSRCHeader header;
    header.set_vehicleid(m_vehicleNumber);
    header.set_timestamp(time(0));
    header.set_chtime(vehicle->GetChTime());
    messageFromVector(header.mutable_vehiclepos(), vehicle->GetVehiclePos());
    header.set_buffersize(buffer.size());

    std::cout << buffer.data() << std::endl;
    auto sendBuf = std::make_shared<boost::asio::streambuf>();
    std::ostream stream(sendBuf.get());
    header.SerializeToOstream(&stream);
    stream.write((char *)buffer.data(), buffer.size());
    stream.flush();
    for (std::pair<int, ChDSRCAgent*> agentPair : vehicleMap)
        if (canReach(agentPair.first) && agentPair.first != m_vehicleNumber)
            agentPair.second->incomingMessages.push(sendBuf);
}

std::vector<uint8_t> ChDSRCAgent::popMessage() {
    if (incomingMessages.size() != 0) {
        auto buffer = incomingMessages.front();
        incomingMessages.pop();
        ChronoMessages::DSRCHeader header;
        std::istream stream(buffer.get());
        buffer->commit(HEADER_SIZE);
        header.ParseFromIstream(&stream);
        std::cout << header.buffersize() << std::endl;
        std::cout << header.timestamp() << std::endl;
        std::cout << buffer->size() << std::endl;
        buffer->commit(header.buffersize());
        std::vector<uint8_t> message(header.buffersize());
        stream.read((char *)message.data(), header.buffersize());
        //stream >> message.data();
        std::cout << message.data() << std::endl;
        return message;
    } else return *(new std::vector<uint8_t>());
}
