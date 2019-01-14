cmake_minimum_required(VERSION 3.5.1)
project(Thread-Pool-C VERSION 0.6  LANGUAGES C)

find_package (Threads)

set(HEADERS_FILES
        atomic_defs.h
        linklist.h
        threadpool.h
        )
set(SOURCE_FILES
        linklist.c
        threadpool.c
        )

add_executable(ThreadPoolThreadSafeTest thread_pool_tester_thread_safe.c ${HEADERS_FILES} ${SOURCE_FILES})
target_link_libraries (ThreadPoolThreadSafeTest ${CMAKE_THREAD_LIBS_INIT})

add_executable(ThreadPoolNotThreadSafeTest thread_pool_tester_NOT_thread_safe.c ${HEADERS_FILES} ${SOURCE_FILES})
target_link_libraries (ThreadPoolNotThreadSafeTest ${CMAKE_THREAD_LIBS_INIT})

add_executable(ThreadPoolUnitTest thread_pool_unit_tests.c ${HEADERS_FILES} ${SOURCE_FILES})
target_link_libraries (ThreadPoolUnitTest ${CMAKE_THREAD_LIBS_INIT})