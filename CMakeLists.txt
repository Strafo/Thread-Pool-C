cmake_minimum_required(VERSION 3.5.1)
project(Thread-Pool-C VERSION 0.6  LANGUAGES C)

add_definitions(-pthread -Wall)

include_directories(src)
include_directories(test)
add_subdirectory(src)
add_subdirectory(test)
