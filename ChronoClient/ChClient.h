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

#ifndef CHCLIENT_H
#define CHCLIENT_H

#include <iostream>
#include <thread>
#include <time.h>
#include <google/protobuf/message.h>
#include <boost/asio.hpp>

#include "MessageCodes.h"
#include "ChronoMessages.pb.h"

class ChClient
{
public:
    ChClient(boost::asio::io_service* ioService, double* stepSize);
    ~ChClient();

    // Getter for the connection number
    int connectionNumber();

    // Returns whether or not a connection to a server currently exists
    bool isConnected();

    // Returns -1 if connection is unsuccessful
    int connectToServer(std::string name, std::string port);

    // Returns immediately and uses the socket to receive from server on another thread
    void asyncListen(std::map<int, std::shared_ptr<google::protobuf::Message>>& serverMessages);

    // Blocks until message has been serialized and sent over socket
    void sendMessage(std::shared_ptr<google::protobuf::Message> message);

    // Disconnect from the ChronoServer
    void disconnect();

    // Gets the current heartrate
    double heartrate();

    void Advance(double step);
private:
    boost::asio::io_service* m_ioService;
    boost::asio::ip::tcp::socket m_socket;
    int m_connectionNumber;
    std::shared_ptr<std::thread> listener;
    boost::asio::streambuf m_buff;
    std::ostream m_outStream;
    double m_heartrate;
    double* m_stepSize;
    clock_t lastHeartbeat;
    double secondsElapsed;
    int stepsElapsed;
};

#endif // CHCLIENT_H
