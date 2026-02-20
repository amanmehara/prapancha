add_subdirectory(controller)
add_subdirectory(policy)

add_executable(${PROJECT_NAME}
        codec.h
        configuration.h
        configuration.cpp
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

target_link_libraries(${PROJECT_NAME} PRIVATE
        prapancha::json
        prapancha::web
)
