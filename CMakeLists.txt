cmake_minimum_required (VERSION 2.6)
project(examples)

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

include_directories("${PROJECT_SOURCE_DIR}/include/")
include_directories("${PROJECT_SOURCE_DIR}/3rdparty/msgpack-c/include")
include_directories("${PROJECT_SOURCE_DIR}/3rdparty/rapidjson/include")

add_executable(simple1 ${PROJECT_SOURCE_DIR}/examples/sample1.cpp)