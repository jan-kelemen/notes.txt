cmake_minimum_required(VERSION 3.24)

project(read_file CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED)
find_package(liburing REQUIRED)

add_executable(read_file src/main.cpp)

target_compile_definitions(read_file
    PRIVATE
        BOOST_ASIO_HAS_IO_URING
        BOOST_ASIO_DISABLE_EPOLL)

target_link_libraries(read_file PRIVATE Boost::headers liburing::liburing)

