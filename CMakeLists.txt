cmake_minimum_required(VERSION 4.3.1)

project(synvixa)


set(CMAKE_C_STANDARD 23)

file(GLOB_RECURSE SRC_FILE CONFIGURE_DEPENDS src/*.c)
add_executable(synvixa ${SRC_FILE})

target_include_directories(synvixa PUBLIC src)

set_target_properties(synvixa PROPERTIES ENABLE_EXPORTS TRUE)

if(WIN32)
    target_link_options(synvixa PRIVATE "-Wl,--export-all-symbols")
endif()

add_subdirectory(plugins)