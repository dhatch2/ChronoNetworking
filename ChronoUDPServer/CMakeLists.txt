# UPD server model files

SET(MODEL_FILES
    ../Vehicle_Protobuf_Messages/${PROTO_SRCS}
    ../Vehicle_Protobuf_Messages/${PROTO_HDRS}
    ../Vehicle_Protobuf_Messages/MessageCodes.h
    main.cpp
    ../ChronoServer/World.cpp
    ../ChronoServer/World.h
)

set_source_files_properties(${PROTO_SRCS} ${PROTO_HDRS} PROPERTIES
                            GENERATED TRUE)

SOURCE_GROUP("subsystems" FILES ${MODEL_FILES})

#--------------------------------------------------------------
# Add path to Chrono headers and to headers of all dependencies
# of the requested modules. -- Added protobuf messages and boost include path
#--------------------------------------------------------------

include_directories(${BOOST_DIR} ../ChronoServer ../Vehicle_Protobuf_Messages)

#--------------------------------------------------------------
# === 3 ===
# Add the executable from your project and specify all C++
# files in your project.
#--------------------------------------------------------------

add_executable(ChUDPServer ${MODEL_FILES})

#--------------------------------------------------------------
# Set properties for your executable target
#
# Note that here we define a macro CHRONO_DATA_DIR which will
# contain the path to the Chrono data directory, either in its
# source tree (if using a build version of Chrono), or in its
# install tree (if using an installed version of Chrono).
#--------------------------------------------------------------

#set_target_properties(ChronoServer PROPERTIES
#	    COMPILE_FLAGS "${CHRONO_CXX_FLAGS} ${EXTRA_COMPILE_FLAGS}"
#	    COMPILE_DEFINITIONS "CHRONO_DATA_DIR=\"${CHRONO_DATA_DIR}\""
#	    LINK_FLAGS "${CHRONO_LINKER_FLAGS}")

#--------------------------------------------------------------
# Link to Chrono libraries and dependency libraries   ----  Added
# boost_system, pthread, and protobuf to allow network message passing.
#--------------------------------------------------------------

target_link_libraries(ChUDPServer boost_system pthread protobuf)

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
