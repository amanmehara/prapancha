add_subdirectory(controller)
add_subdirectory(logging)
add_subdirectory(policy)
add_subdirectory(security)

add_executable(${PROJECT_NAME}
        codec.h
        configuration.h
        configuration.cpp
        logger_registry.h
        main.cpp
        model.h
        persistence.h
        prapancha.cpp
        prapancha.h
        router.cpp
        router.h
        uuid.cpp
        uuid.h
        ${CONTROLLER_SOURCES}
        ${POLICY_SOURCES}
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
        prapancha::logging
        prapancha::security
)

add_dependencies(${PROJECT_NAME} openssl_external)