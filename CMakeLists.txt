cmake_minimum_required(VERSION 3.22)
project(ninja_cpp)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall)

add_executable(njvm
        njvm.cpp
        types.cpp
        instructions.cpp
        loader.cpp
        lib/bigint.c support.cpp
        gc.cpp)
