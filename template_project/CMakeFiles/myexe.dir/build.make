# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project

# Include any dependencies generated for this target.
include CMakeFiles/myexe.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/myexe.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/myexe.dir/flags.make

CMakeFiles/myexe.dir/VehicleServer.cpp.o: CMakeFiles/myexe.dir/flags.make
CMakeFiles/myexe.dir/VehicleServer.cpp.o: VehicleServer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/myexe.dir/VehicleServer.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/myexe.dir/VehicleServer.cpp.o -c /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project/VehicleServer.cpp

CMakeFiles/myexe.dir/VehicleServer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/myexe.dir/VehicleServer.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project/VehicleServer.cpp > CMakeFiles/myexe.dir/VehicleServer.cpp.i

CMakeFiles/myexe.dir/VehicleServer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/myexe.dir/VehicleServer.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project/VehicleServer.cpp -o CMakeFiles/myexe.dir/VehicleServer.cpp.s

CMakeFiles/myexe.dir/VehicleServer.cpp.o.requires:

.PHONY : CMakeFiles/myexe.dir/VehicleServer.cpp.o.requires

CMakeFiles/myexe.dir/VehicleServer.cpp.o.provides: CMakeFiles/myexe.dir/VehicleServer.cpp.o.requires
	$(MAKE) -f CMakeFiles/myexe.dir/build.make CMakeFiles/myexe.dir/VehicleServer.cpp.o.provides.build
.PHONY : CMakeFiles/myexe.dir/VehicleServer.cpp.o.provides

CMakeFiles/myexe.dir/VehicleServer.cpp.o.provides.build: CMakeFiles/myexe.dir/VehicleServer.cpp.o


CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o: CMakeFiles/myexe.dir/flags.make
CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o: /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o -c /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc

CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc > CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.i

CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc -o CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.s

CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o.requires:

.PHONY : CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o.requires

CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o.provides: CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o.requires
	$(MAKE) -f CMakeFiles/myexe.dir/build.make CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o.provides.build
.PHONY : CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o.provides

CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o.provides.build: CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o


# Object files for target myexe
myexe_OBJECTS = \
"CMakeFiles/myexe.dir/VehicleServer.cpp.o" \
"CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o"

# External object files for target myexe
myexe_EXTERNAL_OBJECTS =

myexe: CMakeFiles/myexe.dir/VehicleServer.cpp.o
myexe: CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o
myexe: CMakeFiles/myexe.dir/build.make
myexe: /usr/lib/x86_64-linux-gnu/libIrrlicht.so
myexe: /home/conlain/Desktop/Programs/SBEL/chrono-steering-build/lib/libChronoEngine.so
myexe: /home/conlain/Desktop/Programs/SBEL/chrono-steering-build/lib/libChronoEngine_irrlicht.so
myexe: /home/conlain/Desktop/Programs/SBEL/chrono-steering-build/lib/libChronoEngine_vehicle.so
myexe: /home/conlain/Desktop/Programs/SBEL/chrono-steering-build/lib/libChronoModels_vehicle.so
myexe: CMakeFiles/myexe.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable myexe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/myexe.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/myexe.dir/build: myexe

.PHONY : CMakeFiles/myexe.dir/build

CMakeFiles/myexe.dir/requires: CMakeFiles/myexe.dir/VehicleServer.cpp.o.requires
CMakeFiles/myexe.dir/requires: CMakeFiles/myexe.dir/home/conlain/Desktop/Programs/SBEL/ChronoNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc.o.requires

.PHONY : CMakeFiles/myexe.dir/requires

CMakeFiles/myexe.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/myexe.dir/cmake_clean.cmake
.PHONY : CMakeFiles/myexe.dir/clean

CMakeFiles/myexe.dir/depend:
	cd /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project /home/conlain/Desktop/Programs/SBEL/ChronoNetworking/template_project/CMakeFiles/myexe.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/myexe.dir/depend

