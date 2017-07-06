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
#include "World.h"
#include "ChSafeQueue.h"

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

HMMWV_Full generateTestVehicle() {
    HMMWV_Full my_hmmwv; // Test vehicle for messages

    ChVector<> initLoc1(0, 0, 1.6);
    ChQuaternion<> initRot1(1, 0, 0, 0);

    my_hmmwv.SetContactMethod(contact_method);
    my_hmmwv.SetChassisFixed(false);
    my_hmmwv.SetInitPosition(ChCoordsys<>(initLoc1, initRot1));
    my_hmmwv.SetPowertrainType(powertrain_model);
    my_hmmwv.SetDriveType(drive_type);
    //my_hmmwv.SetTireType(tire_model);
    my_hmmwv.SetTireStepSize(tire_step_size);
    my_hmmwv.SetPacejkaParamfile("hmmwv/tire/HMMWV_pacejka.tir");
    my_hmmwv.Initialize();

    return my_hmmwv;
}

void generateTestDSRCMessage(ChronoMessages::DSRCMessage *message, HMMWV_Full& my_hmmwv, std::string& DMessage) {
    message->set_idnumber(1);
    message->set_timestamp(time(0));
    message->set_chtime(my_hmmwv.GetVehicle().GetChTime());
    messageFromVector(message->mutable_vehiclepos(), my_hmmwv.GetVehicle().GetVehiclePos());
    message->set_buffer(DMessage);
}

void serializeDSRC(std::ostream& stream, ChronoMessages::DSRCMessage& message) {
    uint8_t messageType = DSRC_MESSAGE;
    stream << messageType;
    message.SerializeToOstream(&stream);
    stream.flush();
}

void serializeVehicle(std::ostream& stream, ChronoMessages::VehicleMessage& message) {
    uint8_t messageType = VEHICLE_MESSAGE;
    stream << messageType;
    message.SerializeToOstream(&stream);
    stream.flush();
}

int main(int argc, char **argv) {
    World world;
    ChSafeQueue<std::function<void()>> worldQueue;

    // Client connection tests //////////////////////////////////////////////////////////
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
    ChServerHandler *serverHandler = new ChServerHandler(world, worldQueue, 8082);

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

    ChServerHandler *serverHandler2 = new ChServerHandler(world, worldQueue, 8082);
    ChClientHandler *clientHandler2 = new ChClientHandler("localhost", "8082");

    if (clientHandler2->connectionNumber() == 0) {
        std::cout << "PASSED -- Server-client integration test 2" << std::endl;
    } else std::cout << "FAILED -- Server-client integration test 2 (" << clientHandler->connectionNumber() << ")" << std::endl;

    delete clientHandler2;
    delete serverHandler2;

    // Client Communication tests //////////////////////////////////////////////////////////////////////

    std::string Dmessage1 = "Yeeeeaaaahhhh boiiiiiiiiiiiiiii";
    std::thread client3([&] {
        try {
            ChClientHandler clientHandler("localhost", "8082");
            clientHandler.beginSend();

            HMMWV_Full my_hmmwv = generateTestVehicle(); // Test vehicle for messages
            ChronoMessages::DSRCMessage dsrcMessage;
            generateTestDSRCMessage(&dsrcMessage, my_hmmwv, Dmessage1);
            clientHandler.pushMessage(dsrcMessage);

            clientHandler.beginListen();
            auto newMessage = clientHandler.popDSRCMessage();

            if (newMessage->buffer().compare(Dmessage1) == 0) {
                std::cout << "PASSED -- Client communication test 2" << std::endl;
            } else std::cout << "FAILED -- Client communication test 2" << std::endl;

            auto vMessage = std::static_pointer_cast<ChronoMessages::VehicleMessage>(clientHandler.popSimMessage());
            if (vMessage->DebugString().compare(generateVehicleMessageFromWheeledVehicle(&my_hmmwv.GetVehicle(), 0, 0).DebugString()) == 0) {
                std::cout << "PASSED -- Client communication test 3" << std::endl;
            } else std::cout << "FAILED -- Client communication test 3" << std::endl;

            auto newMessage1 = clientHandler.popDSRCMessage();
            auto newMessage2 = clientHandler.popDSRCMessage();
            auto newMessage3 = clientHandler.popDSRCMessage();
            auto newMessage4 = clientHandler.popDSRCMessage();


            auto vMessage1 = std::static_pointer_cast<ChronoMessages::VehicleMessage>(clientHandler.popSimMessage());
            auto vMessage2 = std::static_pointer_cast<ChronoMessages::VehicleMessage>(clientHandler.popSimMessage());
            auto vMessage3 = std::static_pointer_cast<ChronoMessages::VehicleMessage>(clientHandler.popSimMessage());

            if (newMessage1->buffer().compare(Dmessage1) == 0 && newMessage2->buffer().compare(Dmessage1) == 0 && newMessage3->buffer().compare(Dmessage1) == 0 && newMessage4->buffer().compare(Dmessage1) == 0 && vMessage1->DebugString().compare(generateVehicleMessageFromWheeledVehicle(&my_hmmwv.GetVehicle(), 0, 0).DebugString()) == 0 && vMessage2->DebugString().compare(generateVehicleMessageFromWheeledVehicle(&my_hmmwv.GetVehicle(), 0, 0).DebugString()) == 0 && vMessage3->DebugString().compare(generateVehicleMessageFromWheeledVehicle(&my_hmmwv.GetVehicle(), 0, 0).DebugString()) == 0) {
                std::cout << "PASSED -- Client communication test 4" << std::endl;
            } else std::cout << "FAILED -- Client communication test 4" << std::endl;
        } catch (ConnectionException& exp) {
            std::cout << "FAILED -- Client communication test 2" << std::endl;
            std::cout << "FAILED -- Client communication test 3" << std::endl;
            std::cout << "FAILED -- Client communication test 4" << std::endl;
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
    size_t recSize = udpSocket.receive_from(buff.prepare(available), recEndpoint);
    std::istream inStream(&buff);

    uint8_t inMessageType;
    buff.commit(sizeof(uint8_t));
    inStream >> inMessageType;
    buff.commit(recSize - 1);
    ChronoMessages::DSRCMessage pack;
    pack.ParseFromIstream(&inStream);

    if (pack./*dsrcmessages(0).*/buffer().compare(Dmessage1) == 0) {
        std::cout << "PASSED -- Client communication test 1" << std::endl;
    } else std::cout << "FAILED -- Client communication test 1" << std::endl;

    // Comm test 2 -- DSRC
    HMMWV_Full my_hmmwv = generateTestVehicle(); // Test vehicle for messages

    ChronoMessages::DSRCMessage dsrcMessage;
    generateTestDSRCMessage(&dsrcMessage, my_hmmwv, Dmessage1);
    boost::asio::streambuf buffer;
    std::ostream outStream(&buffer);
    serializeDSRC(outStream, dsrcMessage);

    int sentSize = udpSocket.send_to(buffer.data(), recEndpoint);
    buffer.consume(sentSize);

    // Comm test 3 -- vehicle
    ChronoMessages::VehicleMessage sendVehicle = generateVehicleMessageFromWheeledVehicle(&my_hmmwv.GetVehicle(), 0, 0);
    sendVehicle.CheckInitialized();

    serializeVehicle(outStream, sendVehicle);

    sentSize = udpSocket.send_to(buffer.data(), recEndpoint);
    buffer.consume(sentSize);

    // Comm test 4

    ChronoMessages::VehicleMessage vehicle = generateVehicleMessageFromWheeledVehicle(&my_hmmwv.GetVehicle(), 0, 0);

    ChronoMessages::MessagePacket packet;
    packet.set_connectionnumber(0);
    packet.add_dsrcmessages();
    generateTestDSRCMessage(packet.mutable_dsrcmessages(0), my_hmmwv, Dmessage1);
    packet.add_dsrcmessages();
    generateTestDSRCMessage(packet.mutable_dsrcmessages(1), my_hmmwv, Dmessage1);
    packet.add_dsrcmessages();
    generateTestDSRCMessage(packet.mutable_dsrcmessages(2), my_hmmwv, Dmessage1);
    packet.add_dsrcmessages();
    generateTestDSRCMessage(packet.mutable_dsrcmessages(3), my_hmmwv, Dmessage1);
    packet.add_vehiclemessages();
    packet.mutable_vehiclemessages(0)->CopyFrom(vehicle);
    packet.add_vehiclemessages();
    packet.mutable_vehiclemessages(1)->CopyFrom(vehicle);
    packet.add_vehiclemessages();
    packet.mutable_vehiclemessages(2)->CopyFrom(vehicle);

    uint8_t messageType = MESSAGE_PACKET;
    outStream << messageType;
    packet.SerializeToOstream(&outStream);
    sentSize = udpSocket.send_to(buffer.data(), recEndpoint);
    buffer.consume(sentSize);

    udpSocket.close();

    client3.join();

    // Server Communication Tests /////////////////////////////////////////////////////////
    std::condition_variable var;
    std::mutex initMutex;
    bool isReady = false;

    std::thread server([&] {
        std::unique_lock<std::mutex> lock(initMutex);
        ChServerHandler serverHandler(world, worldQueue, 8082);
        isReady = true;
        var.notify_one();
        lock.unlock();
        serverHandler.beginListen();
        serverHandler.beginSend();

        auto recPair1 = serverHandler.popMessage();
        auto message1 = std::static_pointer_cast<ChronoMessages::VehicleMessage>(recPair1.second);
        if (message1->DebugString().compare(sendVehicle.DebugString()) == 0) {
            std::cout << "PASSED -- Server communication test 1" << std::endl;
        } else std::cout << "FAILED -- Server communication test 1" << std::endl;

        auto recPair2 = serverHandler.popMessage();
        auto message2 = std::static_pointer_cast<ChronoMessages::DSRCMessage>(recPair2.second);
        if (message2->DebugString().compare(dsrcMessage.DebugString()) == 0) {
            std::cout << "PASSED -- Server communication test 2" << std::endl;
        } else std::cout << "FAILED -- Server communication test 2" << std::endl;

        serverHandler.pushMessage(recPair2.first, packet);
        serverHandler.pushMessage(recPair2.first, sendVehicle);

        auto recPair3 = serverHandler.popMessage();
        auto message3 = std::static_pointer_cast<ChronoMessages::MessagePacket>(recPair3.second);
        if (message3->DebugString().compare(packet.DebugString()) == 0) {
            std::cout << "PASSED -- Server communication test 5" << std::endl;
        } else std::cout << "FAILED -- Server communication test 5" << std::endl;

        auto recPair4 = serverHandler.popMessage();
        auto message4 = std::static_pointer_cast<ChronoMessages::VehicleMessage>(recPair4.second);
        if (message4->DebugString().compare(sendVehicle.DebugString()) == 0) {
            std::cout << "PASSED -- Server - client communication test 1" << std::endl;
        } else std::cout << "FAILED -- Server - client communication test 1" << std::endl;

        auto recPair5 = serverHandler.popMessage();
        auto message5 = std::static_pointer_cast<ChronoMessages::MessagePacket>(recPair5.second);
        if (message5->DebugString().compare(packet.DebugString()) == 0) {
            std::cout << "PASSED -- Server - client communication test 2" << std::endl;
        } else std::cout << "FAILED -- Server - client communication test 2" << std::endl;

        serverHandler.pushMessage(recPair5.first, sendVehicle);
        serverHandler.pushMessage(recPair5.first, packet);

        auto recPair6 = serverHandler.popMessage();
        auto message6 = std::static_pointer_cast<ChronoMessages::MessagePacket>(recPair6.second);
        if (message6->DebugString().compare(packet.DebugString()) == 0) {
            std::cout << "PASSED -- Server - client communication test 6" << std::endl;
        } else std::cout << "FAILED -- Server - client communication test 6" << std::endl;
    });

    std::unique_lock<std::mutex> lock(initMutex);
    var.wait(lock, [&] { return isReady; });
    boost::asio::ip::tcp::socket tcpSocket5(ioService);
    boost::asio::connect(tcpSocket5, endpointIterator);

    connectionRequest = CONNECTION_REQUEST;
    tcpSocket5.send(boost::asio::buffer(&connectionRequest, sizeof(uint8_t)));
    tcpSocket5.receive(boost::asio::buffer(&requestResponse, sizeof(uint8_t)));
    tcpSocket5.receive(boost::asio::buffer(&connectionNumber, sizeof(uint32_t)));

    tcpSocket5.close();
    boost::asio::ip::udp::socket udpSocket2(ioService);

    boost::asio::ip::udp::resolver udpResolver(ioService);
    boost::asio::ip::udp::resolver::query udpQuery(boost::asio::ip::udp::v4(), "localhost", "8082");
    boost::asio::ip::udp::endpoint serverEndpoint = *udpResolver.resolve(udpQuery);
    udpSocket2.open(boost::asio::ip::udp::v4());

    // Comm test 1

    serializeVehicle(outStream, sendVehicle);
    sentSize = udpSocket2.send_to(buffer.data(), serverEndpoint);
    buffer.consume(sentSize);

    // Comm test 2

    serializeDSRC(outStream, dsrcMessage);
    sentSize = udpSocket2.send_to(buffer.data(), serverEndpoint);
    buffer.consume(sentSize);

    // Comm test 3

    boost::asio::streambuf newBuffer;
    udpSocket2.receive_from(boost::asio::null_buffers(), serverEndpoint);
    available = udpSocket2.available();
    recSize = udpSocket2.receive_from(newBuffer.prepare(available), serverEndpoint);
    std::istream newIstream(&newBuffer);

    newBuffer.commit(sizeof(uint8_t));
    newIstream >> inMessageType;
    newBuffer.commit(recSize - 1);
    ChronoMessages::MessagePacket recPacket;
    recPacket.ParseFromIstream(&newIstream);
    if (recPacket.DebugString().compare(packet.DebugString()) == 0 && inMessageType == MESSAGE_PACKET) {
        std::cout << "PASSED -- Server communication test 3" << std::endl;
    } else std::cout << "FAILED -- Server communication test 3" << std::endl;

    // Comm test 5

    messageType = MESSAGE_PACKET;
    outStream << messageType;
    packet.SerializeToOstream(&outStream);
    sentSize = udpSocket2.send_to(buffer.data(), serverEndpoint);
    buffer.consume(sentSize);

    // Comm test 4
    boost::asio::streambuf anotherBuffer;
    std::istream anotherIstream(&anotherBuffer);
    udpSocket2.receive_from(boost::asio::null_buffers(), serverEndpoint);
    available = udpSocket2.available();
    recSize = udpSocket2.receive_from(anotherBuffer.prepare(available), serverEndpoint);
    anotherBuffer.commit(recSize);
    anotherIstream >> inMessageType;
    ChronoMessages::VehicleMessage recVehicle;
    recVehicle.ParseFromIstream(&anotherIstream);
    if (recVehicle.DebugString().compare(sendVehicle.DebugString()) == 0 && inMessageType == VEHICLE_MESSAGE) {
        std::cout << "PASSED -- Server communication test 4" << std::endl;
    } else std::cout << "FAILED -- Server communication test 4" << std::endl;
    udpSocket2.close();

    // Server - client communication tests ///////////////////////////////////////////////

    ChClientHandler handler("localhost", "8082");
    handler.beginListen();
    handler.beginSend();
    handler.pushMessage(sendVehicle);
    handler.pushMessage(packet);

    auto message1 = std::static_pointer_cast<ChronoMessages::VehicleMessage>(handler.popSimMessage());
    if (message1->DebugString().compare(sendVehicle.DebugString()) == 0) {
        std::cout << "PASSED -- Server - client communication test 3" << std::endl;
    } else std::cout << "FAILED -- Server - client communication test 3" << std::endl;

    auto newMessage1 = handler.popDSRCMessage();
    auto newMessage2 = handler.popDSRCMessage();
    auto newMessage3 = handler.popDSRCMessage();
    auto newMessage4 = handler.popDSRCMessage();

    auto vMessage1 = std::static_pointer_cast<ChronoMessages::VehicleMessage>(handler.popSimMessage());
    auto vMessage2 = std::static_pointer_cast<ChronoMessages::VehicleMessage>(handler.popSimMessage());
    auto vMessage3 = std::static_pointer_cast<ChronoMessages::VehicleMessage>(handler.popSimMessage());

    if (newMessage1->buffer().compare(Dmessage1) == 0 && newMessage2->buffer().compare(Dmessage1) == 0 && newMessage3->buffer().compare(Dmessage1) == 0 && newMessage4->buffer().compare(Dmessage1) == 0 && vMessage1->DebugString().compare(generateVehicleMessageFromWheeledVehicle(&my_hmmwv.GetVehicle(), 0, 0).DebugString()) == 0 && vMessage2->DebugString().compare(generateVehicleMessageFromWheeledVehicle(&my_hmmwv.GetVehicle(), 0, 0).DebugString()) == 0 && vMessage3->DebugString().compare(generateVehicleMessageFromWheeledVehicle(&my_hmmwv.GetVehicle(), 0, 0).DebugString()) == 0) {
        std::cout << "PASSED -- Server - client communication test 4" << std::endl;
    } else std::cout << "FAILED -- Server - client communication test 4" << std::endl;

    handler.pushMessage(packet);

    server.join();

    return 0;
}
