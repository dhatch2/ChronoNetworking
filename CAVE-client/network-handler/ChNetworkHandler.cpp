#include "ChNetworkHandler.h"
#include "MessageCodes.h"

ChNetworkHandler::ChNetworkHandler() : socket(*(new boost::asio::io_service)) {

}

ChNetworkHandler::~ChNetworkHandler() {

}

void ChNetworkHandler::sendMessage(boost::asio::ip::udp::endpoint& endpoint, boost::asio::streambuf& message) {

}

std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>& ChNetworkHandler::receiveMessage() {

}

ChClientHandler::ChClientHandler(std::string hostname, std::string port) : ChNetworkHandler() {
    boost::asio::ip::tcp::resolver tcpResolver(socket.get_io_service());
    boost::asio::ip::tcp::resolver::query tcpQuery(hostname, port);
    boost::asio::ip::tcp::resolver::iterator endpointIterator = tcpResolver.resolve(tcpQuery);
    boost::asio::ip::tcp::socket tcpSocket(socket.get_io_service());
    boost::asio::connect(tcpSocket, endpointIterator);

    uint8_t connectionRequest = CONNECTION_REQUEST;
    tcpSocket.send(boost::asio::buffer(&connectionRequest, sizeof(uint8_t)));

    uint8_t requestResponse;
    tcpSocket.receive(boost::asio::buffer(&requestResponse, sizeof(uint8_t)));

    if(requestResponse == CONNECTION_DECLINE) throw ConnectionRefusedException();
    if(requestResponse == CONNECTION_ACCEPT) {
        uint32_t connectionNumber;
        tcpSocket.receive(boost::asio::buffer(&connectionNumber, sizeof(uint32_t)));
        tcpSocket.close();
        m_connectionNumber = connectionNumber;

        boost::asio::ip::udp::resolver udpResolver(socket.get_io_service());
        boost::asio::ip::udp::resolver::query udpQuery(boost::asio::ip::udp::v4(), hostname, port);
        serverEndpoint = *udpResolver.resolve(udpQuery);
        socket.open(boost::asio::ip::udp::v4());
    }
}

ChClientHandler::~ChClientHandler() {

}

void ChClientHandler::beginListen() {

}

void ChClientHandler::beginSend() {

}

ChServerHandler::ChServerHandler() : ChNetworkHandler() {

}

ChServerHandler::~ChServerHandler() {

}

void ChServerHandler::beginSend() {

}

void ChServerHandler::beginListen() {

}
