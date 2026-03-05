add_library(prapancha_env INTERFACE)

add_library(prapancha::env ALIAS prapancha_env)

target_include_directories(prapancha_env INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
