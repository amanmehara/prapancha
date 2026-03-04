add_library(prapancha_logging INTERFACE)

add_library(prapancha::logging ALIAS prapancha_logging)

target_include_directories(prapancha_logging INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
