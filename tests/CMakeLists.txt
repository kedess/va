cmake_minimum_required(VERSION 3.21)
project(tests)

set(CMAKE_CXX_STANDARD 20)

find_package(GTest REQUIRED)

add_executable(source-test source.cpp)
target_link_libraries(source-test ${GTEST_LIBRARIES})
add_test(source-test "./source-test")

add_executable(utils-test utils.cpp ../va-recorder/src/utils.cpp)
target_link_libraries(utils-test ${GTEST_LIBRARIES})
add_test(utils-test "./utils-test")