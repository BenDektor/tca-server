cmake_minimum_required(VERSION 3.2.0)
project(laneFinder)

# Set C++ standard to C++11 and enforce it
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

#find_package(OpenCV REQUIRED)
#include_directories(${OpenCV_INCLUDE_DIRS})
#add_executable(laneDetection laneDetection.h laneDetection.cpp image_preprocessing/ImagePreProcessing.h image_preprocessing/ImagePreProcessing.cpp)
#target_link_libraries(laneDetection ${OpenCV_LIBS})


find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(laneFinder laneFindingAlgorithmus.h laneFindingAlgorithmus.cpp image_preprocessing/ImagePreProcessing.h image_preprocessing/ImagePreProcessing.cpp)
target_link_libraries(laneFinder ${OpenCV_LIBS})
