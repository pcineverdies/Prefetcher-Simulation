cmake_minimum_required(VERSION 2.8)

project( DetectFacesEmbeddedBenchmark )

find_package( OpenCV REQUIRED )

include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable( DetectFacesEmbeddedBenchmark DetectFacesEmbeddedBenchmark.cpp )

target_link_libraries( DetectFacesEmbeddedBenchmark ~/Projects/RM4ES/libfacedetection/build/install/lib/libfacedetection.so)
target_link_libraries( DetectFacesEmbeddedBenchmark ${OpenCV_LIBS} )
