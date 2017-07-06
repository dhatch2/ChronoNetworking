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
        auto messagePair = handler.popMessage();
        boost::asio::ip::udp::endpoint& endpoint = messagePair.first;
        std::shared_ptr<google::protobuf::Message>& message = messagePair.second;

        auto reflection = message->GetReflection();
        auto descriptor = message->GetDescriptor();
        auto conDesc = descriptor->FindFieldByName(CONNECTION_NUMBER_FIELD);
        int connectionNumber = reflection->GetInt32(*message, conDesc);
        auto idDesc = descriptor->FindFieldByName(ID_NUMBER_FIELD);
        int idNumber = reflection->GetInt32(*message, idDesc);
        std::string type = descriptor->full_name();

        endpointProfile *profile = world.verifyConnection(connectionNumber, endpoint);
        if (profile == NULL) {
            worldQueue.enqueue([&] { world.registerEndpoint(endpoint, connectionNumber); });
        } else if (type.compare(MESSAGE_PACKET_TYPE) == 0) {
            worldQueue.enqueue([&] { world.updateElementsOfProfile(profile, message); });
        } else {
            worldQueue.enqueue([&] { world.updateElement(message, profile, idNumber); });
        }
    }
}
