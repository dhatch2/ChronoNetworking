SET(MODEL_FILES
    ../Vehicle_Protobuf_Messages/${PROTO_SRCS}
    ../Vehicle_Protobuf_Messages/${PROTO_HDRS}
    World/World.cpp
    World/World.h
    ../network-handler/ChSafeQueue.h
    ../network-handler/ChNetworkHandler.h
    ../network-handler/ChNetworkHandler.cpp
)

SOURCE_GROUP("subsystems" FILES ${MODEL_FILES})

include_directories(${BOOST_DIR} ../Vehicle_Protobuf_Messages World ../network-handler)

add_executable(CAVE-Server CAVE-Server.cpp ${MODEL_FILES})

target_link_libraries(CAVE-Server boost_system pthread protobuf)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_subdirectory(World)
