# Model files for the DSRC Agent

SET(MODEL_FILES
    ChDSRCAgent.cpp
    ChDSRCAgent.h
)

SOURCE_GROUP("subsystems" FILES ${MODEL_FILES})

#--------------------------------------------------------------
# === 2 ===
# Find the Chrono package and any REQUIRED or OPTIONAL modules
# by invoking the find_package function in CONFIG mode:
#    find_package(Chrono
#	              COMPONENTS req_module1 req_module1 ...
#	              OPTIONAL opt_module1 opt_module2 ...
#                 CONFIG)
# The following Chrono modules can be requested (their names
# are case insensitive): Cascade, Cosimulation, FEA, Irrlicht,
# Matlab, Parallel, Postprocess, Python, Vehicle.
#
# Note that you will have to set the variable Chrono_DIR to
# specify the location of the ChronoConfig.cmake script. This
# can be either a Chrono build tree or a Chrono install tree.
#
# The following variables are set and can be used further down:
# CHRONO_FOUND
#   set to true if Chrono and all required components were found
# CHRONO_C_FLAGS
# CHRONO_CXX_FLAGS
#   C and C++ compilation flags
# CHRONO_INCLUDE_DIRS
#   additional paths for included headers
# CHRONO_LIBRARIES
#   list of required libraries (with full path)
# CHRONO_LINKER_FLAGS
#   additional linker flags
# CHRONO_DLLS
#   list of all DLL dependencies (with full path)
# CHRONO_DATA_DIR
#   path to the Chrono data make_directory
#
# In addition, for each requested component [COMPONENT], the
# following variable is set to true (ON) or false (OFF):
# CHRONO_[COMPONENT]_FOUND
#
# In this example, we only request the Irrlicht module (required)
#--------------------------------------------------------------


#--------------------------------------------------------------
# Add path to Chrono headers and to headers of all dependencies
# of the requested modules.
#--------------------------------------------------------------

include_directories(${CHRONO_INCLUDE_DIRS})

#--------------------------------------------------------------
# === 3 ===
# Add the executable from your project and specify all C++
# files in your project.
# If irrlicht is not used, only the blind autonomous client is produced.
#--------------------------------------------------------------

add_executable(DSRC-test dsrc-tests.cpp ${MODEL_FILES})

#--------------------------------------------------------------
# Set properties for your executable target
#
# Note that here we define a macro CHRONO_DATA_DIR which will
# contain the path to the Chrono data directory, either in its
# source tree (if using a build version of Chrono), or in its
# install tree (if using an installed version of Chrono).
#--------------------------------------------------------------

set_target_properties(DSRC-test PROPERTIES
COMPILE_FLAGS "${CHRONO_CXX_FLAGS} ${EXTRA_COMPILE_FLAGS}"
COMPILE_DEFINITIONS "CHRONO_DATA_DIR=\"${CHRONO_DATA_DIR}\""
LINK_FLAGS "${CHRONO_LINKER_FLAGS}")

#--------------------------------------------------------------
# Link to Chrono libraries and dependency libraries   ----  Added boost_system, pthread, and protobuf to allow network message passing.
#--------------------------------------------------------------

target_link_libraries(DSRC-test ${CHRONO_LIBRARIES})

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
add_DLL_copy_command("${CHRONO_DLLS}")
