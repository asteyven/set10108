cmake_minimum_required(VERSION 3.17)
project(set10108-unit4)
set(CMAKE_CXX_STANDARD 17)
include_directories(../contrib)

find_package(OpenCL)
include_directories(${OpenCL_INCLUDE_DIRS})
link_libraries(${OpenCL_LIBRARIES})

add_executable(init-opencl init-opencl.cpp)
add_executable(info-opencl info-opencl.cpp)
add_executable(vec-add vec-add.cpp)

# This will copy the kernel files to the build output
configure_file(vec-add.cl vec-add.cl COPYONLY)
configure_file(matrix-mult.cl matrix-mult.cl COPYONLY)