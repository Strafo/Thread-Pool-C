cmake_minimum_required(VERSION 3.5.1)
project(Thread-Pool-C VERSION 0.6  LANGUAGES C)
enable_testing()
add_subdirectory(test)
set(HEADERS_FILES
        atomic_defs.h
        linklist.h
        threadpool.h
        )
set(SOURCE_FILES
        linklist.c
        threadpool.c
        )


add_test(NAME UnitTest COMMAND ThreadPoolUnitTest)
add_test(NAME ThSafeTest COMMAND ThreadPoolThreadSafeTest)
add_test(NAME NotThSafeTest COMMAND ThreadPoolNotThreadSafeTest)
