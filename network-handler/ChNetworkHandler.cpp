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
    shutdown = false;
}

ChNetworkHandler::~ChNetworkHandler() {
    socket.close();
    shutdown = true;
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
    size_t received;
    auto buffer = std::make_shared<boost::asio::streambuf>();
    // Lock Starts
    do {
        std::unique_lock<std::mutex> lock(socketMutex);
        initVar.wait(lock, [&]{ return socket.is_open(); });
        socket.receive(boost::asio::null_buffers(), 0, error);
        //TODO: Handle error message here
        boost::system::error_code availableError;
        int available = socket.available(availableError);
        if (!availableError && available != 0) {
            received = socket.receive_from(buffer->prepare(available), endpoint, 0, error);
        }
        //TODO: Handle error message again here
    } while (error == boost::asio::error::would_block && socket.is_open() && !shutdown);
    buffer->commit(received);
    return std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>(endpoint, buffer);
}

ChClientHandler::ChClientHandler(std::string hostname, std::string port) :
    ChNetworkHandler(),
    sendQueue([&]{ return socket.is_open() || !shutdown; }) {
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
    } catch (std::exception& err) { throw ConnectionException(FAILED_CONNECTION); }
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
        socket.non_blocking(true);
        // TODO: Complete Handshake over udp.
        initVar.notify_one();
    } else throw ConnectionException(UNDETERMINED_CONNECTION);
}

ChClientHandler::~ChClientHandler() {
    shutdown = true;
    sendQueue.dumpThreads();
    socket.close();
    while (socket.is_open() && !shutdown);
    sendQueue.notifyPredicate();
}

int ChClientHandler::connectionNumber() {
    return m_connectionNumber;
}

void ChClientHandler::beginListen() {
    listener = new std::thread([&, this] {
        while (socket.is_open() && !shutdown) {
            auto recPair = receiveMessage();
            //TODO: Handle endpoint information here
            boost::asio::streambuf& buffer = *(recPair.second);
            std::istream stream(&buffer);
            uint8_t messageType;
            stream >> messageType;

            switch (messageType) {
                case MESSAGE_PACKET: {
                    ChronoMessages::MessagePacket packet;
                    packet.ParseFromIstream(&stream);
                    for (size_t i = 0; i < packet.vehiclemessages_size(); i++) {
                        auto vehicleMessage = std::make_shared<ChronoMessages::VehicleMessage>();
                        vehicleMessage->CopyFrom(packet.vehiclemessages(i));
                        simUpdateQueue.enqueue(vehicleMessage);
                    }
                    for (size_t i = 0; i < packet.dsrcmessages_size(); i++) {
                        auto DMessage = std::make_shared<ChronoMessages::DSRCMessage>();
                        DMessage->CopyFrom(packet.dsrcmessages(i));
                        DSRCUpdateQueue.enqueue(DMessage);
                    }
                    break;
                }
                case VEHICLE_MESSAGE: {
                    auto message = std::make_shared<ChronoMessages::VehicleMessage>();
                    message->ParseFromIstream(&stream);
                    simUpdateQueue.enqueue(message);
                    break;
                }
                case DSRC_MESSAGE: {
                    auto message = std::make_shared<ChronoMessages::DSRCMessage>();
                    message->ParseFromIstream(&stream);
                    DSRCUpdateQueue.enqueue(message);
                    break;
                }
                default:
                    // TODO: Deal with gibberish message.
                    break;
            }
        }
    });
}

void ChClientHandler::beginSend() {
    sender = new std::thread([&, this] {
        try {
            while (socket.is_open() && !shutdown) {
                auto buffer = sendQueue.dequeue();
                sendMessage(serverEndpoint, *buffer);
            }
        } catch (PredicateException ex) {
        }
    });
}

void ChClientHandler::pushMessage(google::protobuf::Message& message) {
    uint8_t messageType;
    std::string type = message.GetDescriptor()->full_name();
    if (type.compare(VEHICLE_MESSAGE_TYPE) == 0) messageType = VEHICLE_MESSAGE;
    else if (type.compare(DSRC_MESSAGE_TYPE) == 0) messageType = DSRC_MESSAGE;
    else if (type.compare(MESSAGE_PACKET_TYPE) == 0) messageType = MESSAGE_PACKET;
    // TODO: else throw some exception about how this message type isn't supported.

    auto buffer = std::make_shared<boost::asio::streambuf>();
    std::ostream stream(buffer.get());
    stream << messageType;
    message.SerializeToOstream(&stream);
    stream.flush();

    sendQueue.enqueue(buffer);
}

std::shared_ptr<google::protobuf::Message> ChClientHandler::popSimMessage() {
    return simUpdateQueue.dequeue();
}

std::shared_ptr<ChronoMessages::DSRCMessage> ChClientHandler::popDSRCMessage() {
    return DSRCUpdateQueue.dequeue();
}

ChServerHandler::ChServerHandler(int portNumber) : ChNetworkHandler(),
    sendQueue([&]{ return socket.is_open() || !shutdown; }),
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
            // TODO: Do something with this accept error thing
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
        acceptor.close();
    } ) {
    std::unique_lock<std::mutex> lock(socketMutex);
    socket.open(boost::asio::ip::udp::v4());
    socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), portNumber));
    socket.non_blocking(true);
    initVar.notify_one();
}

ChServerHandler::~ChServerHandler() {
    socket.close();
    acceptor.join();
    while (!sendQueue.empty());
    shutdown = true;
    sendQueue.dumpThreads();
    socket.close();
    while (socket.is_open() && !shutdown);
    sendQueue.notifyPredicate();
}

void ChServerHandler::beginListen() {
    listener = new std::thread([&, this] {
        while (socket.is_open() && !shutdown)
            receiveQueue.enqueue(receiveMessage());
    });
}

void ChServerHandler::beginSend() {
    sender = new std::thread([&, this] {
        try {
            while (socket.is_open() && !shutdown) {
                auto sendPair = sendQueue.dequeue();
                sendMessage(sendPair.first, *sendPair.second);
            }
        } catch (PredicateException& ex) {
            //std::cout << "caught" << std::endl;
        }
    });
}

std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<google::protobuf::Message>> ChServerHandler::popMessage() {
    auto recPair = receiveQueue.dequeue();
    boost::asio::streambuf& buffer = *(recPair.second);
    std::istream stream(&buffer);
    uint8_t messageType;
    stream >> messageType;

    switch (messageType) {
        case MESSAGE_PACKET: {
            auto packet = std::make_shared<ChronoMessages::MessagePacket>();
            packet->ParseFromIstream(&stream);
            return std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<google::protobuf::Message>>(recPair.first, packet);
        }
        case VEHICLE_MESSAGE: {
            auto message = std::make_shared<ChronoMessages::VehicleMessage>();
            message->ParseFromIstream(&stream);
            return std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<google::protobuf::Message>>(recPair.first, message);
        }
        case DSRC_MESSAGE: {
            auto message = std::make_shared<ChronoMessages::DSRCMessage>();
            message->ParseFromIstream(&stream);
            return std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<google::protobuf::Message>>(recPair.first, message);
        }
        default:
            throw CommunicationException(recPair.first);
            break;
    }
}

void ChServerHandler::pushMessage(boost::asio::ip::udp::endpoint& endpoint, google::protobuf::Message& message) {
    uint8_t messageType;
    std::string type = message.GetDescriptor()->full_name();
    if (type.compare(VEHICLE_MESSAGE_TYPE) == 0) messageType = VEHICLE_MESSAGE;
    else if (type.compare(DSRC_MESSAGE_TYPE) == 0) messageType = DSRC_MESSAGE;
    else if (type.compare(MESSAGE_PACKET_TYPE) == 0) messageType = MESSAGE_PACKET;
    // TODO: else throw some exception about how this message type isn't supported.

    auto buffer = std::make_shared<boost::asio::streambuf>();
    std::ostream stream(buffer.get());
    stream << messageType;
    message.SerializeToOstream(&stream);
    stream.flush();

    sendQueue.enqueue(std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>(endpoint, buffer));
}
