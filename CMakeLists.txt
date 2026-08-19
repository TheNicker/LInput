# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    #enabled warnings flags
    add_compile_options(-Wextra -Wpedantic -Wall -Wpessimizing-move)
    #specifically disable the following warning flags
    add_compile_options(-Wno-c++98-compat -Wno-c++11-compat -Wno-c++14-compat -Wno-c++98-compat-pedantic -Wno-reserved-id-macro -Wno-newline-eof -Wno-global-constructors -Wno-exit-time-destructors -Wno-float-equal -Wno-covered-switch-default -Wno-unreachable-code-break)
 
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
   
# using Visual Studio C++
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/Wall /wd4668 /wd5039 /wd4514 /wd4820 /wd4625 /wd4626 /wd4710 /wd5045" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
endif()

include_directories(./External/LLUtils/Include)
include_directories(./Include)

set(LINPUT_LWS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../LWS" CACHE PATH "Path to LWS")
if(NOT TARGET LWSLib)
    if(EXISTS "${LINPUT_LWS_DIR}/CMakeLists.txt")
        set(LWS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(LWS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        add_subdirectory("${LINPUT_LWS_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/External/LWS")
    else()
        message(FATAL_ERROR "LInput requires LWSLib or a valid LINPUT_LWS_DIR")
    endif()
endif()

include_directories("${LINPUT_LWS_DIR}/src/LWS/include")

option(LINPUT_BUILD_SAMPLES "build LInput samples" ON)

if (LINPUT_BUILD_SAMPLES)
    add_subdirectory("Example")
endif()

