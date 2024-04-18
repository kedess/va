cmake_minimum_required(VERSION 3.21)
project(va-hls VERSION 0.1.0)

set(SRC_FILES src/pico/picohttpparser.c src/pico/picohttpparser.h)
add_library(pico STATIC ${SRC_FILES})

set(CMAKE_CXX_STANDARD 20)

if (SANITIZER)
    set(CMAKE_CXX_FLAGS "-g -O0")
    add_compile_options(-fno-omit-frame-pointer)
    add_compile_options(
        -fsanitize=address
        -fsanitize=leak
        -fsanitize=undefined
        -fsanitize=null
        -fsanitize=shift
        -fsanitize=unreachable
        -fsanitize=signed-integer-overflow
    )
    add_link_options(
        -fsanitize=address
        -fsanitize=leak
        -fsanitize=undefined
        -fsanitize=null
        -fsanitize=shift
        -fsanitize=unreachable
        -fsanitize=signed-integer-overflow
    )
else()
    set(CMAKE_CXX_FLAGS "-O2")
    add_compile_options(
        -Werror
        -Wall
        -Wextra
        -Wpedantic
        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        -Wenum-compare
        -Wfloat-equal
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wsign-conversion
        -Wsign-promo
        -Wunused
        -Wunused-result
    )
endif ()

find_package( Boost COMPONENTS log program_options system thread REQUIRED)
include_directories( ${Boost_INCLUDE_DIR} )

set(SRC_FILES
    src/main.cpp
    src/state/state.cpp
    src/utils.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} pico)