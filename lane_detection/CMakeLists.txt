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
#add_executable(laneFinder laneFindingAlgorithmus.h laneFindingAlgorithmus.cpp image_preprocessing/ImagePreProcessing.h image_preprocessing/ImagePreProcessing.cpp)
#add_executable(strasenfinder2 lane_finder/Strasenfinder2.h lane_finder/Strasenfinder2.cpp image_preprocessing/ImagePreProcessing.h image_preprocessing/ImagePreProcessing.cpp)
add_executable(lanehandler
                lane_handler/lane_handler.h
                lane_handler/lane_handler.cpp
                lane_finder/Strasenfinder2.h 
                lane_finder/Strasenfinder2.cpp 
                image_preprocessing/ImagePreProcessing.h 
                image_preprocessing/ImagePreProcessing.cpp)
#target_link_libraries(laneFinder ${OpenCV_LIBS})
#target_link_libraries(strasenfinder2 ${OpenCV_LIBS})
target_link_libraries(lanehandler ${OpenCV_LIBS})



