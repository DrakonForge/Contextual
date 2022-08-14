cmake_minimum_required(VERSION 3.14)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(TEST_EXECUTABLE context-unit-tests)
set(TEST_SOURCES
        ${CMAKE_SOURCE_DIR}/src/StringTable.cpp
        ${CMAKE_SOURCE_DIR}/src/ContextManager.cpp
        ${CMAKE_SOURCE_DIR}/src/ContextTable.cpp
        ${CMAKE_SOURCE_DIR}/src/RuleDatabase.cpp
        ${CMAKE_SOURCE_DIR}/src/RuleTable.cpp
        ${CMAKE_SOURCE_DIR}/src/DatabaseQuery.cpp
        ${CMAKE_SOURCE_DIR}/src/json/DatabaseParser.cpp
        ${CMAKE_SOURCE_DIR}/src/json/SymbolParser.cpp
        ${CMAKE_SOURCE_DIR}/src/json/RuleParser.cpp
        ${CMAKE_SOURCE_DIR}/src/json/JsonUtils.cpp
        ${CMAKE_SOURCE_DIR}/src/token/TokenBoolean.cpp
        ${CMAKE_SOURCE_DIR}/src/token/TokenContext.cpp
        ${CMAKE_SOURCE_DIR}/src/token/TokenString.cpp
        ${CMAKE_SOURCE_DIR}/src/token/TokenFloat.cpp
        ${CMAKE_SOURCE_DIR}/src/token/TokenFunction.cpp
        ${CMAKE_SOURCE_DIR}/src/token/TokenInt.cpp
        ${CMAKE_SOURCE_DIR}/src/token/TokenList.cpp
        ${CMAKE_SOURCE_DIR}/src/criterion/CriterionStatic.cpp
        ${CMAKE_SOURCE_DIR}/src/criterion/CriterionInvertible.cpp
        ${CMAKE_SOURCE_DIR}/src/criterion/CriterionFloatComparable.cpp
        ${CMAKE_SOURCE_DIR}/src/criterion/CriterionListComparable.cpp
        ${CMAKE_SOURCE_DIR}/src/criterion/CriterionExist.cpp
        ${CMAKE_SOURCE_DIR}/src/criterion/CriterionFail.cpp
        ${CMAKE_SOURCE_DIR}/src/criterion/Criterion.cpp
        ${CMAKE_SOURCE_DIR}/src/criterion/CriterionIncludes.cpp
        ${CMAKE_SOURCE_DIR}/src/criterion/CriterionEmpty.cpp
        ${CMAKE_SOURCE_DIR}/src/criterion/CriterionAlternate.cpp
        ${CMAKE_SOURCE_DIR}/src/criterion/CriterionDynamic.cpp
        ${CMAKE_SOURCE_DIR}/src/response/ResponseSimple.cpp
        ${CMAKE_SOURCE_DIR}/src/response/ResponseMultiple.cpp
        ${CMAKE_SOURCE_DIR}/src/response/ResponseSpeech.cpp
        ${CMAKE_SOURCE_DIR}/src/response/ResponseEvent.cpp
        ${CMAKE_SOURCE_DIR}/src/speech/SpeechTokenizer.cpp
        ${CMAKE_SOURCE_DIR}/src/speech/SpeechGenerator.cpp
        ${CMAKE_SOURCE_DIR}/src/function/FunctionTable.cpp
        ${CMAKE_SOURCE_DIR}/src/function/DefaultFunctionTable.cpp
        Main.cpp
        TestContextTable.cpp
        TestSpeechTokenizer.cpp
        TestSpeechGenerator.cpp)

add_executable(${TEST_EXECUTABLE} ${TEST_SOURCES})
target_include_directories(${TEST_EXECUTABLE} PUBLIC ${INCLUDE_DIR})
target_link_libraries(${TEST_EXECUTABLE} gtest_main)
add_test(NAME unit_tests COMMAND ${TEST_EXECUTABLE})
