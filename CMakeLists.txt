cmake_minimum_required(VERSION 3.16)
project(TD3Extract)

set(CMAKE_CXX_STANDARD 17)

add_executable(TD3Extract main.cpp lzw.cpp lzw.h file.cpp file.h image.cpp image.h lodepng.cpp)
