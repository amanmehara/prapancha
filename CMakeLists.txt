cmake_minimum_required(VERSION 4.1)
project(prapancha LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-march=native)

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
        jsoncpp
        GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
        GIT_TAG 1.9.6
        OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
        drogon
        GIT_REPOSITORY https://github.com/drogonframework/drogon.git
        GIT_TAG v1.9.12
)
FetchContent_MakeAvailable(jsoncpp drogon)

if (TARGET jsoncpp_lib)
    add_library(Jsoncpp_lib ALIAS jsoncpp_lib)
elseif (TARGET jsoncpp_static)
    add_library(Jsoncpp_lib ALIAS jsoncpp_static)
endif ()

if (TARGET jsoncpp_lib)
    add_library(prapancha::json ALIAS jsoncpp_lib)
elseif (TARGET jsoncpp_static)
    add_library(prapancha::json ALIAS jsoncpp_static)
endif ()

if (TARGET drogon)
    add_library(prapancha::web ALIAS drogon)
endif ()

enable_testing()
add_subdirectory(src)
