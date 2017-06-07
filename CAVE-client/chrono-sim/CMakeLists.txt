SET(MODEL_FILES
    )

SOURCE_GROUP("subsystems" FILES ${MODEL_FILES})

include_directories(${PROTOBUF_INCLUDE_DIRS} ../../Vehicle_Protobuf_Messages)

add_executable(sim-tests sim-tests.cpp ${MODEL_FILES})

target_link_libraries(sim-tests protobuf pthread)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
