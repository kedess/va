cmake_minimum_required(VERSION 3.21)

project(va VERSION 0.6.0)

configure_file (version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h @ONLY)

add_subdirectory(va-recorder dist/va-recorder)
add_subdirectory(va-hls dist/va-hls)
add_subdirectory(va-inference dist/va-inference)
if (TEST)
    enable_testing()
    add_subdirectory(tests tests)
endif ()


