set(TARGET space-traders)
set(spdlog_DIR ../external/spdlog/build/)

find_package(cpprestsdk REQUIRED)
find_package(spdlog REQUIRED)

add_executable(${TARGET}
    main.cpp)

target_compile_features(${TARGET} PUBLIC
    cxx_std_14)

add_subdirectory(automation)
add_subdirectory(data_layer)

target_include_directories(${TARGET} PUBLIC
    cpprestsdk)

target_link_libraries(${TARGET} PUBLIC
    data_layer
    automation
    cpprest
    spdlog::spdlog
    ssl
    crypto)



