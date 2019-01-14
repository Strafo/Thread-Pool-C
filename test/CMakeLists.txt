include_directories(${PROJECT_SOURCE_DIR}/src)
link_directories(${PROJECT_SOURCE_DIR}/src)
add_executable(ThreadPoolThreadSafeTest thread_pool_tester_thread_safe.c)
target_link_libraries(ThreadPoolThreadSafeTest ${PROJECT_SOURCE_DIR}/src)