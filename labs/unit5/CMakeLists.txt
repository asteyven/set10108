cmake_minimum_required(VERSION 3.17)
project(set10108-unit5 LANGUAGES CXX CUDA)
set(CMAKE_CXX_STANDARD 17)
include_directories(../contrib)

find_package(OpenCL)
find_package(CUDAToolkit)
include_directories(${OpenCL_INCLUDE_DIRS})
link_libraries(${OpenCL_LIBRARIES} ${CUDA_curand_LIBRARY})

add_executable(monte-carlo-pi-cl monte-carlo-pi.cpp)
add_executable(monte-carlo-cuda monte-carlo-pi.cu)
set_property(TARGET monte-carlo-cuda PROPERTY CUDA_SEPARABLE_COMPILATION ON)
add_executable(mandelbrot-cl mandelbrot.cpp)
add_executable(image-rotate-cl image-rotate.cpp)

# This will copy the kernel files to the build output
configure_file(monte-carlo-pi.cl monte-carlo-pi.cl COPYONLY)
configure_file(mandelbrot.cl mandelbrot.cl COPYONLY)
configure_file(image-rotate.cl image-rotate.cl COPYONLY)

# Copy image across
configure_file(pic.png pic.png COPYONLY)
