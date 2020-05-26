# CMakeList.txt : CMake project for GFXRendererD3D11, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)


set(CMAKE_CXX_STANDARD 17)
include_directories(./LLUtils/Include)
include_directories(d:/development/Win32/Win32/Include)

# Add source to this project's executable.
add_executable (LInputExample "Example.cpp")

target_link_libraries(LInputExample 
Win32
hid )

if(MSVC)
  #target_compile_options(Example PRIVATE /W4 /WX)
else()
  target_compile_options(LInputExample PRIVATE -Wall -Wextra -pedantic -Werror)
endif()