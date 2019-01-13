include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(runBasicTests
        thread_pool_tester_NOT_thread_safe.c
        thread_pool_tester_thread_safe.c
        tester.cpp
        )

target_link_libraries(runBasicTests gtest gtest_main)
target_link_libraries(runBasicTests src)