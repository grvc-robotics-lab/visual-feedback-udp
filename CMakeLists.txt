cmake_minimum_required(VERSION 2.8)

project( VisualFeedback_Display )

find_package( OpenCV REQUIRED )

add_executable( VisualFeedback_Display Main.cpp )

target_link_libraries( VisualFeedback_Display ${OpenCV_LIBS} -pthread )
