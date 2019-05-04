cmake_minimum_required(VERSION 3.5.1)
project(Thread-Pool-C VERSION 0.6  LANGUAGES C)
add_subdirectory(src)
add_subdirectory(test)


enable_testing()
add_test(NAME UnitTest COMMAND ThreadPoolUnitTest)
add_test(NAME ThSafeTest COMMAND ThreadPoolThreadSafeTest)
add_test(NAME NotThSafeTest COMMAND ThreadPoolNotThreadSafeTest)
