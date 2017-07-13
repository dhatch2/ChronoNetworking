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

#include "World.h"
#include "MessageCodes.h"
#include <iostream>

struct endpointProfile {
    int connectionNumber;
    int count;
    boost::asio::ip::udp::endpoint endpoint;
    std::map<std::pair<int, int>, std::shared_ptr<google::protobuf::Message>>::iterator first;
    std::map<std::pair<int, int>, std::shared_ptr<google::protobuf::Message>>::iterator last;
};

World::~World() {
    for (auto endpointPair : endpoints) {
        delete endpointPair.second;
    }
}

bool World::registerConnectionNumber(int connectionNumber) {
    // connectionNumber can't already be registered
    if (registeredConnectionNumbers.find(connectionNumber) != registeredConnectionNumbers.end()) {
        return false;
    }
    // An endpoint can't already be registered under connectionNumber
    if (endpoints.find(connectionNumber) != endpoints.end()) {
        return false;
    }
    registeredConnectionNumbers.insert(connectionNumber);
    return true;
}

bool World::registerEndpoint(boost::asio::ip::udp::endpoint& endpoint, int connectionNumber) {
    auto num = registeredConnectionNumbers.find(connectionNumber);
    // connectionNumber has to be registered
    if (num == registeredConnectionNumbers.end()) {
        return false;
    }
    // An endpoint can't already be registered under connectionNumber
    if (endpoints.find(connectionNumber) != endpoints.end()) {
        return false;
    }
    registeredConnectionNumbers.erase(num);
    endpointProfile *profile = new endpointProfile;
    profile->connectionNumber = connectionNumber;
    profile->count = 0;
    profile->endpoint = endpoint;
    profile->first = elements.end();
    profile->last = elements.end();
    endpoints[connectionNumber] = profile;
    return true;
}

bool World::updateElement(std::shared_ptr<google::protobuf::Message> message, endpointProfile *profile, int idNumber) {
    auto mess = elements.find(std::make_pair(profile->connectionNumber, idNumber));
    // The update must be of the same type as the original message
    if (mess != elements.end() && mess->second->GetDescriptor()->full_name().compare(message->GetDescriptor()->full_name()) != 0) {
        return false;
        // Else adds the update as a new element if not already found
    } else if (mess == elements.end()) {
        auto empPair = elements.insert(std::make_pair(std::make_pair(profile->connectionNumber, idNumber), message));
        if (!empPair.second) return false;
        profile->count++;
        // Moves profile's iterators to re-encompass it's owned elements
        mess = empPair.first;
        auto curr = profile->first;
        while (curr->first.first == profile->connectionNumber) {
            curr--;
        }
        profile->first = ++curr;
        curr = profile->last;
        while (curr->first.first == profile->connectionNumber) {
            curr++;
        }
        profile->last = --curr;
        return true;
    }
    elements[std::make_pair(profile->connectionNumber, idNumber)]->CopyFrom(*message);
    return true;
}

bool World::updateElementsOfProfile(endpointProfile *profile, std::shared_ptr<google::protobuf::Message> message) {
    // TODO: Handle cases of duplicate idNumbers and extra messages after all pre-existing elements have been updated.
    if (message->GetDescriptor()->full_name().compare(MESSAGE_PACKET_TYPE)) return false;
    auto packet = std::static_pointer_cast<ChronoMessages::MessagePacket>(message);
    auto finish = profile->first;
    finish--;
    for (auto curr = profile->last; curr != finish; curr--) {
        std::shared_ptr<google::protobuf::Message>& message = curr->second;
        std::string type = message->GetDescriptor()->full_name();
        int idNumber = curr->first.second;
        if (type.compare(VEHICLE_MESSAGE_TYPE) == 0) {
            ChronoMessages::VehicleMessage *vehicle;
            if (packet->vehiclemessages_size() > 0) {
                vehicle = *(--packet->mutable_vehiclemessages()->pointer_end());
            } else vehicle = NULL;
            // Adds vehicle if not already present in elements
            while (vehicle != NULL && vehicle->idnumber() > idNumber) {
                std::shared_ptr<ChronoMessages::VehicleMessage> vehiclePtr;
                vehiclePtr.reset(packet->mutable_vehiclemessages()->ReleaseLast());
                updateElement(vehiclePtr, profile, vehicle->idnumber());
                if (packet->vehiclemessages_size() > 0) {
                    vehicle = *(--packet->mutable_vehiclemessages()->pointer_end());
                } else vehicle = NULL;
            }
            // Updates vehicle if present
            if (vehicle != NULL && vehicle->idnumber() == idNumber) {
                message.reset(packet->mutable_vehiclemessages()->ReleaseLast());
            } else {
                removeElement(idNumber, profile);
            }
        } else return false;
    }
    // Adds remaining vehicles
    while (!packet->vehiclemessages().empty()) {
        std::shared_ptr<ChronoMessages::VehicleMessage> vehiclePtr;
        vehiclePtr.reset(packet->mutable_vehiclemessages()->ReleaseLast());
        updateElement(vehiclePtr, profile, vehiclePtr->idnumber());
    }
    return true;
}

std::shared_ptr<google::protobuf::Message> World::getElement(int connectionNumber, int idNumber) {
    auto el = elements.find(std::make_pair(connectionNumber, idNumber));
    if (el != elements.end()) {
        return el->second;
    } else {
        // Throws if this element doesn't exist
        throw OutOfBoundsException();
    }
}

std::shared_ptr<ChronoMessages::MessagePacket> World::generateWorldPacket() {
    auto packet = std::make_shared<ChronoMessages::MessagePacket>();
    packet->set_connectionnumber(-1);
    // Iterate through every element and add it to the packet
    for (auto curr : elements) {
        std::string type = curr.second->GetDescriptor()->full_name();
        if (type.compare(VEHICLE_MESSAGE_TYPE) == 0) {
            packet->add_vehiclemessages()->MergeFrom(*curr.second);
        }
    }
    return packet;
}

bool World::removeElement(int idNumber, endpointProfile *profile) {
    auto mess = elements.find(std::pair<int, int>(profile->connectionNumber, idNumber));
    // Element to be removed must be present
    if (mess == elements.end()) {
        return false;
    }
    // Move profile's iterators if needed
    if (mess == profile->last) {
        profile->last--;
        mess--;
    }
    if (mess == profile->first) {
        profile->first++;
    }
    elements.erase(mess);
    profile->count--;
    return true;
}

bool World::removeConnection(endpointProfile *profile) {
    auto prof = endpoints.find(profile->connectionNumber);
    // Profile must be registered to be removed
    if (prof == endpoints.end()) {
        return false;
    }
    // Removes all owned elements
    elements.erase(profile->first, ++profile->last);
    endpoints.erase(prof);
    delete profile;
    return true;
}

endpointProfile *World::verifyConnection(int connectionNumber, boost::asio::ip::udp::endpoint endpoint) {
    auto prof = endpoints.find(connectionNumber);
    if (prof == endpoints.end()) return NULL;
    return prof->second;
}

int World::elementCount() {
    return elements.size();
}

int World::profileElementCount(endpointProfile *profile) {
    return profile->count;
}

int World::connectionCount() {
    return endpoints.size();
}
