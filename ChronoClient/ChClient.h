#ifndef CHCLIENT_H
#define CHCLIENT_H

#include <iostream>
#include <thread>
#include <google/protobuf/message.h>
#include <boost/asio.hpp>

#include "MessageCodes.h"
#include "ChronoMessages.pb.h"

class ChClient
{
public:
    ChClient(boost::asio::io_service* ioService);
    ~ChClient();
    
    // Getter for the connection number
    int connectionNumber();
    
    // Returns -1 if connection is unsuccessful
    int connectToServer(std::string name, std::string port);
    
    // Returns immediately and uses the socket to receive from server on another thread
    void asyncListen(std::map<int, std::shared_ptr<google::protobuf::Message>>& serverMessages);
    
    // Blocks until message has been serialized and sent over socket
    void sendMessage(std::shared_ptr<google::protobuf::Message> message);

private:
    boost::asio::io_service* m_ioService;
    boost::asio::ip::tcp::socket m_socket;
    int m_connectionNumber;
    std::shared_ptr<std::thread> listener;
    boost::asio::streambuf m_buff;
    std::ostream m_outStream;
};

#endif // CHCLIENT_H
