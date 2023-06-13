cmake_minimum_required(VERSION 3.22)

project(lexy-example)

include(FetchContent)
FetchContent_Declare(lexy URL https://lexy.foonathan.net/download/lexy-src.zip)
FetchContent_MakeAvailable(lexy)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(lexy_example)
target_sources(lexy_example PRIVATE main.cpp)
target_link_libraries(lexy_example PRIVATE foonathan::lexy)
