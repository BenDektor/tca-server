cmake_minimum_required(VERSION 3.2.0)
project(kamera_analyse)

# Set C++ standard to C++11 and enforce it
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(kamera_analyse 
            kamera_analyse.h kamera_analyse.cpp 
            socket_test/pc_socket.cpp socket_test/pc_socket.h 
            lane_detection/lane_handler/lane_handler.cpp lane_detection/lane_handler/lane_handler.h
            lane_detection/lane_finder/Strasenfinder2.cpp lane_detection/lane_finder/Strasenfinder2.h
            lane_detection/image_preprocessing/ImagePreProcessing.cpp lane_detection/image_preprocessing/ImagePreProcessing.h
            pid_controller.cpp pid_controller.h)
target_link_libraries(kamera_analyse ${OpenCV_LIBS})




