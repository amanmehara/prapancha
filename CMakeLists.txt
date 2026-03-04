cmake_minimum_required(VERSION 4.1)

set(CMAKE_CXX_STANDARD 26)

project(prapancha LANGUAGES CXX)

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

FetchContent_Declare(
        boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.90.0/boost-1.90.0-cmake.tar.xz
        URL_HASH SHA256=aca59f889f0f32028ad88ba6764582b63c916ce5f77b31289ad19421a96c555f
)
FetchContent_MakeAvailable(boost)

if (TARGET OpenSSL::Crypto)
    add_library(prapancha::crypto ALIAS OpenSSL::Crypto)
endif ()

if (TARGET OpenSSL::SSL)
    add_library(prapancha::ssl ALIAS OpenSSL::SSL)
endif ()

add_subdirectory(apps)
add_subdirectory(libs)
