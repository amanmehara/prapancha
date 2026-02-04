add_executable(${PROJECT_NAME}
        main.cpp
        prapancha.cpp
        prapancha.h
        router.cpp
        router.h
        controller.cpp
        controller.h
        model.h
        persistence.h
        uuid.cpp
        uuid.h)

target_link_libraries(${PROJECT_NAME} PRIVATE
        prapancha::json
        prapancha::web
)
