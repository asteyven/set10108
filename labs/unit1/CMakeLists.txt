cmake_minimum_required(VERSION 3.17)
project(set10108-unit1)
set(CMAKE_CXX_STANDARD 17)
include_directories(../contrib)

add_executable(01_gathering-data timing.cpp)
add_executable(02_linear-index linear-index.cpp)
add_executable(03_lambda lambda.cpp)