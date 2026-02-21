cmake_minimum_required(VERSION 4.1)
project(prapancha LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-march=native)

include(ExternalProject)
include(FetchContent)

set(OPENSSL_INSTALL_DIR "${PROJECT_BINARY_DIR}/external/openssl/install")

ExternalProject_Add(openssl_external
        GIT_REPOSITORY https://github.com/openssl/openssl.git
        GIT_TAG openssl-3.6.1
        PREFIX ${PROJECT_BINARY_DIR}/external/openssl
        CONFIGURE_COMMAND <SOURCE_DIR>/config --prefix=${OPENSSL_INSTALL_DIR} --openssldir=${OPENSSL_INSTALL_DIR} no-shared
        BUILD_COMMAND make
        INSTALL_COMMAND make install_sw
        UPDATE_COMMAND ""
        BUILD_BYPRODUCTS ${OPENSSL_INSTALL_DIR}/lib/libcrypto.a ${OPENSSL_INSTALL_DIR}/lib/libssl.a
)

file(MAKE_DIRECTORY ${OPENSSL_INSTALL_DIR}/include)

add_library(OpenSSL::Crypto STATIC IMPORTED GLOBAL)
set_target_properties(OpenSSL::Crypto PROPERTIES
        IMPORTED_LOCATION "${OPENSSL_INSTALL_DIR}/lib/libcrypto.a"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INSTALL_DIR}/include"
)
add_dependencies(OpenSSL::Crypto openssl_external)
add_library(OpenSSL::SSL STATIC IMPORTED GLOBAL)
set_target_properties(OpenSSL::SSL PROPERTIES
        IMPORTED_LOCATION "${OPENSSL_INSTALL_DIR}/lib/libssl.a"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INSTALL_DIR}/include"
)
add_dependencies(OpenSSL::SSL openssl_external)

set(OPENSSL_ROOT_DIR "${OPENSSL_INSTALL_DIR}" CACHE PATH "Force 3.6.1" FORCE)
set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_INSTALL_DIR}/lib/libcrypto.a" CACHE FILEPATH "" FORCE)
set(OPENSSL_SSL_LIBRARY "${OPENSSL_INSTALL_DIR}/lib/libssl.a" CACHE FILEPATH "" FORCE)

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

if (TARGET OpenSSL::Crypto)
    add_library(prapancha::crypto ALIAS OpenSSL::Crypto)
endif ()

if (TARGET OpenSSL::SSL)
    add_library(prapancha::ssl ALIAS OpenSSL::SSL)
endif ()

enable_testing()
add_subdirectory(src)
