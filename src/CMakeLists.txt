cmake_minimum_required(VERSION 3.5)
project(InvertedIndexDNF)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fpic -g")

set(SOURCE_FILES InvertedIndexDNF.cpp)

ADD_SUBDIRECTORY("utils")
ADD_SUBDIRECTORY("index")

add_executable(InvertedIndexDNF ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(InvertedIndexDNF stdc++ jsoncpp)
