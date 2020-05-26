# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
include_directories(./External/Win32/Win32//Include)
include_directories(./External/LLUtils/Include)
include_directories(./Include)
add_subdirectory(./External/Win32/)

add_subdirectory("Example")