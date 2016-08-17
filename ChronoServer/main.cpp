#include <iostream>
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <typeinfo>
#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>

#include "World.h"
#include "ChronoMessages.pb.h"
#include "MessageCodes.h"

// Start of the listener thread
void listenForConnection(World& world, std::queue<std::function<void()>>& queue, std::shared_ptr<std::mutex> queueMutex);

// Start of the client connection handling thread
void processConnection(World& world, std::queue<std::function<void()>>& queue, std::shared_ptr<std::mutex> queueMutex, std::shared_ptr<boost::asio::ip::tcp::socket> socket, int connectionNumber);

int main(int argc, char **argv)
{
    std::queue<std::function<void()>> worldQueue;
    std::shared_ptr<std::mutex> queueMutex = std::make_shared<std::mutex>();
    
    World world(1, 1);
    
    std::function<void(World&, std::queue<std::function<void()>>&, std::shared_ptr<std::mutex>)> listenerFunc = listenForConnection;
    std::thread listener(listenerFunc, std::ref(world), std::ref(worldQueue), queueMutex);
    
    std::cout << "World queue processing is about to start." << std::endl;
    while(true) {
        if (!worldQueue.empty()){
            worldQueue.front()();
            worldQueue.pop();
        }
    }
    listener.join();
	return 0;
}

void listenForConnection(World& world, std::queue<std::function<void()>>& queue, std::shared_ptr<std::mutex> queueMutex) {
    std::cout << "Listener thread started" << std::endl;
    
    // Used to give vehicles unique id numbers
    int connectionNumber = 0;
    
    // Setting up socket
    std::vector<std::thread> clientConnections;
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::acceptor acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8082));
    
    // Making function for listener threads
    std::function<void(World&, std::queue<std::function<void()>>&, std::shared_ptr<std::mutex>, std::shared_ptr<boost::asio::ip::tcp::socket>, int)> connectionFunc = processConnection;
    while(true) {
        // Deletion of this socket is done in the client connection thread
        std::shared_ptr<boost::asio::ip::tcp::socket> socket = std::make_shared<boost::asio::ip::tcp::socket>(ioService);
        std::cout << "About to listen" << std::endl;
        acceptor.accept(*socket);
        std::cout << "Accepted Connection" << std::endl;
        
        // Creates new thread for the client connection here
        clientConnections.emplace_back(connectionFunc, std::ref(world), std::ref(queue), queueMutex, socket, connectionNumber);
        connectionNumber++;
    }
    // Joins all the threads back to the listener thread
    for(uint i = 0; i < clientConnections.size(); i++)
        clientConnections[i].join();
}

void processConnection(World& world, std::queue<std::function<void()>>& queue, std::shared_ptr<std::mutex> queueMutex, std::shared_ptr<boost::asio::ip::tcp::socket> socket, int connectionNumber) {
    std::cout << "Processing connection" << std::endl;
    
    try {
        // Send its identification number
        char* connectionBuff = (char *)&connectionNumber;
        boost::asio::write(*socket, boost::asio::buffer(connectionBuff, sizeof(int)));
        
        uint8_t receivingCode;
        boost::asio::streambuf buffer;
        std::istream startStream(&buffer);
        
        // Receives Initial state of the vehicle
        socket->receive(boost::asio::buffer(&receivingCode, sizeof(uint8_t)));
        
        std::shared_ptr<ChronoMessages::VehicleMessage> vehicle = std::make_shared<ChronoMessages::VehicleMessage>();
        socket->receive(buffer.prepare(361));
        buffer.commit(361);
        vehicle->ParseFromIstream(&startStream);
        buffer.consume(vehicle->ByteSize());
        
        // Pushes addVehicle to queue so that the vehicle may be added to the world
        std::lock_guard<std::mutex>* guard = new std::lock_guard<std::mutex>(*queueMutex);
        queue.push([&world, vehicle] { world.addVehicle(0, 0, vehicle.get()); });
        delete guard;
        
        while(world.getSection(0, 0).find(connectionNumber) == world.getSection(0, 0).end());
        
        while(socket->is_open()) {
            // Responds to client with all other serialized vehicles in the world. Edit demo to add the vehicles to its system.
            uint8_t messageCode = VEHICLE_MESSAGE;
            std::map<int, ChronoMessages::VehicleMessage>& section = world.getSection(0, 0);
            for(std::pair<const int, ChronoMessages::VehicleMessage> worldPair : section) {
                if(worldPair.second.vehicleid() != connectionNumber) {
                    if(worldPair.second.IsInitialized()) {
                        messageCode = VEHICLE_MESSAGE;
                        socket->send(boost::asio::buffer(&messageCode, sizeof(uint8_t)));
                        
                        boost::asio::streambuf worldBuffer;
                        std::ostream outStream(&worldBuffer);
                        worldPair.second.SerializeToOstream(&outStream);
                        boost::asio::write(*socket, worldBuffer);
                    } else {
                        std::cout << "Serialization Error" << std::endl;
                        messageCode = VEHICLE_ID;
                        socket->send(boost::asio::buffer(&messageCode, sizeof(uint8_t)));
                        uint32_t id = vehicle->vehicleid();
                        socket->send(boost::asio::buffer(&id, sizeof(uint32_t)));
                    }
                }
            }
            
            messageCode = VEHICLE_MESSAGE_END;
            socket->send(boost::asio::buffer(&messageCode, sizeof(uint8_t)));
            
            std::istream inStream(&buffer);
            
            // Receives update message
            socket->receive(boost::asio::buffer(&receivingCode, sizeof(uint8_t)));
            
            socket->receive(buffer.prepare(361));
            buffer.commit(361);
            vehicle->ParseFromIstream(&inStream);
            buffer.consume(vehicle->ByteSize());
            
            // Pushes updateVehicle to queue to update the vehicle state in the world
            std::lock_guard<std::mutex>* guard = new std::lock_guard<std::mutex>(*queueMutex);
            queue.push([&world, vehicle] { world.updateVehicle(0, 0, *vehicle); });
            delete guard;
        }
    } catch (std::exception& error) {
        std::cout << error.what() << std::endl;
        std::lock_guard<std::mutex>* guard = new std::lock_guard<std::mutex>(*queueMutex);
        queue.push([&world, connectionNumber] { world.removeVehicle(0, 0, connectionNumber); });
        delete guard;
        socket->close();
    }
}