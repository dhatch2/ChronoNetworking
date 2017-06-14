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

#define REFUSED_CONNECTION 0
#define UNDETERMINED_CONNECTION 1
#define FAILED_CONNECTION 2

class ChNetworkHandler {
public:
    ChNetworkHandler();
    virtual ~ChNetworkHandler();

    // Begins receiving messages.
    virtual void beginListen() = 0;

    // Begins sending messages.
    virtual void beginSend() = 0;

protected:
    void sendMessage(boost::asio::ip::udp::endpoint& endpoint, boost::asio::streambuf& message);
    std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>& receiveMessage();
    boost::asio::ip::udp::socket socket;
};

class ChClientHandler : public ChNetworkHandler {
public:
    ChClientHandler(std::string hostname, std::string port);
    ~ChClientHandler();

    bool connectToServer(std::string name, std::string port);
    bool isConnected();
    int connectionNumber();

    // Begins receiving messages.
    void beginListen();

    // Begins sending messages.
    void beginSend();

    // Pushes message to be sent.
    void pushMessage(google::protobuf::Message& message);

    // Returns message related to physical simulation.
    google::protobuf::Message& popSimMessage();

    // Returns simulated DSRC message.
    ChronoMessages::DSRCMessage& popDSRCMessage();

private:
    ChSafeQueue<google::protobuf::Message> simUpdateQueue;
    ChSafeQueue<google::protobuf::Message> DSRCUpdateQueue;
    boost::asio::ip::udp::endpoint serverEndpoint;
    int m_connectionNumber;
};

class ChServerHandler : public ChNetworkHandler {
public:
    ChServerHandler(int portNumber);
    ~ChServerHandler();

    // Begins receiving messages.
    void beginListen();

    // Begins sending messages.
    void beginSend();

    // Returns message recieved from the network.
    std::pair<boost::asio::ip::udp::endpoint, google::protobuf::Message>& popMessage();

    // Pushes message to queue to be sent.
    void pushMessage(boost::asio::ip::udp::endpoint& endpoint, google::protobuf::Message& message);
private:
    ChSafeQueue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>> receiveQueue;
    ChSafeQueue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>> sendQueue;
    std::thread acceptor;
    int connectionCount;
    std::mutex initMutex;
    std::condition_variable initVar;
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

#endif // CHNETWORKHANDLER_H
