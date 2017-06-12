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
//	Class declaration for the client network interface. All communication with
//  the chrono server is done with instances of this class. Instances of this
//  class may create additional threads for network communication purposes.
//
// =============================================================================

#ifndef CHNETWORKHANDLER_H
#define CHNETWORKHANDLER_H

#include <google/protobuf/message.h>
#include <boost/asio.hpp>

#include "MessageCodes.h"
#include "ChronoMessages.pb.h"
#include "ChSafeQueue.h"

class ChNetworkHandler {
public:

    // Begins receiving messages.
    virtual void beginListen();

    // Begins sending messages.
    virtual void beginSend();

protected:
    void sendMessage(boost::asio::ip::udp::endpoint& endpoint, boost::asio::streambuf& message);
    std::pair<boost::asio::ip::udp::endpoint, boost::asio::streambuf>& receiveMessage();
};

class ChClientHandler : public ChNetworkHandler {
public:
    bool connectToServer(std::string name, std::string port);
    bool isConnected();
    int connectionNumber();

    // Pushes message to be sent.
    void pushMessage(google::protobuf::Message& message);

    // Returns message related to physical simulation.
    google::protobuf::Message& popSimMessage();

    // Returns simulated DSRC message.
    ChronoMessages::DSRCMessage& popDSRCMessage();

private:
    ChSafeQueue<google::protobuf::Message> simUpdateQueue;
    ChSafeQueue<google::protobuf::Message> DSRCUpdateQueue;
    boost::asio::ip::udp::endpoint& serverEndpoint;
    int m_connectionNumber;
};

class ChServerHandler : ChNetworkHandler {
public:
    // Returns message recieved from the network.
    std::pair<boost::asio::ip::udp::endpoint, google::protobuf::Message>& popMessage();

    // Pushes message to queue to be sent.
    void pushMessage(boost::asio::ip::udp::endpoint& endpoint, google::protobuf::Message& message);
private:
    ChSafeQueue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>> receiveQueue;
    ChSafeQueue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>> sendQueue;
};

#endif // CHNETWORKHANDLER_H
