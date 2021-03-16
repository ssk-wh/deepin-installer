cmake_minimum_required(VERSION 3.1)
project(utest)

add_subdirectory(googletest)

# demo测试用例
add_executable(demo_test demo_test_cases.cpp)
target_link_libraries(demo_test gtest gtest_main)

# 实际的测试用例
## ...

enable_testing()
