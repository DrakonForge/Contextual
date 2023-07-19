cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(ContextualApp)

set(CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc -static")

set(EXECUTABLE_NAME context-app)
set(APP_SOURCES
        ${LIB_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/app/Main.cpp
        ${CMAKE_SOURCE_DIR}/src/app/ContextParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/QueryParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/TestParser.cpp)
set(APP_INCLUDE_DIR
        ${LIB_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/include/app)

add_executable(${EXECUTABLE_NAME} ${APP_SOURCES})
target_include_directories(${EXECUTABLE_NAME} PUBLIC ${APP_INCLUDE_DIR})
