#include <iostream>
#include <thread>
#include "ChNetworkHandler.h"

int main(int argc, char **argv) {
    // Client connection tests
    try {
        ChClientHandler clientHandler("dummy_hostname", "24601");
        std::cout << "FAILED -- Client connection test 1" << std::endl;
    } catch (ConnectionException& exp) {
        if (exp.type() == FAILED_CONNECTION) {
            std::cout << "PASSED -- Client connection test 1" << std::endl;
        } else std::cout << "FAILED -- Client connection test 1: " << exp.what() << std::endl;
    }
    boost::asio::io_service ioService;

    boost::asio::ip::tcp::acceptor acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8082));

    boost::asio::ip::tcp::socket tcpSocket1(ioService);

    std::thread client1([&] {
        try {
            ChClientHandler clientHandler("localhost", "8082");
            std::cout << "FAILED -- Client connection test 2" << std::endl;
        } catch (ConnectionException& exp) {
            if (exp.type() == REFUSED_CONNECTION) {
                std::cout << "PASSED -- Client connection test 2" << std::endl;
            } else std::cout << "FAILED -- Client connection test 2" << std::endl;
        }
    });

    acceptor.accept(tcpSocket1);

    uint8_t requestMessage;
    tcpSocket1.receive(boost::asio::buffer(&requestMessage, sizeof(uint8_t)));
    uint8_t declineMessage = CONNECTION_DECLINE;
    tcpSocket1.send(boost::asio::buffer(&declineMessage, sizeof(uint8_t)));

    client1.join();

    tcpSocket1.close();

    boost::asio::ip::tcp::socket tcpSocket2(ioService);

    std::thread client2([&] {
        try {
            ChClientHandler clientHandler("localhost", "8082");
            if (clientHandler.connectionNumber() == 0) {
                std::cout << "PASSED -- Client connection test 3" << std::endl;
            } else std::cout << "FAILED -- Client connection test 3" << std::endl;
        } catch (ConnectionException& exp) {
            std::cout << "FAILED -- Client connection test 3" << exp.what() << std::endl;
        }
    });

    acceptor.accept(tcpSocket2);
    tcpSocket2.receive(boost::asio::buffer(&requestMessage, sizeof(uint8_t)));
    uint8_t acceptMessage = CONNECTION_ACCEPT;
    tcpSocket2.send(boost::asio::buffer(&acceptMessage, sizeof(uint8_t)));
    uint32_t connectionNumber = 0;
    tcpSocket2.send(boost::asio::buffer(&connectionNumber, sizeof(uint32_t)));
    client2.join();
    tcpSocket2.close();

    // Server connection tests
    std::thread server1([&] {
        ChServerHandler serverHandler(8082);
    });

    boost::asio::ip::tcp::socket tcpSocket3(ioService);
    boost::asio::ip::tcp::resolver tcpResolver(ioService);
    boost::asio::ip::tcp::resolver::query tcpQuery("localhost", "8082");
    uint8_t requestResponse;
    boost::asio::ip::tcp::resolver::iterator endpointIterator = tcpResolver.resolve(tcpQuery);
    boost::asio::connect(tcpSocket3, endpointIterator);
    std::cout << "connected" << std::endl;
    uint8_t connectionRequest = CONNECTION_REQUEST;
    tcpSocket3.send(boost::asio::buffer(&connectionRequest, sizeof(uint8_t)));
    std::cout << "sent" << std::endl;
    tcpSocket3.receive(boost::asio::buffer(&requestResponse, sizeof(uint8_t)));
    if(requestResponse == CONNECTION_ACCEPT) {
        uint32_t connectionNumber;
        tcpSocket3.receive(boost::asio::buffer(&connectionNumber, sizeof(uint32_t)));
        if (connectionNumber == 0) {
            std::cout << "PASSED -- Server connection test 1" << std::endl;
        } else std::cout << "FAILED -- Server connection test 1" << std::endl;
    } else std::cout << "FAILED -- Server connection test 1" << std::endl;

    server1.join();
    return 0;
}
