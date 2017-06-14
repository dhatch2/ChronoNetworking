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
//	Class definition for the client ans server network interfaces. All
//  communication between the client and server is done with instances of this a
//  child of ChNetworkHandler. Instances of this class may create additional
//  threads for network communication purposes.
//
// =============================================================================

#include "ChNetworkHandler.h"
#include "MessageCodes.h"

#include <iostream>

ChNetworkHandler::ChNetworkHandler() : socket(*(new boost::asio::io_service)) {

}

ChNetworkHandler::~ChNetworkHandler() {
    socket.close();
    delete &socket.get_io_service();
}

//TODO: make sure to send buffer size BEFORE the buffer so that the receiving end knows how much to receive.
void ChNetworkHandler::sendMessage(boost::asio::ip::udp::endpoint& endpoint, boost::asio::streambuf& message) {
    std::unique_lock<std::mutex> lock(initMutex);
    initVar.wait(lock, [&]{ return socket.is_open(); });
    boost::system::error_code error;
    socket.send_to(boost::asio::buffer(&message, message.size()), endpoint, 0, error);
    if (error == boost::asio::error::host_not_found) {
        //TODO: Handle event by removing endpoint from world object or something
    }
}

//TODO: make sure receive size first so we know how much to receive.
std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>> ChNetworkHandler::receiveMessage() {
    std::unique_lock<std::mutex> lock(initMutex);
    initVar.wait(lock, [&]{ return socket.is_open(); });
    boost::system::error_code error;
    boost::asio::ip::udp::endpoint endpoint;
    auto buffer = std::make_shared<boost::asio::streambuf>();
    socket.receive_from(boost::asio::buffer(buffer.get(), buffer->size()), endpoint, 0, error);
    //TODO: Handle error message here

    return std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>(endpoint, buffer);
}

ChClientHandler::ChClientHandler(std::string hostname, std::string port) : ChNetworkHandler() {
    std::unique_lock<std::mutex> lock(initMutex);
    boost::asio::ip::tcp::resolver tcpResolver(socket.get_io_service());
    boost::asio::ip::tcp::resolver::query tcpQuery(hostname, port);
    uint8_t requestResponse;
    boost::asio::ip::tcp::socket tcpSocket(socket.get_io_service());
    try {
        boost::asio::ip::tcp::resolver::iterator endpointIterator = tcpResolver.resolve(tcpQuery);
        boost::asio::connect(tcpSocket, endpointIterator);
        uint8_t connectionRequest = CONNECTION_REQUEST;
        tcpSocket.send(boost::asio::buffer(&connectionRequest, sizeof(uint8_t)));
        tcpSocket.receive(boost::asio::buffer(&requestResponse, sizeof(uint8_t)));
    } catch (std::exception& err) {
        throw ConnectionException(FAILED_CONNECTION);
    }
    if(requestResponse == CONNECTION_DECLINE) throw ConnectionException(REFUSED_CONNECTION);
    if(requestResponse == CONNECTION_ACCEPT) {
        uint32_t connectionNumber;
        tcpSocket.receive(boost::asio::buffer(&connectionNumber, sizeof(uint32_t)));
        tcpSocket.close();
        m_connectionNumber = connectionNumber;

        boost::asio::ip::udp::resolver udpResolver(socket.get_io_service());
        boost::asio::ip::udp::resolver::query udpQuery(boost::asio::ip::udp::v4(), hostname, port);
        serverEndpoint = *udpResolver.resolve(udpQuery);
        socket.open(boost::asio::ip::udp::v4());
    } else throw ConnectionException(UNDETERMINED_CONNECTION);
}

ChClientHandler::~ChClientHandler() {
    //socket.close();
    //delete &socket.get_io_service();
}

int ChClientHandler::connectionNumber() {
    return m_connectionNumber;
}

void ChClientHandler::beginListen() {

}

void ChClientHandler::beginSend() {

}

ChServerHandler::ChServerHandler(int portNumber) : ChNetworkHandler(),
    acceptor([&, this] {
        std::unique_lock<std::mutex> lock(initMutex);
        initVar.wait(lock, [&]{ return socket.is_open(); });
        connectionCount = 0;

        boost::asio::ip::tcp::acceptor acceptor(socket.get_io_service(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8082));
        acceptor.non_blocking(true);

        lock.unlock();

        while (socket.is_open()) {
            boost::asio::ip::tcp::socket tcpSocket(socket.get_io_service());
            boost::system::error_code acceptError;
            acceptor.accept(tcpSocket, acceptError);

            if (acceptError != boost::asio::error::would_block){
                uint8_t requestMessage;
                tcpSocket.receive(boost::asio::buffer(&requestMessage, sizeof(uint8_t)));
                if (requestMessage == CONNECTION_REQUEST) {
                    uint8_t acceptMessage = CONNECTION_ACCEPT;
                    tcpSocket.send(boost::asio::buffer(&acceptMessage, sizeof(uint8_t)));
                    tcpSocket.send(boost::asio::buffer((uint32_t *)(&connectionCount), sizeof(uint32_t)));
                    boost::asio::ip::tcp::endpoint tcpEndpoint = tcpSocket.remote_endpoint();
                    boost::asio::ip::udp::endpoint udpEndpoint(tcpEndpoint.address(), tcpEndpoint.port());
                    // TODO: Add udp endpoint information to world object map.
                    connectionCount++;
                } else {
                    uint8_t declineMessage = CONNECTION_DECLINE;
                    tcpSocket.send(boost::asio::buffer(&declineMessage, sizeof(uint8_t)));
                }
            }
            tcpSocket.close();
        }
    } ) {
    std::unique_lock<std::mutex> lock(initMutex);
    socket.open(boost::asio::ip::udp::v4());
    socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), portNumber));
    socket.non_blocking(true);
}

ChServerHandler::~ChServerHandler() {
    socket.close();
    acceptor.join();
}

void ChServerHandler::beginSend() {

}

void ChServerHandler::beginListen() {

}
