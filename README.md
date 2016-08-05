# ChronoNetworking

To run this software you will need a compiled version of [ProjectChrono](https://github.com/projectchrono/chrono) 
and the full Boost library.

To build the server code: 
- Create a build directory in the project root (i.e. ChronoNetworking/ChronoServer-build)
- In that directory run `cmake ../ChronoServer` 
  + If you have `ccmake`, you may use that instead of `cmake`
- You may need to provide paths to the locations of Boost
- Once CMake has run, simply run `make` in your build directory 

To build the client code: 
- The client is built in the same way as the server, but it also requires a path to a built version of Chrono
- If CMake cannot find Chrono, provide it a path to `<chrono_build_dir>/cmake`
