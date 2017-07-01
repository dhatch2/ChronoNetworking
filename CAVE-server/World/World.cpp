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
    if (registeredConnectionNumbers.find(connectionNumber) != registeredConnectionNumbers.end()) {
        return false;
    }
    if (endpoints.find(connectionNumber) != endpoints.end()) {
        return false;
    }
    registeredConnectionNumbers.insert(connectionNumber);
    return true;
}

bool World::registerEndpoint(boost::asio::ip::udp::endpoint& endpoint, int connectionNumber) {
    auto num = registeredConnectionNumbers.find(connectionNumber);
    if (num == registeredConnectionNumbers.end()) {
        return false;
    }
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
    if (mess != elements.end() && mess->second->GetDescriptor()->full_name().compare(message->GetDescriptor()->full_name()) != 0) {
        return false;
    } else if (mess == elements.end()) {
        auto empPair = elements.emplace(std::make_pair(std::make_pair(profile->connectionNumber, idNumber), message));
        if (!empPair.second) return false;
        profile->count++;
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
        /*if (mess->first >= profile->last->first) {
            profile->last = ++mess;
        } else if (profile->first == elements.end()) {
            profile->first = mess;
            profile->last = ++mess;
        } else if (mess->first <= profile->first->first) {
            profile->first = mess;
        }*/
        return true;
    }
    elements[std::make_pair(profile->connectionNumber, idNumber)] = message;
    return true;
}

bool World::removeElement(int idNumber, endpointProfile *profile) {
    auto mess = elements.find(std::pair<int, int>(profile->connectionNumber, idNumber));
    if (mess == elements.end()) {
        return false;
    }
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
    if (prof == endpoints.end()) {
        return false;
    }
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
