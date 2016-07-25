#include <iostream>
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <typeinfo>
#include <string>
#include <boost/asio.hpp>

#include "World.h"
#include "ChronoMessages.pb.h"

void printNum(int num);

void raiseInt(int& i, int n) {
    i += n;
}

// Start of the listener thread
void listenForConnection(World& world, std::queue<std::function<void()>>& queue, std::mutex* queueMutex);

// Start of the client connection handling thread
void processConnection(World& world, std::queue<std::function<void()>>& queue, std::mutex* queueMutex);

int main(int argc, char **argv)
{
	std::function<void(int)> func = std::bind(printNum, std::placeholders::_1);
    
    int i = 0;
    std::thread th(func, i);
    std::thread th1(func, i + 1);
    std::thread th2(func, i + 2);
    th.join();
    th1.join();
    th2.join();
    
    std::queue<std::function<void()>> worldQueue;
    worldQueue.push(std::bind(raiseInt, std::ref(i), 2));
    worldQueue.push(std::bind(printNum, std::ref(i)));
    std::mutex* queueMutex = new std::mutex();
    
    World world(1, 1);
    
    std::function<void(World&, std::queue<std::function<void()>>&, std::mutex*)> listenerFunc = listenForConnection;
    std::thread listener(listenerFunc, std::ref(world), std::ref(worldQueue), queueMutex);
    
    while(true) {
        if (!worldQueue.empty()){
            worldQueue.front()();
            worldQueue.pop();
        }
    }
	return 0;
}
// TODO: Get started on world object, and making the queue of messages to write to it. After that, start thinking about the listener thread.
// zero mq or just a standard queue. Or look up an atomic queue implementation.
void printNum(int num) {
    std::cout << num << std::endl;
}

void listenForConnection(World& world, std::queue<std::function<void()>>& queue, std::mutex* queueMutex) {
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::acceptor acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 1300));
    while(true) {
        boost::asio::ip::tcp::socket socket(ioService);
        acceptor.accept(socket);
        // Create new thread for the client connection here
        
    }
}

void processConnection(World& world, std::queue<std::function<void()>>& queue, std::mutex* queueMutex) {
    
}