add_executable(${PROJECT_NAME}
        codec.h
        controller.cpp
        controller.h
        main.cpp
        model.h
        persistence.h
        prapancha.cpp
        prapancha.h
        router.cpp
        router.h
        uuid.cpp
        uuid.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        prapancha::json
        prapancha::web
)
