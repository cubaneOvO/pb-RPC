cmake_minimum_required(VERSION 3.12)

project(tnlib
        VERSION 0.0.1
        LANGUAGES CXX
    )

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -g") 

include(3rd.cmake)

# 全局包含头文件根路径，可通过该路径查头文件
include_directories(${PROJECT_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/deploy/bins)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/deploy/libs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/deploy/libs)

add_subdirectory(proto)
add_subdirectory(src)

#add_subdirectory(test)