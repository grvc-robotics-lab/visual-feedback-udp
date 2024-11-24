cmake_minimum_required(VERSION 2.8)

find_package( OpenCV REQUIRED )

include_directories( ${VISUALFEEDBACK_SOURCE_DIR}/UDPImage )

link_directories( ${VISUALFEEDBACK_BINARY_DIR}/UDPImage )

add_executable( VisualFeedback Main.cpp )

target_link_libraries( VisualFeedback UDPImage ${OpenCV_LIBS} -pthread )

