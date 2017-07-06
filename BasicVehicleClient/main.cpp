#include <iostream>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "ChronoMessages.pb.h"

using namespace std;
using boost::asio::ip::udp;

void setVector(ChronoMessages::VehicleMessage_MVector *vehVector, ChronoMessages::VehicleMessage_MVector *otherVector) {
    vehVector->set_x(otherVector->x());
    vehVector->set_y(otherVector->y());
    vehVector->set_z(otherVector->z());
}

void setQuaternion(ChronoMessages::VehicleMessage_MQuaternion *vehQuaternion, ChronoMessages::VehicleMessage_MQuaternion *otherQuaternion) {
    vehQuaternion->set_e0(otherQuaternion->e0());
    vehQuaternion->set_e1(otherQuaternion->e1());
    vehQuaternion->set_e2(otherQuaternion->e2());
    vehQuaternion->set_e3(otherQuaternion->e3());
}

int main(int argc, char **argv)
{
	cout << "Connecting to server..." << endl;
    
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
        
        boost::shared_ptr<std::string> message(
                  new std::string("Hello, world."));
        //socket.send_to(boost::asio::buffer(*message), receiverEndpoint);
        
        //boost::array <char, 128> receivingBuffer;
        //udp::endpoint senderEndpoint;
        
        //socket.receive_from(boost::asio::buffer(receivingBuffer), senderEndpoint);
        
        cout << "Connected" << endl;
        cout << "Creating Vehicle Message..." << endl;
        
        ChronoMessages::VehicleMessage vehicle;
        vehicle.set_timestamp(time(nullptr));
        vehicle.set_chtime(10.235);
        vehicle.set_speed(41236136436436673);
        vehicle.set_idnumber(24601);
        
        ChronoMessages::VehicleMessage_MVector vector;
        vector.set_x(3.14);
        vector.set_y(123.0);
        vector.set_z(42.0);
        setVector(vehicle.mutable_chassiscom(), &vector);
        setVector(vehicle.mutable_backleftwheelcom(), &vector);
        setVector(vehicle.mutable_backrightwheelcom(), &vector);
        setVector(vehicle.mutable_frontleftwheelcom(), &vector);
        setVector(vehicle.mutable_frontrightwheelcom(), &vector);
        
        ChronoMessages::VehicleMessage_MQuaternion quaternion;
        quaternion.set_e0(3.141592653598);
        quaternion.set_e2(143);
        quaternion.set_e1(43232);
        quaternion.set_e3(1.00);
        setQuaternion(vehicle.mutable_chassisrot(), &quaternion);
        setQuaternion(vehicle.mutable_backleftwheelrot(), &quaternion);
        setQuaternion(vehicle.mutable_backrightwheelrot(), &quaternion);
        setQuaternion(vehicle.mutable_frontleftwheelrot(), &quaternion);
        setQuaternion(vehicle.mutable_frontrightwheelrot(), &quaternion);
        
        cout << "Vehicle Message created." << endl;
        cout << "Vehicle: " << endl;
        cout << vehicle.DebugString();
        
        cout << "Sending message..." << endl;
        
        boost::asio::streambuf buff;
        ostream outStream(&buff);
        
        //::google::protobuf::io::OstreamOutputStream raw_output_stream(&outStream);
        //::google::protobuf::io::CodedOutputStream coded_output_stream(&raw_output_stream);
        //coded_output_stream.WriteLittleEndian32(0x600DBEEF);
        //coded_output_stream.WriteVarint32(vehicle.ByteSize());
        
        vehicle.SerializeToOstream(&outStream);
        //vehicle.SerializeToCodedStream(&coded_output_stream);
        
        //google::protobuf::io::CodedOutputStream Stream(google::protobuf::io::ZeroCopyOutputStream());
        //vehicle.SerializeToOstream(&outStream);
                
        socket.send_to(buff.data(), receiverEndpoint);
        
        cout << "Message sent." << endl;
    } catch (exception& e) {
        cerr << e.what() << endl;
    }
    
	return 0;
}
