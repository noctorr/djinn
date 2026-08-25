cmake_minimum_required(VERSION 3.10)

project(Djinn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 ")
add_compile_options("-Wall" "-Wextra" "-pedantic")

file(GLOB DJINN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(program "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
target_include_directories(program PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")