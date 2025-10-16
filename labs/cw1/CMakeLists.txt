project(set10108-cw1)
cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 17)

include_directories(../contrib)
if (WIN32)
include_directories(../contrib/sfml/include)

link_directories(../contrib/sfml/lib/Debug)
link_directories(../contrib/sfml/lib/Release)
else()
find_package(SFML 2.5 COMPONENTS window graphics system REQUIRED)
endif()

add_executable(cw1 main.cpp)

target_link_libraries(cw1 optimized sfml-system optimized sfml-window optimized sfml-graphics debug sfml-system-d debug sfml-window-d debug sfml-graphics-d)


add_custom_command(TARGET cw1 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/../contrib/sfml/lib/Debug"       # Source folder with DLLs
    "$<TARGET_FILE_DIR:cw1>" # Destination: the directory of the executable
)
add_custom_command(TARGET cw1 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/../contrib/sfml/lib/Release"       # Source folder with DLLs
    "$<TARGET_FILE_DIR:cw1>" # Destination: the directory of the executable
)
