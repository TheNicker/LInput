# CMakeList.txt : CMake project for GFXRendererD3D11, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)


set(CMAKE_CXX_STANDARD 17)
include_directories("d:/development/LLUtils/Include")
# Add source to this project's executable.
add_executable (Example "Example.cpp")

TARGET_LINK_LIBRARIES(Example hid )