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
// Main file for the Chrono simulation server.
//
// =============================================================================

#include <iostream>
#include <boost/asio.hpp>
#include <thread>

#include "MessageCodes.h"
//#include "ChronoMessages.pb.h"
#include "ChSafeQueue.h"
#include "ChNetworkHandler.h"
#include "World.h"

void processMessages(World& world, ChSafeQueue<std::function<void()>>& worldQueue, ChServerHandler& handler);

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage: " << std::string(argv[0]) << " <port number>" << std::endl;
        return 1;
    }
    World world;
    ChSafeQueue<std::function<void()>> worldQueue;
    ChServerHandler handler(world, worldQueue, std::stoi(std::string(argv[1])));
    handler.beginListen();
    handler.beginSend();

    std::thread worker(processMessages, std::ref(world), std::ref(worldQueue), std::ref(handler));

    while (true) {
        worldQueue.dequeue()();
    }
    worker.join();
    return 0;
}

void processMessages(World& world, ChSafeQueue<std::function<void()>>& worldQueue, ChServerHandler& handler) {
    while (true) {
        std::cout << "popping..." << std::endl;
        auto messagePair = handler.popMessage();
        boost::asio::ip::udp::endpoint& endpoint = messagePair.first;
        std::shared_ptr<google::protobuf::Message>& message = messagePair.second;
        std::cout << "popped" << std::endl;

        auto reflection = message->GetReflection();
        auto descriptor = message->GetDescriptor();
        auto conDesc = descriptor->FindFieldByName(CONNECTION_NUMBER_FIELD);
        int connectionNumber = reflection->GetInt32(*message, conDesc);
        auto idDesc = descriptor->FindFieldByName(ID_NUMBER_FIELD);
        int idNumber = reflection->GetInt32(*message, idDesc);
        std::string type = descriptor->full_name();
        std::cout << "done doing all that stuff" << std::endl;
        std::cout << "connectionNumber = " << connectionNumber << std::endl;

        endpointProfile *profile = world.verifyConnection(connectionNumber, endpoint);
        if (profile == NULL) {
            std::cout << "profile is null" << std::endl;
            worldQueue.enqueue([&] {
                std::cout << "executing lambda thing" << std::endl;
                if(world.registerEndpoint(endpoint, connectionNumber)) {
                    profile = world.verifyConnection(connectionNumber, endpoint);
                    if (profile != NULL) {
                        if (type.compare(MESSAGE_PACKET_TYPE) == 0) {
                            world.updateElementsOfProfile(profile, message);
                        } else {
                            world.updateElement(message, profile, idNumber);
                        }
                        std::cout << "endpoint registered" << std::endl;
                    }
                }
            });
        } else if (type.compare(MESSAGE_PACKET_TYPE) == 0) {
            worldQueue.enqueue([&] { world.updateElementsOfProfile(profile, message); });
        } else {
            std::cout << "updating element..." << std::endl;
            worldQueue.enqueue([&] { world.updateElement(message, profile, idNumber); });
        }
        if (profile != NULL) {
            handler.pushMessage(endpoint, *world.generateWorldPacket());
        }
    }
}
