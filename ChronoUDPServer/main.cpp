#include <iostream>
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <typeinfo>
#include <chrono>
#include <time.h>
#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/all.hpp>

#include "World.h"
#include "ChronoMessages.pb.h"
#include "MessageCodes.h"

#define PORT_NUMBER 8082
#define HEARTBEAT_LENGTH 1

// Receives all packets from the network
void listenOnSocket(std::shared_ptr<std::queue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>>> incomingBufferQueue,
            std::shared_ptr<std::mutex> socketMutex, std::shared_ptr<boost::asio::ip::udp::socket> socket);

// Sends all packets to their appropriate endoints
void sendOnSocket(std::shared_ptr<std::queue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>>> outgoingBufferQueue,
            std::shared_ptr<std::mutex> socketMutex, std::shared_ptr<boost::asio::ip::udp::socket> socket);

// Performs all parsing, serialization, and processing of message packets before handing instructions off to the world thread to modify the world object
void processMessages(std::shared_ptr<std::queue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>>> incomingBufferQueue, std::shared_ptr<std::mutex> incomingMutex,
            std::shared_ptr<std::queue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>>> outgoingBufferQueue, std::shared_ptr<std::mutex> outgoingMutex
            std::shared_ptr<std::queue<std::function<void()>>>, std::shared_ptr<std::mutex> worldMutex);

int main(int argc, char **argv) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // Queue of lambda expressions modifying the world object
    auto worldQueue = std::make_shared<std::queue<std::function<void()>>>();

    // Mutex controlling pushes to the end of the world queue
    auto worldQueueMutex = std::make_shared<std::mutex>();

    // Queue of buffers received by external hosts
    auto incomingBufferQueue = std::make_shared<std::queue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>>>();

    // Queue of buffers waiting to be sent to clients
    auto outgoingBufferQueue = std::make_shared<std::queue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>>>();

    // Controls send/receive cycle of server
    auto socketMutex = std::make_shared<std::mutex>();

    // Set up socket for network communication
    boost::asio::io_service ioService;
    auto socket = std::make_shared<boost::asio::ip::udp::socket>(ioService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), PORT_NUMBER));
    socket->non_blocking(true);

    unsigned int threadCount = std::thread::hardware_concurrency();
    if (threadCount == 0) {
        std::cout << "Unable to detect the number of threads supported." << std::endl;
        threadCount = 4;
    }

    std::cout << "Thread count: " << threadCount << std::endl;

    // Starts listener thread
    std::thread socketListener(listenOnSocket, incomingBufferQueue, socketMutex, socket);
    std::thread socketSender(sendOnSocket, outgoingBufferQueue, socketMutex, socket);

    threadCount -= 2;

    // Starts worker threads
    // TODO: add mutexes and stuff for worker threads
    std::vector<std::thread> workers;
    for (int i = 0; i < threadCount - 1; i++)
        workers.emplace_back(processMessages);

    socketListener.join();
    socketSender.join();
}

void listenOnSocket(std::shared_ptr<std::queue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>>> incomingBufferQueue,
            std::shared_ptr<std::mutex> socketMutex, std::shared_ptr<boost::asio::ip::udp::socket> socket) {
    while(socket->is_open()) {
        try {
            // Buffer to contain the actual message
            auto receivingBuffer = std::make_shared<boost::asio::streambuf>();
            receivingBuffer->prepare(VEHICLE_MESSAGE_SIZE);

            // Endpoint identifying the sender
            boost::asio::ip::udp::endpoint receivingEndpoint;

            boost::system::error_code errorCode;

            // Locks to guarantee thread safety with sending thread
            socketMutex->lock();

            // The receive call. Throws an error instead of blocking
            socket->receive_from(boost::asio::buffer(receivingBuffer.get(), VEHICLE_MESSAGE_SIZE), receivingEndpoint, 0, errorCode);

            // Unlocks as soon as possible
            socketMutex->unlock();

            // If there was a message waiting in the udp queue
            if (errorCode != boost::asio::error::would_block)
                // Pushes received message and endpoint to queue to be handled by another thread
                incomingBufferQueue->push(std::make_pair(receivingEndpoint, receivingBuffer));

        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }
}

void sendOnSocket(std::shared_ptr<std::queue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>>> outgoingBufferQueue,
            std::shared_ptr<std::mutex> socketMutex, std::shared_ptr<boost::asio::ip::udp::socket> socket) {
    while(socket->is_open()) {
        try {
            if (!outgoingBufferQueue->empty()) {
                // Removes first message to be sent
                auto sendingPair = outgoingBufferQueue->front();
                outgoingBufferQueue->pop();

                // Locks to guarantee thread safety with receiving thread
                socketMutex->lock();

                // The sending call. Sends to the endpoint corresponding to the message popped off the queue.
                socket->send_to(boost::asio::buffer(sendingPair.second.get(), VEHICLE_MESSAGE_SIZE), sendingPair.first);

                // Unlocks as soon as possible
                socketMutex->unlock();
            }
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }
}

void processMessages(std::shared_ptr<std::queue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>>> incomingBufferQueue, std::shared_ptr<std::mutex> incomingMutex,
            std::shared_ptr<std::queue<std::pair<boost::asio::ip::udp::endpoint, std::shared_ptr<boost::asio::streambuf>>>> outgoingBufferQueue, std::shared_ptr<std::mutex> outgoingMutex
            std::shared_ptr<std::queue<std::function<void()>>>, std::shared_ptr<std::mutex> worldMutex) {
    std::cout << "Yo biatch" << std::endl;
}
