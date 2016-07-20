#include <iostream>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "ChronoMessages.pb.h"
#include <vector>

using namespace std;
using boost::asio::ip::udp;

class udp_server
{
public:
  udp_server(boost::asio::io_service& io_service)
    : socket_(io_service, udp::endpoint(udp::v4(), 1300))
  {
    start_receive();
    //receiveVehicle();
  }

private:
  void start_receive()
  {
      while(true) {
            udp::endpoint* remote_endpoint_ = new udp::endpoint(); // endpoint object is cleaned up by handler
            socket_.receive_from( // Go ahead and make everything synchronous for testing purposes--------------------
                boost::asio::buffer(recv_buffer_), *remote_endpoint_); // Should stay as a synchronous call to maintain a listening thread.
            cout << "receive_from() called" << endl;
            handle_receive(remote_endpoint_);
      }
  }

  void handle_receive(udp::endpoint* remote_endpoint_)
  {
        
        if (recv_buffer_[0] == 'p') {
            cout << "Called and found a p" << endl;
            boost::shared_ptr<std::string> message(
                new std::string("Hello, world."));

            // To make synchronous, delete async_ in function and remove the call to boost::bind
            socket_.async_send_to(boost::asio::buffer(*message), *remote_endpoint_,
                        boost::bind(&udp_server::receiveVehicle, this, remote_endpoint_)); 
              
            cout << "Connected to client." << endl;

            // receiveVehicle(remote_endpoint_); // When everything is synchronous, uncomment this line.
            
        }
        
        //delete remote_endpoint_;
  }
  
    
    boost::asio::streambuf* receiveVehicle(udp::endpoint* remote_endpoint_) {
        while (true) {
            cout << "Receiving vehicle message..." << endl;
            //std::vector<uint8_t> buff(512, 0);
            boost::asio::streambuf *buff = new boost::asio::streambuf(512);
            
            cout << buff->size() << endl;
            
            istream* inStream = new istream(buff); // The vehicle can be parsed directly from this istream if necessary, no CodedInputStream needed.
            //google::protobuf::io::IstreamInputStream* inputStream = new google::protobuf::io::IstreamInputStream(is);
            
            //google::protobuf::io::CodedInputStream* inStream = new google::protobuf::io::CodedInputStream(inputStream);
            
            socket_.receive_from(buff->prepare(512), *remote_endpoint_);
            
            buff->commit(512);
            cout << buff->size() << endl;
            cout << "Vehicle Message received:" << endl;
            ChronoMessages::VehicleMessage vehicle;
            //uint32_t magic = 0;
            //uint32_t sz = 0;
            //inStream->ReadLittleEndian32(&magic);
            //if (magic != 0x600DBEEF) {
            //    cerr << "Parsing failed, stream is not sane! [BAD_MAGIC: " << std::hex << magic << "]" << endl;
            //}
            //else {
            //    cout << "Stream is sane." << endl;
            //}
            //inStream->ReadVarint32(&sz);
            //cout << "Parsing " << sz << " bytes..." << endl;
            //vehicle.ParseFromCodedStream(inStream);
            
            vehicle.ParseFromIstream(inStream);
            cout << "Parsed." << endl;
            cout << vehicle.DebugString() << endl;
            cout << "Debug string should have printed" << endl;
            delete buff;
            delete inStream;
        }
    }

  void handle_send(boost::shared_ptr<std::string> /*message*/)
  {
  }
  vector<udp::endpoint> endpoints;
  udp::socket socket_;
  //udp::endpoint remote_endpoint_;
  boost::array<char, 1> recv_buffer_;
};

int main(int argc, char **argv)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    cout << "Starting server...\n";
    try {
        boost::asio::io_service io_service;
        udp_server server(io_service);
        io_service.run();
    } catch (std::exception& e) {
        cerr << e.what() << std::endl;
    }
	return 0;
}
