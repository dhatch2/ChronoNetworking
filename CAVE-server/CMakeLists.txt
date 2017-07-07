SET(MODEL_FILES
    ../Vehicle_Protobuf_Messages/${PROTO_SRCS}
    ../Vehicle_Protobuf_Messages/${PROTO_HDRS}
    World/World.cpp
    World/World.h
    ../network-handler/ChSafeQueue.h
    ../network-handler/ChNetworkHandler.h
    ../network-handler/ChNetworkHandler.cpp
)

SET(TEST_FILES
    ../Vehicle_Protobuf_Messages/${PROTO_SRCS}
    ../Vehicle_Protobuf_Messages/${PROTO_HDRS}
    ../network-handler/ChNetworkHandler.h
    ../network-handler/ChNetworkHandler.cpp
    ../CAVE-client/chrono-sim/MessageConversions.h
    ../CAVE-client/chrono-sim/MessageConversions.cpp
    World/World.cpp
    World/World.h
)

SOURCE_GROUP("subsystems" FILES ${MODEL_FILES})
SOURCE_GROUP("subsystems" FILES ${TEST_FILES})

include_directories(${CHRONO_INCLUDE_DIRS} ${BOOST_DIR} ../CAVE-client/chrono-sim ../Vehicle_Protobuf_Messages World ../network-handler)

add_executable(CAVE-Server CAVE-Server.cpp ${MODEL_FILES})
add_executable(server-test server-test.cpp ${TEST_FILES})

set_target_properties(server-test PROPERTIES
COMPILE_FLAGS "${CHRONO_CXX_FLAGS} ${EXTRA_COMPILE_FLAGS}"
COMPILE_DEFINITIONS "CHRONO_DATA_DIR=\"${CHRONO_DATA_DIR}\""
LINK_FLAGS "${CHRONO_LINKER_FLAGS}")

target_link_libraries(CAVE-Server boost_system pthread protobuf)
target_link_libraries(server-test ${CHRONO_LIBRARIES} boost_system pthread protobuf)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
add_DLL_copy_command("${CHRONO_DLLS}")

add_subdirectory(World)
