#include "ChNetworkHandler.h"
#include "MessageCodes.h"

#include <iostream>

ChNetworkHandler::ChNetworkHandler(int portNumber) : socket(*(new boost::asio::io_service), boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), portNumber)) {

}

ChNetworkHandler::~ChNetworkHandler() {
    socket.close();
    delete &socket.get_io_service();
}

void ChNetworkHandler::sendMessage(boost::asio::ip::udp::endpoint& endpoint, boost::asio::streambuf& message) {

}

std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>& ChNetworkHandler::receiveMessage() {

}

ChClientHandler::ChClientHandler(std::string hostname, std::string port) : ChNetworkHandler(std::stoi(port)) {
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
        throw ConnectionException(FAILED_CONNECTION);
    }
    if(requestResponse == CONNECTION_DECLINE) throw ConnectionException(REFUSED_CONNECTION);
    if(requestResponse == CONNECTION_ACCEPT) {
        uint32_t connectionNumber;
        tcpSocket.receive(boost::asio::buffer(&connectionNumber, sizeof(uint32_t)));
        tcpSocket.close();
        m_connectionNumber = connectionNumber;

        boost::asio::ip::udp::resolver udpResolver(socket.get_io_service());
        boost::asio::ip::udp::resolver::query udpQuery(boost::asio::ip::udp::v4(), hostname, port);
        serverEndpoint = *udpResolver.resolve(udpQuery);
    } else throw ConnectionException(UNDETERMINED_CONNECTION);
}

ChClientHandler::~ChClientHandler() {

}

int ChClientHandler::connectionNumber() {
    return m_connectionNumber;
}

void ChClientHandler::beginListen() {

}

void ChClientHandler::beginSend() {

}

ChServerHandler::ChServerHandler(int portNumber) : ChNetworkHandler(portNumber),
    acceptor([&, this] {
        std::unique_lock<std::mutex> lock(initMutex);
        initVar.wait(lock, [&]{ return socket.is_open(); });
        connectionCount = 0;
        boost::asio::ip::tcp::acceptor acceptor(socket.get_io_service(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), portNumber));
        lock.unlock();
        std::cout << "listening..." << std::endl;
        while (socket.is_open()) {
            boost::asio::ip::tcp::socket tcpSocket(socket.get_io_service());
            std::cout << "accepting..." << std::endl;
            acceptor.accept(tcpSocket);

            uint8_t requestMessage;
            std::cout << "receiving..." << std::endl;
            tcpSocket.receive(boost::asio::buffer(&requestMessage, sizeof(uint8_t)));
            std::cout << "received" << std::endl;
            if (requestMessage == CONNECTION_REQUEST) {
                uint8_t acceptMessage = CONNECTION_ACCEPT;
                tcpSocket.send(boost::asio::buffer(&acceptMessage, sizeof(uint8_t)));
                tcpSocket.send(boost::asio::buffer((uint32_t *)(&connectionCount), sizeof(uint32_t)));
                boost::asio::ip::tcp::endpoint tcpEndpoint = tcpSocket.remote_endpoint();
                boost::asio::ip::udp::endpoint udpEndpoint(tcpEndpoint.address(), tcpEndpoint.port());
                connectionCount++;
            } else {
                uint8_t declineMessage = CONNECTION_DECLINE;
                tcpSocket.send(boost::asio::buffer(&declineMessage, sizeof(uint8_t)));
            }
            tcpSocket.close();
        }
    } ) {
    std::unique_lock<std::mutex> lock(initMutex);
}

ChServerHandler::~ChServerHandler() {
    acceptor.join();
}

void ChServerHandler::beginSend() {

}

void ChServerHandler::beginListen() {

}
