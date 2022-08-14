cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(Contextual)

set(CMAKE_CXX_STANDARD 17)

# Based on: https://github.com/owensgroup/RXMesh/blob/main/CMakeLists.txt
include(FetchContent)

# GoogleTest
FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850 # 1.12.1
        )
FetchContent_MakeAvailable(googletest)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# rapidjson
FetchContent_Declare(rapidjson
        GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
        GIT_TAG        f54b0e47a08782a6131cc3d60f94d038fa6e0a51
        )
FetchContent_Populate(rapidjson)

set(EXECUTABLE_NAME context)
set(SOURCES
        src/Main.cpp
        src/StringTable.cpp
        src/ContextManager.cpp
        src/ContextTable.cpp
        src/RuleDatabase.cpp
        src/RuleTable.cpp
        src/DatabaseQuery.cpp
        src/json/DatabaseParser.cpp
        src/json/SymbolParser.cpp
        src/json/RuleParser.cpp
        src/json/JsonUtils.cpp
        src/token/TokenBoolean.cpp
        src/token/TokenContext.cpp
        src/token/TokenString.cpp
        src/token/TokenFloat.cpp
        src/token/TokenFunction.cpp
        src/token/TokenInt.cpp
        src/token/TokenList.cpp
        src/criterion/CriterionStatic.cpp
        src/criterion/CriterionInvertible.cpp
        src/criterion/CriterionFloatComparable.cpp
        src/criterion/CriterionListComparable.cpp
        src/criterion/CriterionExist.cpp
        src/criterion/CriterionFail.cpp
        src/criterion/Criterion.cpp
        src/criterion/CriterionIncludes.cpp
        src/criterion/CriterionEmpty.cpp
        src/criterion/CriterionAlternate.cpp
        src/criterion/CriterionDynamic.cpp
        src/response/ResponseSimple.cpp
        src/response/ResponseMultiple.cpp
        src/response/ResponseSpeech.cpp
        src/response/ResponseEvent.cpp
        src/speech/SpeechTokenizer.cpp
        src/speech/SpeechGenerator.cpp
        src/function/FunctionTable.cpp
        src/function/DefaultFunctionTable.cpp)
set(INCLUDE_DIR
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/json
        ${CMAKE_SOURCE_DIR}/include/criterion
        ${CMAKE_SOURCE_DIR}/include/token
        ${CMAKE_SOURCE_DIR}/include/speech
        ${CMAKE_SOURCE_DIR}/include/response
        ${CMAKE_SOURCE_DIR}/include/function
        ${rapidjson_SOURCE_DIR}/include)

add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_include_directories(${EXECUTABLE_NAME} PUBLIC ${INCLUDE_DIR})

include(GoogleTest)
add_subdirectory(tests)