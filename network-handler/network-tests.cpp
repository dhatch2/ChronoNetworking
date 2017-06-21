// =============================================================================
// Authors: Dylan Hatch
// =============================================================================
//
//	Unit tests for ChNetworkHandler, ChClientHandler, and ChServerHandler.
//
// =============================================================================

#include <iostream>
#include <thread>
#include "ChNetworkHandler.h"
#include "ChronoMessages.pb.h"
#include "MessageCodes.h"
#include "MessageConversions.h"

#include "chrono/core/ChFileutils.h"
#include "chrono/core/ChStream.h"
#include "chrono/core/ChRealtimeStep.h"
#include "chrono/physics/ChSystem.h"
#include "chrono/utils/ChUtilsInputOutput.h"

#include "chrono_vehicle/ChConfigVehicle.h"
#include "chrono_vehicle/ChVehicleModelData.h"
#include "chrono_vehicle/terrain/RigidTerrain.h"
//#include "chrono_vehicle/driver/ChIrrGuiDriver.h"
#include "chrono_vehicle/driver/ChDataDriver.h"
//#include "chrono_vehicle/wheeled_vehicle/utils/ChWheeledVehicleIrrApp.h"

#include "chrono_models/vehicle/hmmwv/HMMWV.h"

using namespace chrono;
using namespace chrono::vehicle;
using namespace chrono::vehicle::hmmwv;

enum DriverMode { DEFAULT, RECORD, PLAYBACK };
DriverMode driver_mode = DEFAULT;

// Type of powertrain model (SHAFTS, SIMPLE)
PowertrainModelType powertrain_model = PowertrainModelType::SHAFTS;

// Drive type (FWD, RWD, or AWD)
DrivelineType drive_type = DrivelineType::AWD;

// Type of tire model (RIGID, RIGID_MESH, PACEJKA, LUGRE, FIALA)
//TireModelType tire_model = TireModelType::RIGID;

// Rigid terrain
RigidTerrain::Type terrain_model = RigidTerrain::FLAT;
bool terrain_vis = true;
double terrainHeight = 0;      // terrain height (FLAT terrain only)
double terrainLength = 1000.0;  // size in X direction
double terrainWidth = 1000.0;   // size in Y direction

// Point on chassis tracked by the camera
ChVector<> trackPoint(0.0, 0.0, 1.75);

// Contact method
ChMaterialSurface::ContactMethod contact_method = ChMaterialSurface::SMC;
bool contact_vis = false;

// Simulation step sizes
double step_size = 1e-3;
double tire_step_size = step_size;

// Simulation end time
double t_end = 1000;

// Time interval between two render frames
double render_step_size = 1.0 / 50;  // FPS = 50

// Debug logging
bool debug_output = false;
double debug_step_size = 1.0 / 1;  // FPS = 1

// POV-Ray output
bool povray_output = false;

int main(int argc, char **argv) {
    // Client connection tests ///////////////////////////////////////////////////////////////////
    try {
        ChClientHandler clientHandler("dummy_hostname", "24601");
        std::cout << "FAILED -- Client connection test 1" << std::endl;
    } catch (ConnectionException& exp) {
        if (exp.type() == FAILED_CONNECTION) {
            std::cout << "PASSED -- Client connection test 1" << std::endl;
        } else std::cout << "FAILED -- Client connection test 1: " << exp.what() << std::endl;
    }
    boost::asio::io_service ioService;

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

    boost::asio::ip::tcp::acceptor acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8082));
    boost::asio::ip::tcp::socket tcpSocket1(ioService);
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
    acceptor.close();

    // Server connection tests ///////////////////////////////////////////////////////////////////
    ChServerHandler *serverHandler = new ChServerHandler(8082);

    boost::asio::ip::tcp::socket tcpSocket3(ioService);
    boost::asio::ip::tcp::resolver tcpResolver(ioService);
    boost::asio::ip::tcp::resolver::query tcpQuery("localhost", "8082");
    uint8_t requestResponse;
    boost::asio::ip::tcp::resolver::iterator endpointIterator = tcpResolver.resolve(tcpQuery);

    boost::asio::connect(tcpSocket3, endpointIterator);
    uint8_t connectionRequest = CONNECTION_REQUEST;
    tcpSocket3.send(boost::asio::buffer(&connectionRequest, sizeof(uint8_t)));
    tcpSocket3.receive(boost::asio::buffer(&requestResponse, sizeof(uint8_t)));
    if(requestResponse == CONNECTION_ACCEPT) {
        uint32_t connectionNumber;
        tcpSocket3.receive(boost::asio::buffer(&connectionNumber, sizeof(uint32_t)));
        if (connectionNumber == 0) {
            std::cout << "PASSED -- Server connection test 1" << std::endl;
        } else std::cout << "FAILED -- Server connection test 1" << std::endl;
    } else std::cout << "FAILED -- Server connection test 1" << std::endl;

    boost::asio::connect(tcpSocket3, endpointIterator);
    tcpSocket3.send(boost::asio::buffer(&connectionRequest, sizeof(uint8_t)));
    tcpSocket3.receive(boost::asio::buffer(&requestResponse, sizeof(uint8_t)));
    if(requestResponse == CONNECTION_ACCEPT) {
        uint32_t connectionNumber;
        tcpSocket3.receive(boost::asio::buffer(&connectionNumber, sizeof(uint32_t)));
        if (connectionNumber == 1) {
            std::cout << "PASSED -- Server connection test 2" << std::endl;
        } else std::cout << "FAILED -- Server connection test 2" << std::endl;
    } else std::cout << "FAILED -- Server connection test 2" << std::endl;

    boost::asio::connect(tcpSocket3, endpointIterator);
    connectionRequest = 100;
    tcpSocket3.send(boost::asio::buffer(&connectionRequest, sizeof(uint8_t)));
    tcpSocket3.receive(boost::asio::buffer(&requestResponse, sizeof(uint8_t)));
    if(requestResponse == CONNECTION_DECLINE) {
        std::cout << "PASSED -- Server connection test 3" << std::endl;
    } else std::cout << "FAILED -- Server connection test 3" << std::endl;

    tcpSocket3.close();

    // Server-client integration test
    ChClientHandler *clientHandler = new ChClientHandler("localhost", "8082");

    if (clientHandler->connectionNumber() == 2) {
        std::cout << "PASSED -- Server-client integration test 1" << std::endl;
    } else std::cout << "FAILED -- Server-client integration test 1" << std::endl;

    delete clientHandler;
    delete serverHandler;

    ChServerHandler *serverHandler2 = new ChServerHandler(8082);
    ChClientHandler *clientHandler2 = new ChClientHandler("localhost", "8082");

    if (clientHandler2->connectionNumber() == 0) {
        std::cout << "PASSED -- Server-client integration test 2" << std::endl;
    } else std::cout << "FAILED -- Server-client integration test 2 (" << clientHandler->connectionNumber() << ")" << std::endl;

    delete clientHandler2;
    delete serverHandler2;

    // Communication tests //////////////////////////////////////////////////////////////////////

    std::string Dmessage1 = "Yeeeeaaaahhhh boiiiiiiiiiiiiiii";
    std::thread client3([&] {
        try {
            ChClientHandler clientHandler("localhost", "8082");
            clientHandler.beginSend();

            HMMWV_Full my_hmmwv1; // Test vehicle for messages

            ChVector<> initLoc1(0, 0, 1.6);
            ChQuaternion<> initRot1(1, 0, 0, 0);

            my_hmmwv1.SetContactMethod(contact_method);
            my_hmmwv1.SetChassisFixed(false);
            my_hmmwv1.SetInitPosition(ChCoordsys<>(initLoc1, initRot1));
            my_hmmwv1.SetPowertrainType(powertrain_model);
            my_hmmwv1.SetDriveType(drive_type);
            //my_hmmwv.SetTireType(tire_model);
            my_hmmwv1.SetTireStepSize(tire_step_size);
            my_hmmwv1.SetPacejkaParamfile("hmmwv/tire/HMMWV_pacejka.tir");
            my_hmmwv1.Initialize();

            ChronoMessages::MessagePacket packet;
            packet.add_dsrcmessages();

            ChronoMessages::DSRCMessage *message = packet.mutable_dsrcmessages(0);
            message->set_vehicleid(1);
            message->set_timestamp(time(0));
            message->set_chtime(my_hmmwv1.GetVehicle().GetChTime());
            messageFromVector(message->mutable_vehiclepos(), my_hmmwv1.GetVehicle().GetVehiclePos());
            message->set_buffer(Dmessage1);

            clientHandler.pushMessage(packet);

            clientHandler.beginListen();
            auto newMessage = clientHandler.popDSRCMessage();
            if (newMessage->buffer().compare(Dmessage1) == 0) {
                std::cout << "PASSED -- Client communication test 2" << std::endl;
            } else std::cout << "FAILED -- Client communication test 2" << std::endl;

            clientHandler.popSimMessage();
            auto vMessage = std::static_pointer_cast<ChronoMessages::VehicleMessage>(clientHandler.popSimMessage());
            if (vMessage->DebugString().compare(generateVehicleMessageFromWheeledVehicle(&my_hmmwv1.GetVehicle(), 0).DebugString()) == 0) {
                std::cout << "PASSED -- Client communication test 3" << std::endl;
            } else std::cout << "FAILED -- Client communication test 3" << std::endl;
        } catch (ConnectionException& exp) {
            std::cout << "FAILED -- Client communication test 2" << std::endl;
            std::cout << "FAILED -- Client communication test 3" << std::endl;
        }
    });

    boost::asio::ip::tcp::acceptor acceptor2(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8082));
    boost::asio::ip::tcp::socket tcpSocket4(ioService);
    acceptor2.accept(tcpSocket4);


    tcpSocket4.receive(boost::asio::buffer(&requestMessage, sizeof(uint8_t)));
    tcpSocket4.send(boost::asio::buffer(&acceptMessage, sizeof(uint8_t)));
    tcpSocket4.send(boost::asio::buffer(&connectionNumber, sizeof(uint32_t)));

    boost::asio::ip::tcp::endpoint tcpEndpoint = tcpSocket4.remote_endpoint();
    boost::asio::ip::udp::endpoint udpEndpoint(tcpEndpoint.address(), tcpEndpoint.port());

    tcpSocket4.close();
    acceptor2.close();

    boost::asio::ip::udp::socket udpSocket(ioService);
    udpSocket.open(boost::asio::ip::udp::v4());
    udpSocket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 8082));

    boost::asio::streambuf buff;
    udpSocket.receive(boost::asio::null_buffers());
    int available = udpSocket.available();
    boost::asio::ip::udp::endpoint recEndpoint;
    udpSocket.receive_from(buff.prepare(available), recEndpoint);
    std::istream inStream(&buff);

    uint8_t inMessageType;
    buff.commit(sizeof(uint8_t));
    inStream >> inMessageType;
    uint32_t length;
    buff.commit(sizeof(uint32_t));
    inStream >> length;
    buff.commit(length);
    ChronoMessages::MessagePacket pack;
    pack.ParseFromIstream(&inStream);

    if (pack.dsrcmessages(0).buffer().compare(Dmessage1) == 0) {
        std::cout << "PASSED -- Client communication test 1" << std::endl;
    } else std::cout << "FAILED -- Client communication test 1" << std::endl;

    HMMWV_Full my_hmmwv1; // Test vehicle for messages

    ChVector<> initLoc1(0, 0, 1.6);
    ChQuaternion<> initRot1(1, 0, 0, 0);

    my_hmmwv1.SetContactMethod(contact_method);
    my_hmmwv1.SetChassisFixed(false);
    my_hmmwv1.SetInitPosition(ChCoordsys<>(initLoc1, initRot1));
    my_hmmwv1.SetPowertrainType(powertrain_model);
    my_hmmwv1.SetDriveType(drive_type);
    //my_hmmwv.SetTireType(tire_model);
    my_hmmwv1.SetTireStepSize(tire_step_size);
    my_hmmwv1.SetPacejkaParamfile("hmmwv/tire/HMMWV_pacejka.tir");
    my_hmmwv1.Initialize();

    ChronoMessages::MessagePacket packet;
    packet.add_dsrcmessages();

    ChronoMessages::DSRCMessage *message = packet.mutable_dsrcmessages(0);
    message->set_vehicleid(1);
    message->set_timestamp(time(0));
    message->set_chtime(my_hmmwv1.GetVehicle().GetChTime());
    messageFromVector(message->mutable_vehiclepos(), my_hmmwv1.GetVehicle().GetVehiclePos());
    message->set_buffer(Dmessage1);

    boost::asio::streambuf buffer;
    std::ostream stream(&buffer);

    uint8_t messageType = MESSAGE_PACKET;
    stream << messageType;
    uint32_t size = packet.ByteSize();
    stream << size;
    packet.SerializeToOstream(&stream);
    stream.flush();

    udpSocket.send_to(buffer.data(), recEndpoint);

    ChronoMessages::VehicleMessage sendVehicle = generateVehicleMessageFromWheeledVehicle(&my_hmmwv1.GetVehicle(), 0);

    messageType = VEHICLE_MESSAGE;
    stream << messageType;
    size = sendVehicle.ByteSize();
    stream << size;
    packet.SerializeToOstream(&stream);
    stream.flush();

    udpSocket.send_to(buffer.data(), recEndpoint);

    client3.join();

    return 0;
}
