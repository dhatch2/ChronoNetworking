# Model files for the World server object

SET(MODEL_FILES
    ../../Vehicle_Protobuf_Messages/${PROTO_SRCS}
    ../../Vehicle_Protobuf_Messages/${PROTO_HDRS}
    ../../CAVE-client/chrono-sim/MessageConversions.h
    ../../CAVE-client/chrono-sim/MessageConversions.cpp
    World.cpp
    World.h
)

SOURCE_GROUP("subsystems" FILES ${MODEL_FILES})

include_directories(${CHRONO_INCLUDE_DIRS} ${BOOST_DIR} ../../Vehicle_Protobuf_Messages ../../CAVE-client/chrono-sim)

add_executable(world-test world-test.cpp ${MODEL_FILES})

set_target_properties(world-test PROPERTIES
COMPILE_FLAGS "${CHRONO_CXX_FLAGS} ${EXTRA_COMPILE_FLAGS}"
COMPILE_DEFINITIONS "CHRONO_DATA_DIR=\"${CHRONO_DATA_DIR}\""
LINK_FLAGS "${CHRONO_LINKER_FLAGS}")

target_link_libraries(world-test ${CHRONO_LIBRARIES} boost_system pthread protobuf)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
add_DLL_copy_command("${CHRONO_DLLS}")
