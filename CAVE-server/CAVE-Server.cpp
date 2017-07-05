#include <iostream>
#include <boost/asio.hpp>
#include <thread>

#include "MessageCodes.h"
#include "ChronoMessages.pb.h"
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
    return 0;
}
