cmake_minimum_required(VERSION 2.8)
project(Inertia)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -std=c11")
set(CMAKE_C_COMPILER "/usr/local/bin/x86_64-apple-darwin15.6.0-gcc-6")
set(SOURCE_FILES src/main.c)
add_executable(Inertia ${SOURCE_FILES})
