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
    listener = nullptr;
    sender = nullptr;
}

ChNetworkHandler::~ChNetworkHandler() {
    socket.close();
    delete &socket.get_io_service();
    if (listener != nullptr) {
        listener->join();
        delete listener;
    }
    if (sender != nullptr) {
        sender->join();
        delete sender;
    }
}

void ChNetworkHandler::sendMessage(boost::asio::ip::udp::endpoint& endpoint, boost::asio::streambuf& message) {
    boost::system::error_code error;
    // Lock Starts
    std::unique_lock<std::mutex> lock(socketMutex);
    initVar.wait(lock, [&] { return socket.is_open(); });
    socket.send_to(message.data(), endpoint, 0, error);
    // Lock Ends
    lock.unlock();
    if (error == boost::asio::error::host_not_found) {
        //TODO: Handle event by removing endpoint from world object or something
    }
}

std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>> ChNetworkHandler::receiveMessage() {
    boost::system::error_code error;
    boost::asio::ip::udp::endpoint endpoint;
    auto buffer = std::make_shared<boost::asio::streambuf>();
    // Lock Starts
    std::unique_lock<std::mutex> lock(socketMutex);
    initVar.wait(lock, [&]{ return socket.is_open(); });

    socket.receive(boost::asio::null_buffers(), 0, error);
    //TODO: Handle error message here
    int available = socket.available();
    socket.receive_from(buffer->prepare(available), endpoint, 0, error);
    //TODO: Handle error message again here
    // Lock Ends
    lock.unlock();

    return std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>(endpoint, buffer);
}

ChClientHandler::ChClientHandler(std::string hostname, std::string port) : ChNetworkHandler() {
    std::unique_lock<std::mutex> lock(socketMutex);
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
        //delete &socket.get_io_service();
        throw ConnectionException(FAILED_CONNECTION);
    }
    if(requestResponse == CONNECTION_DECLINE) {
        throw ConnectionException(REFUSED_CONNECTION);
    }
    if(requestResponse == CONNECTION_ACCEPT) {
        uint32_t connectionNumber;
        tcpSocket.receive(boost::asio::buffer(&connectionNumber, sizeof(uint32_t)));
        tcpSocket.close();
        m_connectionNumber = connectionNumber;

        boost::asio::ip::udp::resolver udpResolver(socket.get_io_service());
        boost::asio::ip::udp::resolver::query udpQuery(boost::asio::ip::udp::v4(), hostname, port);
        serverEndpoint = *udpResolver.resolve(udpQuery);
        socket.open(boost::asio::ip::udp::v4());
    } else {
        throw ConnectionException(UNDETERMINED_CONNECTION);
    }
}

ChClientHandler::~ChClientHandler() {

}

int ChClientHandler::connectionNumber() {
    return m_connectionNumber;
}

void ChClientHandler::beginListen() {
    listener = new std::thread([&, this] {
        auto recPair = receiveMessage();
        //TODO: Handle endpoint information here

        boost::asio::streambuf& buffer = *(recPair.second);
        std::istream stream(&buffer);
        buffer.commit(sizeof(uint8_t));
        uint8_t messageType;
        stream >> messageType;

        //switch (messageType) {
            //case MESSAGE_PACKET:
                //TODO: This is currently broken
                /*buffer.commit(sizeof(uint32_t));
                uint32_t size;
                stream >> size;
                buffer.commit(size);
                ChronoMessages::MessagePacket packet;
                packet.ParseFromIstream(&stream);
                for (size_t i = 0; i < packet.vehiclemessages_size(); i++) {
                    //simUpdateQueue.enqueue(packet.vehiclemessages(i));
                }*/
        //}
    });
}

void ChClientHandler::beginSend() {

}

ChServerHandler::ChServerHandler(int portNumber) : ChNetworkHandler(),
    acceptor([&, this] {
        std::unique_lock<std::mutex> lock(socketMutex);
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
    std::unique_lock<std::mutex> lock(socketMutex);
    socket.open(boost::asio::ip::udp::v4());
    socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), portNumber));
    socket.non_blocking(true);
}

ChServerHandler::~ChServerHandler() {
    socket.close();
    acceptor.join();
}

void ChServerHandler::beginListen() {

}

void ChServerHandler::beginSend() {

}
