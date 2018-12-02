cmake_minimum_required(VERSION 3.5.1)
project(Thread-Pool-C)

file(GLOB SOURCES
        libhl/atomic_defs.h
        libhl/linklist.c
        libhl/linklist.h
        ./threadpool.h
        ./threadpool.c
        debug/debug.c
        debug/debug.h
        debug/thread_pool_tester_NOT_thread_safe.c
        )

set(CMAKE_C_FLAGS "-Wall -pthread")
add_definitions(${CMAKE_C_FLAGS})
add_executable(test_thread_pool ${SOURCES})


#change debug/thread_pool_tester_NOT_thread_safe.c to debug/thread_pool_tester_thread_safe.c to get the second type of tester