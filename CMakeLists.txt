cmake_minimum_required(VERSION 2.8)

project( VisualFeedback )

find_package( OpenCV REQUIRED )

add_subdirectory( UDPImage )

add_subdirectory( Main )

