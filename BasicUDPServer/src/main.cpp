#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include "addressbook.pb.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;
using boost::asio::ip::udp;

int main(int argc, char **argv)
{
    cout << "Hello, world.\n";
    boost::asio::io_service ioService;
    udp::socket socket(ioService, udp::endpoint(udp::v4(), 1300));
    
    while (true) {
        boost::array <char, 1> receivingBuffer;
        udp::endpoint remoteEndpoint;
        boost::system::error_code error;
        socket.receive_from(boost::asio::buffer(receivingBuffer), remoteEndpoint, 0, error);
        cout << "Recieved " << receivingBuffer.data() << endl;
        if (error && error != boost::asio::error::message_size)
            throw boost::system::system_error(error);
        cout << "Message to send: ";
        char* line = (char *) malloc(20 * sizeof(char));
        cin.getline(line, 30);
        string message(line);
        boost::system::error_code ignoredError;
        socket.send_to(boost::asio::buffer(message), remoteEndpoint, 0, ignoredError);
        
        tutorial::Person person;
        string name;
        cout << "Name: ";
        cin >> name;
        person.set_name(name);
        int id;
        cout << "id: ";
        cin >> id;
        person.set_id(id);
        string email;
        cout << "email:";
        cin >> email;
        person.set_email(email);
        
        boost::asio::streambuf* b = new boost::asio::streambuf();
        b->data();
        
        boost::asio::streambuf buff;
        ostream outStream(&buff);
        
        ::google::protobuf::io::OstreamOutputStream raw_output_stream(&outStream);
        ::google::protobuf::io::CodedOutputStream coded_output_stream(&raw_output_stream);
        
        person.SerializeToCodedStream(&coded_output_stream);
        
        //boost::asio::streambuf buff;
        //ostream outStream(&buff);
        //person.SerializeToOstream(&outStream);
        
        socket.send_to(buff.data(), remoteEndpoint, 0, ignoredError);
    }
	return 0;
}
