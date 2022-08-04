cmake_minimum_required(VERSION 3.7)
project(Contextual)

set(CMAKE_CXX_STANDARD 17)

include("${CMAKE_SOURCE_DIR}/vendor/rapidjson.cmake")

set(EXECUTABLE_NAME context)
set(SOURCES
        src/Main.cpp
        src/StringTable.cpp
        src/ContextManager.cpp
        src/ContextTable.cpp
        src/RuleDatabase.cpp
        src/RuleTable.cpp
        src/RuleParser.cpp
        src/Token.cpp
        src/TokenBoolean.cpp
        src/TokenContext.cpp
        src/TokenString.cpp
        src/DatabaseQuery.cpp
        src/TokenFloat.cpp
        src/TokenFunction.cpp
        src/TokenGroup.cpp
        src/TokenInt.cpp
        src/TokenList.cpp
        src/TokenSymbol.cpp)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include ${RAPIDJSON_INCLUDE_DIR})

add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_include_directories(${EXECUTABLE_NAME} PUBLIC ${INCLUDE_DIR})
add_dependencies(${EXECUTABLE_NAME} rapidjson)