set(LOGGING_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/async_sink.h
        ${CMAKE_CURRENT_SOURCE_DIR}/console_sink.h
        ${CMAKE_CURRENT_SOURCE_DIR}/file_sink.h
        ${CMAKE_CURRENT_SOURCE_DIR}/log_level.h
        ${CMAKE_CURRENT_SOURCE_DIR}/log_sink.h
        ${CMAKE_CURRENT_SOURCE_DIR}/logger.h
)

set(LOGGING_SOURCES ${LOGGING_SOURCES} PARENT_SCOPE)
