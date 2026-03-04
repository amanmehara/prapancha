add_library(prapancha_security STATIC
        src/hasher.cpp
)

add_library(prapancha::security ALIAS prapancha_security)

target_include_directories(prapancha_security PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(prapancha_security PRIVATE prapancha::crypto)

add_dependencies(prapancha_security openssl_external)
