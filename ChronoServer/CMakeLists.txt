# Set model files for the ChronoServer

SET(MODEL_FILES
    ../Vehicle_Protobuf_Messages/${PROTO_SRCS}
    ../Vehicle_Protobuf_Messages/${PROTO_HDRS}
    ../Vehicle_Protobuf_Messages/MessageCodes.h
    main.cpp
    World.cpp
    World.h
)

set_source_files_properties(${PROTO_SRCS} ${PROTO_HDRS} PROPERTIES
                            GENERATED TRUE)

SOURCE_GROUP("subsystems" FILES ${MODEL_FILES})

#--------------------------------------------------------------
# Add path to Chrono headers and to headers of all dependencies
# of the requested modules. -- Added protobuf messages and boost include path
#--------------------------------------------------------------

include_directories(${BOOST_DIR} ${PROTOBUF_INCLUDE_DIRS} ../Vehicle_Protobuf_Messages)

#--------------------------------------------------------------
# === 3 ===
# Add the executable from your project and specify all C++
# files in your project.
#--------------------------------------------------------------

add_executable(ChServer ${MODEL_FILES})

target_link_libraries(ChServer boost_system pthread protobuf)
link_directories(${MyProj_BINARY_DIR}/Vehicle_Protobuf_Messages)

#--------------------------------------------------------------
# === 4 (OPTIONAL) ===
#
# Optionally, add a custom command for copying all Chrono and
# dependency DLLs to the appropriate binary output folder.
# This function has effect only on Windows.
#
# Note that you must first set EXECUTABLE_OUTPUT_PATH
# (this can simply be ${CMAKE_BINARY_DIR}, like in this example)
#--------------------------------------------------------------

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
#add_DLL_copy_command("${CHRONO_DLLS}")
