set(LIBRARY_NAME automation)
set(spdlog_DIR ../../external/spdlog/build/)

add_library(${LIBRARY_NAME})
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

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

target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        spdlog::spdlog
        fmt)