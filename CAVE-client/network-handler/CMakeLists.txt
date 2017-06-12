SET(MODEL_FILES
    ../ChSafeQueue.h
    ChNetworkHandler.h
    )

SOURCE_GROUP("subsystems" FILES ${MODEL_FILES})

include_directories(${BOOST_DIR} ${PROTOBUF_INCLUDE_DIRS} .. ../../Vehicle_Protobuf_Messages)

add_executable(network-tests network-tests.cpp ${MODEL_FILES})

target_link_libraries(network-tests protobuf boost_system pthread)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
