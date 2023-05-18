set(LIBRARY_NAME data_layer)

add_library(${LIBRARY_NAME})

target_sources(${LIBRARY_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/data_access.cpp
        ${CMAKE_CURRENT_LIST_DIR}/schema.cpp
        ${CMAKE_CURRENT_LIST_DIR}/error.cpp
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/data_access.h
        ${CMAKE_CURRENT_LIST_DIR}/schema.h
        ${CMAKE_CURRENT_LIST_DIR}/error.h
)

target_include_directories(${LIBRARY_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)