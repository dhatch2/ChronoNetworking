#include <stdio.h>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using namespace std;
using boost::asio::ip::tcp;

int main(int argc, char *argv[])
{
    cout << "Hello, world.\n";
    try {
        if (argc != 2) {
            cerr << "Usage: client <host>" << endl;
            return 1;
        }
        
        boost::asio::io_service ioService;
        tcp::resolver resolver(ioService);
        tcp::resolver::query query(argv[1], "1300");
        tcp::resolver::iterator endpointIterator = resolver.resolve(query);
                
        tcp::socket socket(ioService);
        //socket.connect(endpoint);
        boost::asio::connect(socket, endpointIterator);
        
        boost::system::error_code error;
        
        do {
            boost::array <char, 128> buffer;
            
            size_t length = socket.read_some(boost::asio::buffer(buffer), error);
            
            cout << "Message: " << buffer.data() << endl;
            
            char* line = (char *) malloc(20 * sizeof(char));
            cout << "Send: ";
            cin.getline(line, 20);
            string message(line);
            
            for (unsigned int i = 0; i < message.length(); i++) {
                if (message[i] == ' ') message[i] = '_';
            }
            
            cout << message << endl;
            
            boost::system::error_code ignoredError;
            boost::asio::write(socket, boost::asio::buffer(message, 128), ignoredError);
            
            if (error != boost::asio::error::eof && error != nullptr)
                throw boost::system::system_error(error);
        } while (error != boost::asio::error::eof);
    } catch (std::exception& e) {
        cerr << e.what() << endl;
    }
	return 0;
}
