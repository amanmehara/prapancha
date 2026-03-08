add_executable(${PROJECT_NAME}
        src/beast_adapter.cpp
        src/configuration.cpp
        src/main.cpp
        src/prapancha.cpp
        src/uuid.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${OPENSSL_INSTALL_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        prapancha::crypto
        Boost::asio
        Boost::beast
        Boost::headers
        Boost::json
        Boost::system
        prapancha::env
        prapancha::logging
        prapancha::security
)

add_dependencies(${PROJECT_NAME} openssl_external)