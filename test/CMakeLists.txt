cmake_minimum_required(VERSION 3.5.1)
project(Thread-Pool-C-Test VERSION 0.6  LANGUAGES C)


include_directories(minunitdir)

set(MINUNIT minunitdir/minunit.h )


add_executable(ThreadPoolThreadSafeTest thread_pool_tester_thread_safe.c )
target_link_libraries (ThreadPoolThreadSafeTest
        ThreadPool
        )

add_executable(ThreadPoolNotThreadSafeTest thread_pool_tester_NOT_thread_safe.c )
target_link_libraries (ThreadPoolNotThreadSafeTest
        ThreadPool
        )

add_executable(ThreadPoolUnitTest thread_pool_unit_tests.c  ${MINUNIT})
target_link_libraries (ThreadPoolUnitTest
        ThreadPool
        )


