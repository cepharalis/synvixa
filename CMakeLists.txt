cmake_minimum_required(VERSION 4.3.1)

project(synvixa)

set(CMAKE_C_STANDARD 23)

include(FetchContent)
FetchContent_Declare(pcre2 GIT_REPOSITORY https://github.com/PCRE2Project/pcre2.git GIT_TAG pcre2-10.49)
FetchContent_MakeAvailable(pcre2)

file(GLOB_RECURSE SRC_FILE CONFIGURE_DEPENDS src/*.c)
add_executable(synvixa ${SRC_FILE})

target_include_directories(synvixa PUBLIC src)

target_link_libraries(synvixa PUBLIC pcre2-posix-static)

set_target_properties(synvixa PROPERTIES ENABLE_EXPORTS TRUE)

if(WIN32)
    target_link_options(synvixa PRIVATE "-Wl,--export-all-symbols")
    target_link_libraries(synvixa PRIVATE ws2_32)
endif()

add_subdirectory(plugins)