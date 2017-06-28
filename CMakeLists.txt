#-----------------------------------------------------------------------------
# Enable C++11 support
#-----------------------------------------------------------------------------

#find_package(ModernCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CH_CXX_FLAGS "${CH_CXX_FLAGS} -std=c++11")

# TODO This might not work on windows

cmake_minimum_required(VERSION 2.8)

#--------------------------------------------------------------
# === 1 ===
# Modify the project name if you want:
#--------------------------------------------------------------

project(ChronoNetworking)

include(FindProtobuf)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build.")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)
set(CPACK_BUILD_TYPE ${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CH_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CH_CXX_FLAGS} -D_DEBUG -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CH_CXX_FLAGS} -DNDEBUG")

find_package( Protobuf REQUIRED )
include_directories( ${PROTOBUF_INCLUDE_DIRS} )

set(USE_IRRLICHT OFF CACHE BOOL OFF)

if(USE_IRRLICHT)
find_package(Chrono
COMPONENTS Irrlicht Vehicle
CONFIG)
else()
find_package(Chrono
COMPONENTS Vehicle
CONFIG)
endif()

#--------------------------------------------------------------
# Return now if Chrono or a required component was not found.
#--------------------------------------------------------------

if (NOT Chrono_FOUND)
message("Could not find Chrono or one of its required modules")
return()
endif()


#--------------------------------------------------------------
# Add path to Chrono headers and to headers of all dependencies
# of the requested modules. -- Added protobuf messages and boost include path
#--------------------------------------------------------------

if(NOT DEFINED BOOST_DIR)
set(BOOST_DIR "boost include directory" CACHE PATH "" FORCE)
endif()

#--------------------------------------------------------------
# Tweaks to disable some warnings with MSVC
#--------------------------------------------------------------
if(MSVC)
add_definitions("-D_CRT_SECURE_NO_DEPRECATE")  # avoids deprecation warnings
add_definitions("-D_SCL_SECURE_NO_DEPRECATE")  # avoids deprecation warnings
set(EXTRA_COMPILE_FLAGS "/wd4275")             # disable warnings triggered by Irrlicht
else()
set(EXTRA_COMPILE_FLAGS "")
endif()

add_subdirectory(Vehicle_Protobuf_Messages)
add_subdirectory(network-handler)
add_subdirectory(ChronoServer)
add_subdirectory(ChronoClient)
add_subdirectory(ChronoUDPServer)
add_subdirectory(CAVE-client)
add_subdirectory(CAVE-server)
