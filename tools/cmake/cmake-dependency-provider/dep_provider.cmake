cmake_minimum_required(VERSION 3.24)

set (FETCHCONTENT_QUIET OFF)

include(FetchContent)

FetchContent_Declare(
    lexy
    GIT_REPOSITORY https://github.com/foonathan/lexy.git
    GIT_TAG 1b31b097fa4fcaf5465f038793fe88cdc2140b71
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG f5e54359df4c26b6230fc61d38aa294581393084 # Release 10.1.1
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    date
    GIT_REPOSITORY https://github.com/HowardHinnant/date.git
    GIT_TAG 6e921e1b1d21e84a5c82416ba7ecd98e33a436d0 # Release 3.0.1
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG 6e79e682b726f524310d55dec8ddac4e9c52fb5f # Release 3.4.0
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    ZeroMQ
    GIT_REPOSITORY https://github.com/zeromq/libzmq.git
    GIT_TAG 4097855ddaaa65ed7b5e8cb86d143842a594eebd # Release 4.3.4
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    cppzmq
    GIT_REPOSITORY https://github.com/zeromq/cppzmq.git
    GIT_TAG c94c20743ed7d4aa37835a5c46567ab0790d4acc # Release 4.10.0
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    flatbuffers
    GIT_REPOSITORY https://github.com/google/flatbuffers.git
    GIT_TAG 0100f6a5779831fa7a651e4b67ef389a8752bd9b # Release 23.5.26
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    Boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG 564e2ac16907019696cdaba8a93e3588ec596062 # Release 1.83.0
    GIT_PROGRESS TRUE
)

macro(jk_provide_dependecy method dep_name)
    if("${dep_name}" MATCHES "^(lexy|fmt|date|Catch2|cppzmq|ZeroMQ|flatbuffers|Boost)$")
        list(APPEND jk_provider_args ${method} ${dep_name})

        if ("${dep_name}" MATCHES "^cppzmq$")
            set (CPPZMQ_BUILD_TESTS OFF CACHE INTERNAL "Turn off tests")
        elseif ("${dep_name}" MATCHES "^ZeroMQ$")
            set (ZMQ_BUILD_TESTS OFF CACHE INTERNAL "Turn off tests")
            set (WITH_PERF_TOOL OFF CACHE INTERNAL "Disable unnecessary targets")
        elseif ("${dep_name}" MATCHES "^flatbuffers$")
            set (FLATBUFFERS_BUILD_TESTS OFF CACHE INTERNAL "Turn off tests")
        endif()

        FetchContent_MakeAvailable(${dep_name})

        list(POP_BACK jk_provider_args dep_name method)

        if ("${method}" STREQUAL "FIND_PACKAGE")
            set(${dep_name}_FOUND TRUE)
        endif()
    endif()
endmacro()

cmake_language(
    SET_DEPENDENCY_PROVIDER jk_provide_dependecy
    SUPPORTED_METHODS
        FIND_PACKAGE
        FETCHCONTENT_MAKEAVAILABLE_SERIAL
)

