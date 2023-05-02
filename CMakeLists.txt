cmake_minimum_required(VERSION 2.8)
project( camera-calib )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( run_calib_v1 calib_v1.cpp )
target_link_libraries( run_calib_v1 ${OpenCV_LIBS} )

add_executable( run_calib_w_pictures calib_w_pictures.cpp )
target_link_libraries( run_calib_w_pictures ${OpenCV_LIBS} )