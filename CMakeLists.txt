# build the main.cpp
########################################################################################################################
### SIMPLEHTTPSERVER LIBRARY MAIN
########################################################################################################################

cmake_minimum_required(VERSION 3.2)

########################################################################################################################
### PROJECT DESCRIPTION AND META INFO
########################################################################################################################

# set(SPACE_TRADERS_PROJECT_NAME        "SPACE_TRADERS")
# set(SPACE_TRADERS_PROJECT_DESCRIPTION "A simple http server written in C++ based on Boost.Beast")
# set(SPACE_TRADERS_AUTHOR_DOMAIN       "https://github.com/mostsignificant/SPACE_TRADERS")
# set(SPACE_TRADERS_AUTHOR_MAINTAINER   "mostsig@gmail.com")
# set(SPACE_TRADERS_VERSION_MAJOR       "0")
# set(SPACE_TRADERS_VERSION_MINOR       "0")
# set(SPACE_TRADERS_VERSION_PATCH       "1")

# set(SIMPLEHTTPSERVER_VERSION
#     "${SIMPLEHTTPSERVER_VERSION_MAJOR}.${SIMPLEHTTPSERVER_VERSION_MINOR}.${SIMPLEHTTPSERVER_VERSION_PATCH}")
# set(SIMPLEHTTPSERVER_NAME_VERSION
#     "${SIMPLEHTTPSERVER_PROJECT_NAME} v${SIMPLEHTTPSERVER_VERSION} (${SIMPLEHTTPSERVER_VERSION_REVISION})")

########################################################################################################################
### DECLARE PROJECT
########################################################################################################################

project(space-traders)

set(SPACE_TRADERS_SRC_DIR space-traders/src)

add_subdirectory(src)

# https://stackoverflow.com/questions/13703647/how-to-properly-add-include-directories-with-cmake
# somewhere below
# # enable testing and define tests
# enable_testing()
# add_subdirectory(tests)