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

void printNum(int num);

void raiseInt(int& i, int n) {
    i += n;
}

// Start of the listener thread
void listenForConnection(World& world, std::queue<std::function<void()>>& queue, std::mutex* queueMutex);

// Start of the client connection handling thread
void processConnection(World& world, std::queue<std::function<void()>>& queue, std::mutex* queueMutex, boost::asio::ip::tcp::socket* socket, int connectionNumber);

int main(int argc, char **argv)
{
	std::function<void(int)> func = std::bind(printNum, std::placeholders::_1);
    
    int i = 0;
    std::thread th(func, i);
    std::thread th1(func, i + 1);
    std::thread th2(func, i + 2);
    th.join();
    th1.join();
    th2.join();
    
    std::queue<std::function<void()>> worldQueue;
    worldQueue.push(std::bind(raiseInt, std::ref(i), 2));
    worldQueue.push(std::bind(printNum, std::ref(i)));
    std::mutex* queueMutex = new std::mutex();
    
    World world(1, 1);
    
    std::function<void(World&, std::queue<std::function<void()>>&, std::mutex*)> listenerFunc = listenForConnection;
    std::thread listener(listenerFunc, std::ref(world), std::ref(worldQueue), queueMutex);
    
    std::cout << "World queue processing is about to start." << std::endl;
    while(true) {
        if (!worldQueue.empty()){
            std::cout << "About to call front function" << std::endl;
            worldQueue.front()();
            std::cout << "About to pop" << std::endl;
            worldQueue.pop();
            std::cout << "Popped" << std::endl;
        }
    }
    listener.join();
	return 0;
}
// zero mq or just a standard queue. Or look up an atomic queue implementation.
void printNum(int num) {
    std::cout << num << std::endl;
}

void listenForConnection(World& world, std::queue<std::function<void()>>& queue, std::mutex* queueMutex) {
    std::cout << "Listener thread started" << std::endl;
    
    // Used to give vehicles unique id numbers
    int connectionNumber = 0;
    
    // Setting up socket
    std::vector<std::thread> clientConnections;
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::acceptor acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 1300));
    
    // Making function for listener threads
    std::function<void(World&, std::queue<std::function<void()>>&, std::mutex*, boost::asio::ip::tcp::socket*, int)> connectionFunc = processConnection;
    while(true) {
        // Deletion of this socket is done in the client connection thread
        boost::asio::ip::tcp::socket* socket = new boost::asio::ip::tcp::socket(ioService);
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

void processConnection(World& world, std::queue<std::function<void()>>& queue, std::mutex* queueMutex, boost::asio::ip::tcp::socket* socket, int connectionNumber) {
    std::cout << "Processing connection" << std::endl;
    
    // Send its identification number
    char* connectionBuff = (char *)&connectionNumber;
    boost::asio::write(*socket, boost::asio::buffer(connectionBuff, sizeof(int)));
    
    boost::asio::streambuf buffer;
    std::istream startStream(&buffer);
    
    // Receives Initial state of the vehicle
    std::cout << "Parsing vehicle..." << std::endl;
    ChronoMessages::VehicleMessage* vehicle = new ChronoMessages::VehicleMessage();
    socket->receive(buffer.prepare(512));
    buffer.commit(512);
    vehicle->ParseFromIstream(&startStream);
    buffer.consume(vehicle->ByteSize());
    //std::cout << vehicle->DebugString() << std::endl;
    
    // Pushes addVehicle to queue so that the vehicle may be added to the world
    std::lock_guard<std::mutex>* guard = new std::lock_guard<std::mutex>(*queueMutex);
    queue.push([&world, vehicle] { world.addVehicle(0, 0, *vehicle); });
    delete guard;
    
    while(true) {
        // Responds to client with all other serialized vehicles in the world. Edit demo to add the vehicles to its system.
        int count = world.numVehicles();
        char* countBuff = (char *)&count;
        boost::asio::write(*socket, boost::asio::buffer(countBuff, sizeof(int)));
        std::cout << "Count sent: " << count << std::endl;
        boost::asio::streambuf worldBuffer;
        std::ostream outStream(&worldBuffer);
        
        for(std::pair<const int, ChronoMessages::VehicleMessage> worldPair : world.getSection(0, 0))
            if(worldPair.second.vehicleid() != connectionNumber)
                worldPair.second.SerializeToOstream(&outStream);
        
        if(count > 1)
            boost::asio::write(*socket, worldBuffer);
        
        std::istream inStream(&buffer);
        
        // Receives update on vehicle
        std::cout << "Parsing vehicle..." << std::endl;
        socket->receive(buffer.prepare(512));
        buffer.commit(512);
        vehicle->ParseFromIstream(&inStream);
        buffer.consume(vehicle->ByteSize());
        //std::cout << vehicle->DebugString() << std::endl;
        std::cout << "Debug string should have printed" << std::endl;
        // Pushes updateVehicle to queue to update the vehicle state in the world
        std::lock_guard<std::mutex>* guard = new std::lock_guard<std::mutex>(*queueMutex);
        queue.push([&world, vehicle] { world.updateVehicle(0, 0, *vehicle); });
        delete guard;
    }
}