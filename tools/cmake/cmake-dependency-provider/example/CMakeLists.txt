cmake_minimum_required(VERSION 3.24)

project(cmake-dependency-provider)

find_package(lexy REQUIRED)
find_package(fmt REQUIRED)
find_package(date REQUIRED)
find_package(Catch2 REQUIRED)
find_package(cppzmq REQUIRED)
find_package(flatbuffers REQUIRED)
find_package(Boost REQUIRED)

build_flatbuffers(
    "wire.fbs" # Schema files
    "" # Schema include directories 
    "generate_fb" # Target name 
    "" # Additional dependencies
    "${CMAKE_CURRENT_BINARY_DIR}/include" # Generated includes directory
    "" # Binary schema directory
    "" # Text schemas directory
)

add_executable(cmake-dependency-provider)

target_sources(cmake-dependency-provider PRIVATE main.cpp)

target_link_libraries(cmake-dependency-provider 
    PRIVATE
        flatbuffers
        foonathan::lexy
        Boost::program_options
        fmt::fmt
        date::date
        cppzmq
)
add_dependencies(cmake-dependency-provider generate_fb)

add_executable(cmake-dependency-provider-tests)

target_sources(cmake-dependency-provider-tests PRIVATE test.cpp)

target_link_libraries(cmake-dependency-provider-tests
    PRIVATE
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(cmake-dependency-provider-tests)
