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
    ChNetworkHandler();
    ~ChNetworkHandler();

    int connectionNumber();
    bool isConnected();
    bool connectToServer(std::string name, std::string port);
    void beginListen();
    void beginSend();
    void setSimUpdateQueue(ChSafeQueue& queue);
    void setDSRCUpdateQueue(ChSafeQueue& queue);
    void pushMessage(google::protobuf::Message& message);
}

#endif // CHNETWORKHANDLER_H
