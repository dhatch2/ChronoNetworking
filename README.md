# ChronoNetworking

To run this software you will need a compiled version of [ProjectChrono](https://github.com/projectchrono/chrono) 
and the full Boost library.

To build this code:
- Create a build directory in the same directory as ChronoNetworking (i.e. ChronoNetworking-build/)
- cd into that directory and run 'cmake ../ChronoNetworking' or 'ccmake ../ChronoNetworking'
	+ You may need to provide paths to the locations of Boost and Chrono
- Once cmake has finished running, run 'make' in the build directory
- The output executables should be ChServer, ChClient, ChAutoClient, ChBlindClient, and ChUDPServer
- If CMake cannot find Chrono, provide it a path to `<chrono_build_dir>/cmake`

To run:
- Run './ChServer' on a host
- Run './ChClient' on another host, specifying the hostname of the server as a command line argument
- ChClient should now be able to connect to the virtual world of the ChServer
- DRSRTest and ChUDPServer are still in early stages of development
