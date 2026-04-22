cmake_minimum_required(VERSION 3.31)

project(BoostStringViewTest)

find_package(Boost REQUIRED CONFIG)

add_executable(BoostStringViewTest)

target_sources(BoostStringViewTest
    PRIVATE
        test.cpp
)

target_link_libraries(BoostStringViewTest
    PRIVATE
        Boost::headers
)

add_executable(BoostStringViewAliasedTest)

target_sources(BoostStringViewAliasedTest
    PRIVATE
        test_aliased.cpp
)

target_link_libraries(BoostStringViewAliasedTest
    PRIVATE
        Boost::headers
)
