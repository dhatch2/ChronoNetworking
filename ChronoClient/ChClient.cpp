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
//	Client-side interface for communication with the TCP-based Chrono Server
//
// =============================================================================

#include "ChClient.h"
#include <iostream>
#include <fstream>
#include <set>
#include <chrono>

std::chrono::time_point<std::chrono::steady_clock> startTime;
std::chrono::time_point<std::chrono::steady_clock> endTime;

ChClient::ChClient(boost::asio::io_service* ioService, double* stepSize)
    : m_socket(*ioService), m_outStream(&m_buff)
{
    m_ioService = ioService;
    m_connectionNumber = -1;
    m_heartrate = 1.0;
    lastHeartbeat = clock();
    m_stepSize = stepSize;
    stepsElapsed = 0;
    secondsElapsed = 0.0;
}

ChClient::~ChClient()
{
    m_socket.close();
    listener->join();
}

int ChClient::connectionNumber() {
    return m_connectionNumber;
}

bool ChClient::isConnected() {
    return m_socket.is_open();
}

int ChClient::connectToServer(std::string name, std::string port) {
    // Connect to server
    boost::asio::ip::tcp::resolver resolver(*m_ioService);
    boost::asio::ip::tcp::resolver::query query(
        name,
        port);  // Change to the correct port and ip address
    try {
        boost::asio::ip::tcp::resolver::iterator endpointIterator = resolver.resolve(query);
        boost::asio::connect(m_socket, endpointIterator);
    } catch(std::exception error) {
        return -1;
    }

    // Receive connection number
    m_socket.read_some(boost::asio::buffer(&m_connectionNumber, sizeof(int)));
    return m_connectionNumber;
}

void ChClient::asyncListen(std::map<int, std::shared_ptr<google::protobuf::Message>>& serverMessages) {
    listener = std::make_shared<std::thread>([&serverMessages, this] {
        std::set<uint32_t> idnumbers;
        //std::ofstream outputFile;
        //outputFile.open("scaling-client-output.csv");
        while (m_socket.is_open()) {
            uint8_t messageCode;
            m_socket.receive(boost::asio::buffer(&messageCode, sizeof(uint8_t)));
            //endTime = std::chrono::steady_clock::now();
            //auto diff = endTime - startTime;
            //outputFile << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() << '\n';
            // std::cout << (int)messageCode << std::endl;

            switch(messageCode) {
                // Normal vehicle message receiving
                case VEHICLE_MESSAGE: {
                    boost::asio::streambuf worldBuffer;
                    std::istream inStream(&worldBuffer);
                    m_socket.receive(worldBuffer.prepare(VEHICLE_MESSAGE_SIZE));
                    worldBuffer.commit(VEHICLE_MESSAGE_SIZE);
                    std::shared_ptr<ChronoMessages::VehicleMessage> worldVehicle = std::make_shared<ChronoMessages::VehicleMessage>();
                    worldVehicle->ParseFromIstream(&inStream);
                    if(serverMessages.find(worldVehicle->idnumber()) == serverMessages.end()) {
                        serverMessages.insert(std::pair<int, std::shared_ptr<google::protobuf::Message>>(worldVehicle->idnumber(), worldVehicle));
                    } else {
                        worldVehicle->GetReflection()->Swap(serverMessages[worldVehicle->idnumber()].get(), worldVehicle.get());
                    }
                    idnumbers.insert(worldVehicle->idnumber());
                    break;
                }
                // For the last vehicle in a group. Removes all vehicles that didn't receive updates.
                case VEHICLE_MESSAGE_END: {
                    for(std::map<int, std::shared_ptr<google::protobuf::Message>>::iterator it = serverMessages.begin(); it != serverMessages.end();) {
                        if(idnumbers.find(it->first) == idnumbers.end()){
                            it = serverMessages.erase(it);
                            std::cout << "Vehicle removed" << std::endl;
                        } else ++it;
                    }
                    idnumbers.erase(idnumbers.begin(), idnumbers.end());
                    break;
                }
                // If the whole message cannot be sent, the server just sends the id so that the client knows to keep the vehicle.
                case VEHICLE_ID: {
                    int32_t id;
                    m_socket.receive(boost::asio::buffer(&id, sizeof(uint32_t)));
                    idnumbers.insert(id);
                    break;
                }
                // Calculate the heartrate from the heartbeat
                case HEARTBEAT: {
                    m_heartrate = (clock() - lastHeartbeat) / (double) CLOCKS_PER_SEC;
                    lastHeartbeat = clock();

                    /*if (secondsElapsed < m_heartrate/10 - 0.001)
                        *m_stepSize += 0.0001;
                    else if (secondsElapsed > m_heartrate/10 + 0.001)
                        *m_stepSize -= 0.0001;*/

                    //std::cout << "Steps elapsed: " << stepsElapsed << std::endl;
                    stepsElapsed = 0;
                    secondsElapsed = 0.0;
                    break;
                }
            }
        }
        //outputFile.close();
    });
}

void ChClient::sendMessage(std::shared_ptr<google::protobuf::Message> message) {
    uint8_t messageCode = VEHICLE_MESSAGE;
    startTime = std::chrono::steady_clock::now();
    m_socket.send(boost::asio::buffer(&messageCode, sizeof(uint8_t)));

    message->SerializeToOstream(&m_outStream);
    boost::asio::write(m_socket, m_buff);
    m_buff.consume(message->ByteSize());
}

void ChClient::disconnect() {
    uint8_t messageCode = DISCONNECT_MESSAGE;
    m_socket.send(boost::asio::buffer(&messageCode, sizeof(uint8_t)));
}

double ChClient::heartrate() {
    return m_heartrate;
}

void ChClient::Advance(double step) {
    secondsElapsed += step;
    stepsElapsed++;
}
