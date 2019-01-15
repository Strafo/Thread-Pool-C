cmake_minimum_required(VERSION 3.5.1)
project(Thread-Pool-C VERSION 0.6 LANGUAGES C)


include_directories(src)
include_directories(tests_dir)
add_subdirectory(src)
add_subdirectory(tests_dir)
