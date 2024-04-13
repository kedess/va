cmake_minimum_required(VERSION 3.21)
project(va-recorder VERSION 0.2.0)

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

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavdevice libavcodec libavformat libavutil)

find_package( Boost 1.74 COMPONENTS log program_options system thread REQUIRED)
include_directories( ${Boost_INCLUDE_DIR} )

set(SRC_FILES
    src/main.cpp
    src/source/source.cpp
    src/state/state.cpp
    src/capture/ffmpeg/avformat.cpp
    src/capture/ffmpeg/avpacket.cpp
    src/capture/ffmpeg/avframe.cpp
    src/capture/ffmpeg/backend.cpp
    src/capture/ffmpeg/codec.cpp
    src/capture/ffmpeg/archive.cpp
)

include_directories(${FFMPEG_INCLUDE_DIRS})

configure_file (version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h @ONLY)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBRARIES} ${Boost_LIBRARIES})