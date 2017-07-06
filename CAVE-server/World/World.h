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
//	Class used to maintain the current state of the virtual world on the server.
//
// =============================================================================

#ifndef WORLD_H
#define WORLD_H

#include <boost/asio.hpp>
#include <google/protobuf/message.h>

#include "ChronoMessages.pb.h"

// Uniquely identifies any registered endoint in the world.
struct endpointProfile;

class World {
public:
    ~World();

    // Adds new connection number to list of numbers server can receive from.
    // Returns true (success) if the provided number has not already been
    // registered.
    bool registerConnectionNumber(int connectionNumber);

    // Adds new endpoint to list of endpoints the server can receive from,
    // completing the handshake and transfer to udp communication.
    // Returns true (success) if the the provided connection number has been
    // registered and has no corresponding endpoint.
    bool registerEndpoint(boost::asio::ip::udp::endpoint& endpoint, int connectionNumber);

    // Updates existing world element, or adds one if new.
    // Returns true (success) if connectionNumber is the correct owner of the
    // element, and if message is of the same type as the original element.
    bool updateElement(std::shared_ptr<google::protobuf::Message> message, endpointProfile *profile, int idNumber);

    // Updates all the elements in the given profile with the message packet.
    // Efficient, as long as the elemnts are sorted correctly in the packet
    bool updateElementsOfProfile(endpointProfile *profile, std::shared_ptr<google::protobuf::Message> packet);

    // Returns a shared_ptr to the corresponding element. If element does not
    // exist, returns a shared_ptr to NULL.
    std::shared_ptr<google::protobuf::Message> getElement(int connectionNumber, int idNumber);

    // Returns a packet containing all world elements
    std::shared_ptr<ChronoMessages::MessagePacket> generateWorldPacket();

    // Removes and element from the world. Returns true (success) if element
    // exists and connectionNumber is the correct owner.
    bool removeElement(int idNumber, endpointProfile *profile);

    // Removes endpoint and all elements associated with connectionNumber.
    // Returns true (success) if connectionNumber has been registered.
    // Invalidates profile.
    bool removeConnection(endpointProfile *profile);

    // Returns a pointer to a valid profile if connectionNumber is registered
    // with the endpoint. Returns NULL on failure.
    endpointProfile *verifyConnection(int connectionNumber, boost::asio::ip::udp::endpoint endpoint);

    // Number of elements
    int elementCount();

    // Returns the number of elements associated with this endpointProfile
    int profileElementCount(endpointProfile *profile);

    // Number of client connections
    int connectionCount();

private:
    // Set of connection numbers with no endpoints
    std::set<int> registeredConnectionNumbers;
    // Maps connection numbers to endpoints and owned element idNumbers
    std::map<int, endpointProfile *> endpoints;
    // Maps connection number-id number pair to elements in the world
    std::map<std::pair<int, int>, std::shared_ptr<google::protobuf::Message>> elements;
};

class OutOfBoundsException : std::exception {
    virtual const char* what() const throw() {
        return "No element matching given information.";
    }
};

#endif
