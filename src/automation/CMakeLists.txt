set(LIBRARY_NAME automation)

add_library(${LIBRARY_NAME})

target_sources(${LIBRARY_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/ship_auto.cpp
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/ship_auto.h
)

target_include_directories(${LIBRARY_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)