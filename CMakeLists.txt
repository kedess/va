cmake_minimum_required(VERSION 3.21)

project(va)

add_subdirectory(va-recorder dist/va-recorder)
if (TEST)
    enable_testing()
    add_subdirectory(tests tests)
endif ()


