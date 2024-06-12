cmake_minimum_required(VERSION 3.2.0)
project(socket)

# Set C++ standard to C++11 and enforce it
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(pc_socket pc_socket.h pc_socket.cpp)
target_link_libraries(pc_socket ${OpenCV_LIBS})
