#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>

using namespace std;
using boost::asio::ip::tcp;

string makeDaytimeString() {
    time_t now = time(0);
    return ctime(&now);
}

int main(int argc, char **argv)        
{
    try {
        boost::asio::io_service ioService;
        tcp::acceptor acceptor(ioService, tcp::endpoint(tcp::v4(), 1300));
        
        while (true) {
            tcp::socket socket(ioService);
            
            cout << "I'm about to accept" << endl;
            
            acceptor.accept(socket);
            
            string message = makeDaytimeString();
            
            cout << "Message: " << message << endl;
            
            boost::system::error_code ignoredError;
            boost::asio::write(socket, boost::asio::buffer(message), ignoredError);
            
            char* buffer = (char *) malloc(128 * sizeof(char));
            boost::system::error_code error;
            size_t length = socket.read_some(boost::asio::buffer(buffer, 128), error);
            string output(buffer);
            for (unsigned int i = 0; i < output.length(); i++) {
                if (output[i] == '_') output[i] = ' ';
            }
            cout << "Length: " << length << endl << "Message received: " << output << endl;
        }
    } catch (exception& e) {
        cerr << e.what() << endl;
    }
    
	return 0;
}
