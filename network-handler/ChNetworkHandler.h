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
//	Class declaration for the client ans server network interfaces. All
//  communication between the client and server is done with instances of this a
//  child of ChNetworkHandler. Instances of this class may create additional
//  threads for network communication purposes, all of which are bound by the
//  lifetime of the ChNetworkHandler.
//
// =============================================================================

#ifndef CHNETWORKHANDLER_H
#define CHNETWORKHANDLER_H

#include <google/protobuf/message.h>
#include <boost/asio.hpp>
#include <exception>
#include <thread>

#include "MessageCodes.h"
#include "ChronoMessages.pb.h"
#include "ChSafeQueue.h"
#include "World.h"

#define REFUSED_CONNECTION 0
#define UNDETERMINED_CONNECTION 1
#define FAILED_CONNECTION 2

class ChNetworkHandler {
public:
    ChNetworkHandler();
    ~ChNetworkHandler();

    // Begins receiving messages.
    virtual void beginListen() = 0;

    // Begins sending messages.
    virtual void beginSend() = 0;

protected:
    // Sends message in buffer
    void sendMessage(boost::asio::ip::udp::endpoint& endpoint, boost::asio::streambuf& message);

    // Returns buffer with message waiting to be commited to the input sequence
    std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>> receiveMessage();

    boost::asio::ip::udp::socket socket;
    std::mutex socketMutex;
    std::condition_variable initVar;
    std::thread* listener;
    std::thread* sender;
    bool shutdown;
};

class ChClientHandler : public ChNetworkHandler {
public:
    ChClientHandler(std::string hostname, std::string port);
    ~ChClientHandler();

    bool isConnected();
    int connectionNumber();

    // Begins receiving messages.
    void beginListen();

    // Begins sending messages.
    void beginSend();

    // Pushes message to be sent.
    void pushMessage(google::protobuf::Message& message);

    // Returns message related to physical simulation.
    std::shared_ptr<google::protobuf::Message> popSimMessage();

    // Returns the number of waiting simulation messages.
    int waitingMessages();

    // Returns simulated DSRC message.
    std::shared_ptr<ChronoMessages::DSRCMessage> popDSRCMessage();

private:
    ChSafeQueue<std::shared_ptr<boost::asio::streambuf>> sendQueue;
    ChSafeQueue<std::shared_ptr<google::protobuf::Message>> simUpdateQueue;
    ChSafeQueue<std::shared_ptr<ChronoMessages::DSRCMessage>> DSRCUpdateQueue;
    boost::asio::ip::udp::endpoint serverEndpoint;
    int m_connectionNumber;
};

class ChServerHandler : public ChNetworkHandler {
public:
    ChServerHandler(World& world, ChSafeQueue<std::function<void()>>& worldQueue, unsigned short portNumber);
    ~ChServerHandler();

    // Begins receiving messages.
    void beginListen();

    // Begins sending messages.
    void beginSend();

    // Returns message recieved from the network.
    std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<google::protobuf::Message>> popMessage();

    // Pushes message to queue to be sent.
    void pushMessage(boost::asio::ip::udp::endpoint& endpoint, google::protobuf::Message& message);
private:
    ChSafeQueue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>> receiveQueue;
    ChSafeQueue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>> sendQueue;
    std::thread acceptor;
    int connectionCount;
};

class ConnectionException : public std::exception {
public:
    ConnectionException(int type) : std::exception() {
        m_type = type;
    }

    int type() { return m_type; }

    virtual const char* what() const throw() {
        switch (m_type) {
            case REFUSED_CONNECTION:
                return "Connection refuesed by server.";
            case FAILED_CONNECTION:
                return "Failed to connect to host.";
            case UNDETERMINED_CONNECTION:
                return "Undetermined connection made.";
            default:
                return "Unknown error.";
        }
    }

private:
    int m_type;
};

class CommunicationException : public std::exception {
public:
    CommunicationException(boost::asio::ip::udp::endpoint endpoint) {
        m_endpoint = endpoint;
    }

    boost::asio::ip::udp::endpoint endpoint() { return m_endpoint; }

    virtual const char* what() const throw() {
        std::stringstream buffer;
        buffer << "Cannot parse message from endpoint " << m_endpoint;
        return buffer.str().c_str();
    }

private:
    boost::asio::ip::udp::endpoint m_endpoint;
};

#endif // CHNETWORKHANDLER_H
