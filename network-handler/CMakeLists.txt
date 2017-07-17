SET(MODEL_FILES
    ChSafeQueue.h
    ChNetworkHandler.h
    ChNetworkHandler.cpp
    ../Vehicle_Protobuf_Messages/${PROTO_SRCS}
    ../Vehicle_Protobuf_Messages/${PROTO_HDRS}
    ../Vehicle_Protobuf_Messages/MessageCodes.h
    ../CAVE-client/chrono-sim/MessageConversions.h
    ../CAVE-client/chrono-sim/MessageConversions.cpp
    ../CAVE-server/World/World.h
    ../CAVE-server/World/World.cpp
)

SOURCE_GROUP("subsystems" FILES ${MODEL_FILES})

include_directories(${CHRONO_INCLUDE_DIRS} ${BOOST_DIR} ${PROTOBUF_INCLUDE_DIRS} .. ../Vehicle_Protobuf_Messages ../CAVE-client/chrono-sim ../CAVE-server/World)

add_executable(network-tests network-tests.cpp ${MODEL_FILES})

set_target_properties(network-tests PROPERTIES
COMPILE_FLAGS "${CHRONO_CXX_FLAGS} ${EXTRA_COMPILE_FLAGS}"
COMPILE_DEFINITIONS "CHRONO_DATA_DIR=\"${CHRONO_DATA_DIR}\""
LINK_FLAGS "${CHRONO_LINKER_FLAGS}")

target_link_libraries(network-tests ${CHRONO_LIBRARIES} protobuf boost_system pthread)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
add_DLL_copy_command("${CHRONO_DLLS}")
