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
        src/json/DatabaseParser.cpp
        src/json/TokenParser.cpp
        src/json/RuleParser.cpp
        src/json/JsonUtils.cpp
        src/token/Token.cpp
        src/token/TokenBoolean.cpp
        src/token/TokenContext.cpp
        src/token/TokenString.cpp
        src/DatabaseQuery.cpp
        src/token/TokenFloat.cpp
        src/token/TokenFunction.cpp
        src/token/TokenGroup.cpp
        src/token/TokenInt.cpp
        src/token/TokenList.cpp
        src/criterion/CriterionStatic.cpp
        src/criterion/CriterionInvertible.cpp
        src/criterion/CriterionFloatComparable.cpp
        src/criterion/CriterionExist.cpp)
set(INCLUDE_DIR
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/json
        ${CMAKE_SOURCE_DIR}/include/criterion
        ${CMAKE_SOURCE_DIR}/include/token
        ${RAPIDJSON_INCLUDE_DIR})

add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_include_directories(${EXECUTABLE_NAME} PUBLIC ${INCLUDE_DIR})
add_dependencies(${EXECUTABLE_NAME} rapidjson)