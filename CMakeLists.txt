cmake_minimum_required(VERSION 3.0.0)
project(gmtkGamejam2022)

set(APP_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(EXTERNALS_DIR "${PROJECT_SOURCE_DIR}/extern")
set(RAYLIB_DIR "${EXTERNALS_DIR}/raylib/")

set(CMAKE_CXX_STANDARD 17)

add_executable(
    gmtkGamejam2022
    ${APP_SOURCE_DIR}/pch.h
    ${APP_SOURCE_DIR}/main.cpp
    ${APP_SOURCE_DIR}/global.h
    ${APP_SOURCE_DIR}/Timer/Timer.h
    ${APP_SOURCE_DIR}/Timer/Timer.cpp
)

target_include_directories(
    gmtkGamejam2022
    PRIVATE ${EXTERNALS_DIR}
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
add_subdirectory("${RAYLIB_DIR}")

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
