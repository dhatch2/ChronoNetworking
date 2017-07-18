SET(MODEL_FILES
    ../../network-handler/ChSafeQueue.h
    ../../network-handler/ChNetworkHandler.h
    ../../network-handler/ChNetworkHandler.cpp
    ../../Vehicle_Protobuf_Messages/${PROTO_SRCS}
    ../../Vehicle_Protobuf_Messages/${PROTO_HDRS}
    ../../Vehicle_Protobuf_Messages/MessageCodes.h
    MessageConversions.h
    MessageConversions.cpp
    ../../CAVE-server/World/World.h
    ../../CAVE-server/World/World.cpp
    ../../ChronoClient/ServerVehicle.cpp
    ../../ChronoClient/ServerVehicle.h
    )

SOURCE_GROUP("subsystems" FILES ${MODEL_FILES})

include_directories(${CHRONO_INCLUDE_DIRS} ${BOOST_DIR} ${PROTOBUF_INCLUDE_DIRS} ../../Vehicle_Protobuf_Messages ../../network-handler ../../CAVE-server/World ../../ChronoClient)

add_executable(sim-tests sim-tests.cpp ${MODEL_FILES})

set_target_properties(sim-tests PROPERTIES
COMPILE_FLAGS "${CHRONO_CXX_FLAGS} ${EXTRA_COMPILE_FLAGS}"
COMPILE_DEFINITIONS "CHRONO_DATA_DIR=\"${CHRONO_DATA_DIR}\""
LINK_FLAGS "${CHRONO_LINKER_FLAGS}")

target_link_libraries(sim-tests ${CHRONO_LIBRARIES} protobuf boost_system pthread)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
add_DLL_copy_command("${CHRONO_DLLS}")
