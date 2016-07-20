#include <iostream>
#include <stdint.h>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "addressbook.pb.h"

using namespace std;
using boost::asio::ip::udp;

int main(int argc, char **argv)
{
    cout << "Hello, world.\n";
    
    try {
        if (argc != 2) {
            std::cerr << "Usage: client <host>" << std::endl;
            return 1;
        }
        
        boost::asio::io_service ioService;
        udp::resolver resolver(ioService);
        udp::resolver::query query(udp::v4(), argv[1], "1300");
        udp::endpoint receiverEndpoint = *resolver.resolve(query);
        udp::socket socket(ioService);
        socket.open(udp::v4());
        
        boost::array <char, 1> sendBuffer = {{ 0 }};
        socket.send_to(boost::asio::buffer(sendBuffer), receiverEndpoint);
        cout << "Sent " << sendBuffer.data() << endl;
        boost::array <char, 128> receivingBuffer;
        udp::endpoint senderEndpoint;
        
        size_t length = socket.receive_from(boost::asio::buffer(receivingBuffer), senderEndpoint);
        
        cout << receivingBuffer.data() << endl;
        
        boost::asio::streambuf buff;
        
        istream is(&buff);
        google::protobuf::io::IstreamInputStream inputStream(&is);
        
        google::protobuf::io::CodedInputStream inStream(&inputStream);
        
        socket.receive_from(buff.data(), senderEndpoint);
        
        //uint8_t* buff = (uint8_t *) malloc(128 * sizeof(uint8_t));
        //google::protobuf::io::CodedInputStream inStream(buff, 128);
        
        //length = socket.receive_from(boost::asio::buffer(buff, 128), senderEndpoint);
        
        tutorial::Person person;
        
        person.ParseFromCodedStream(&inStream);
        
        cout << "Received person: " << person.DebugString() << endl;
    } catch (exception& e) {
        cerr << e.what() << std::endl;
    }
    
	return 0;
}
