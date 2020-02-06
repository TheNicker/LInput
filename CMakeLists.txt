# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
include_directories("./Include")
include_directories("D:/development/LLUtils/Include")

add_subdirectory("Example")