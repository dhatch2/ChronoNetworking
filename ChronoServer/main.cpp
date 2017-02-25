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
//	Main file for the TCP-based Chrono Server
//
// =============================================================================

#include <iostream>
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <typeinfo>
#include <chrono>
#include <time.h>
#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>

#include "World.h"
#include "ChronoMessages.pb.h"
#include "MessageCodes.h"

#define PORT_NUMBER 8082
#define HEARTBEAT_LENGTH 1

int messagesRecieved = 0;

// Start of the listener thread
void listenForConnection(World& world, std::queue<std::function<void()>>& queue, std::shared_ptr<std::mutex> queueMutex, std::shared_ptr<std::condition_variable> emptyqueueCV);

// Start of the client connection handling thread
void processConnection(World& world, std::queue<std::function<void()>>& queue, std::shared_ptr<std::mutex> queueMutex, std::shared_ptr<std::condition_variable> emptyqueueCV, std::shared_ptr<boost::asio::ip::tcp::socket> socket, int connectionNumber);

// Sends the heartbeat to all clients, independent of their thread states
void regulateHeartbeat(std::vector<std::shared_ptr<boost::asio::ip::tcp::socket>>& sockets);

int main(int argc, char **argv)
{
    std::queue<std::function<void()>> worldQueue;
    std::shared_ptr<std::mutex> queueMutex = std::make_shared<std::mutex>();
    std::shared_ptr<std::condition_variable> emptyqueueCV = std::make_shared<std::condition_variable>();

    World world(1, 1);

    std::thread listener(listenForConnection, std::ref(world), std::ref(worldQueue), queueMutex, emptyqueueCV);

    //std::cout << "World queue processing is about to start." << std::endl;
    while(true) {
        std::unique_lock<std::mutex> lck(*queueMutex);
        while (worldQueue.empty()) emptyqueueCV->wait(lck);
        worldQueue.front()();
        worldQueue.pop();
    }
    listener.join();
    return 0;
}

void listenForConnection(World& world, std::queue<std::function<void()>>& queue, std::shared_ptr<std::mutex> queueMutex, std::shared_ptr<std::condition_variable> emptyqueueCV) {
    //std::cout << "Listener thread started" << std::endl;

    // Used to give vehicles unique id numbers
    int connectionNumber = 0;

    // Setting up acceptor
    std::vector<std::thread> clientConnections;
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::acceptor acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER));

    // Setting up a vector for the sockets
    std::vector<std::shared_ptr<boost::asio::ip::tcp::socket>> sockets;

    // Making function for listener threads
    auto connectionFunc = processConnection;

    // Making thread for the heartbeat
    auto regulate = regulateHeartbeat;
    std::thread pacer(regulate, std::ref(sockets));

    while(true) {
        // Deletion of this socket is done automatically
        std::shared_ptr<boost::asio::ip::tcp::socket> socket = std::make_shared<boost::asio::ip::tcp::socket>(ioService);
        //std::cout << "About to listen" << std::endl;
        acceptor.accept(*socket);
        sockets.push_back(socket);
        //std::cout << "Accepted Connection" << std::endl;

        // Creates new thread for the client connection here
        clientConnections.emplace_back(connectionFunc, std::ref(world), std::ref(queue), queueMutex, emptyqueueCV, socket, connectionNumber);
        connectionNumber++;
    }
    // Join heartbeat
    pacer.join();

    // Joins all the threads back to the listener thread
    for(uint i = 0; i < clientConnections.size(); i++)
        clientConnections[i].join();
}

void regulateHeartbeat(std::vector<std::shared_ptr<boost::asio::ip::tcp::socket>>& sockets) {
    // Making timer for heartbeat
    try {
        while(true) {
            std::this_thread::sleep_for(std::chrono::seconds(HEARTBEAT_LENGTH));
            for(std::shared_ptr<boost::asio::ip::tcp::socket> socket : sockets) {
                uint8_t messageCode = HEARTBEAT;
                if(socket->is_open())
                    socket->send(boost::asio::buffer(&messageCode, sizeof(uint8_t)));
            }
        }
    } catch (std::exception& error) {
        //std::cout << "regulateHeartbeat: " << error.what() << std::endl;
    }
}

void processConnection(World& world, std::queue<std::function<void()>>& queue, std::shared_ptr<std::mutex> queueMutex, std::shared_ptr<std::condition_variable> emptyqueueCV, std::shared_ptr<boost::asio::ip::tcp::socket> socket, int connectionNumber) {
    //std::cout << "Processing connection" << std::endl;

    try {
        // Send its identification number
        char* connectionBuff = (char *)&connectionNumber;
        boost::asio::write(*socket, boost::asio::buffer(connectionBuff, sizeof(int)));

        uint8_t receivingCode;
        boost::asio::streambuf buffer;
        std::istream startStream(&buffer);

        // Get start time
        auto startTime = std::chrono::steady_clock::now();

        // Receives Initial state of the vehicle
        socket->receive(boost::asio::buffer(&receivingCode, sizeof(uint8_t)));

        std::shared_ptr<ChronoMessages::VehicleMessage> newVehicle = std::make_shared<ChronoMessages::VehicleMessage>();
        socket->receive(buffer.prepare(361));
        buffer.commit(361);
        newVehicle->ParseFromIstream(&startStream);
        buffer.consume(newVehicle->ByteSize());

        // Pushes addVehicle to queue so that the vehicle may be added to the world
        // The client only has one chance to identify it's connection number correctly
        if(newVehicle->vehicleid() == connectionNumber) {
            {
                std::lock_guard<std::mutex> guard(*queueMutex);
                queue.push([&world, newVehicle] { world.addVehicle(0, 0, newVehicle); });
                emptyqueueCV->notify_all();
            }

            while(world.getSection(0, 0).find(connectionNumber) == world.getSection(0, 0).end());
        } else socket->close();

        while(socket->is_open()) {
            // Responds to client with all other serialized vehicles in the world. Edit demo to add the vehicles to its system.
            std::shared_ptr<ChronoMessages::VehicleMessage> vehicle = std::make_shared<ChronoMessages::VehicleMessage>();

            uint8_t messageCode = VEHICLE_MESSAGE;
            std::map<int, std::shared_ptr<ChronoMessages::VehicleMessage>>& section = world.getSection(0, 0);
            for(std::pair<const int, std::shared_ptr<ChronoMessages::VehicleMessage>> worldPair : section) {
                if(worldPair.first != connectionNumber) {
                    if(worldPair.second->IsInitialized()) {
                        messageCode = VEHICLE_MESSAGE;
                        socket->send(boost::asio::buffer(&messageCode, sizeof(uint8_t)));

                        boost::asio::streambuf worldBuffer;
                        std::ostream outStream(&worldBuffer);
                        worldPair.second->SerializeToOstream(&outStream);
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
            // Get end time
            //static std::mutex mtx;
            //auto endTime = std::chrono::steady_clock::now();
            //auto diff = endTime - startTime;
            //{std::unique_lock<std::mutex> l{mtx};
            //std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() << '\n';}

            // Get start time
            //startTime = std::chrono::steady_clock::now();

            std::istream inStream(&buffer);

            // Receives update message
            socket->receive(boost::asio::buffer(&receivingCode, sizeof(uint8_t)));

            switch(receivingCode) {
                case VEHICLE_MESSAGE: {
                    socket->receive(buffer.prepare(VEHICLE_MESSAGE_SIZE));
                    buffer.commit(VEHICLE_MESSAGE_SIZE);
                    vehicle->ParseFromIstream(&inStream);
                    buffer.consume(vehicle->ByteSize());

                    // Pushes updateVehicle to queue to update the vehicle state in the world
                    // If the id does not correspond to the connection number, it is not updated
                    if(vehicle->vehicleid() == connectionNumber) {
                        std::lock_guard<std::mutex> guard(*queueMutex);
                        queue.push([&world, vehicle] { world.updateVehicle(0, 0, vehicle); messagesRecieved++; });
                        emptyqueueCV->notify_all();
                    } else std::cout << "Update Error" << std::endl;
                    break;
                }

                case DISCONNECT_MESSAGE: {
                    {
                        std::lock_guard<std::mutex> guard(*queueMutex);
                        queue.push([&world, connectionNumber] { world.removeVehicle(0, 0, connectionNumber); });
                        emptyqueueCV->notify_all();
                    }
                    socket->close();
                    std::cout << "Vehicle Removed" << std::endl;
                    break;
                }
            }
        }
    } catch (std::exception& error) {
        std::cout << "processConnection: " << error.what() << std::endl;
        {
            std::lock_guard<std::mutex> guard(*queueMutex);
            queue.push([&world, connectionNumber] { world.removeVehicle(0, 0, connectionNumber); });
            emptyqueueCV->notify_all();
        }
        socket->close();
    }
}
