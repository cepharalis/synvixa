cmake_minimum_required(VERSION 4.3.1)

project(synvixa)

set(CMAKE_C_STANDARD 23)

file(GLOB_RECURSE SRC_FILE CONFIGURE_DEPEnDS src/*.c)
add_executable(synvixa ${SRC_FILE})

target_include_directories(synvixa PUBLIC src)