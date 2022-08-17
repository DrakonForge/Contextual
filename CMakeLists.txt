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
        GIT_TAG        f54b0e47a08782a6131cc3d60f94d038fa6e0a51 # v1.1.0
        )
FetchContent_Populate(rapidjson)

# plog
FetchContent_Declare(plog
        GIT_REPOSITORY https://github.com/SergiusTheBest/plog.git
        GIT_TAG 89ac49396ae6978a056034d1e34bb170bfd3de33 # 1.1.8
        )
FetchContent_Populate(plog)

set(EXECUTABLE_NAME context)
set(LIB_SOURCES
        ${CMAKE_SOURCE_DIR}/src/MathUtils.cpp
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
        ${CMAKE_SOURCE_DIR}/src/json/CriteriaParser.cpp
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
        ${CMAKE_SOURCE_DIR}/src/response/ResponseContext.cpp
        ${CMAKE_SOURCE_DIR}/src/response/ResponseContextSetStatic.cpp
        ${CMAKE_SOURCE_DIR}/src/response/ResponseContextSetDynamic.cpp
        ${CMAKE_SOURCE_DIR}/src/response/ResponseContextAdd.cpp
        ${CMAKE_SOURCE_DIR}/src/response/ResponseContextMultiply.cpp
        ${CMAKE_SOURCE_DIR}/src/response/ResponseContextSetList.cpp
        ${CMAKE_SOURCE_DIR}/src/response/ResponseContextInvert.cpp
        ${CMAKE_SOURCE_DIR}/src/speech/SpeechTokenizer.cpp
        ${CMAKE_SOURCE_DIR}/src/speech/SpeechGenerator.cpp
        ${CMAKE_SOURCE_DIR}/src/function/FunctionTable.cpp
        ${CMAKE_SOURCE_DIR}/src/function/DefaultFunctionTable.cpp)
set(LIB_INCLUDE_DIR
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/json
        ${CMAKE_SOURCE_DIR}/include/criterion
        ${CMAKE_SOURCE_DIR}/include/token
        ${CMAKE_SOURCE_DIR}/include/speech
        ${CMAKE_SOURCE_DIR}/include/response
        ${CMAKE_SOURCE_DIR}/include/function
        ${rapidjson_SOURCE_DIR}/include
        ${plog_SOURCE_DIR}/include)

add_executable(${EXECUTABLE_NAME} src/Main.cpp ${LIB_SOURCES})
target_include_directories(${EXECUTABLE_NAME} PUBLIC ${LIB_INCLUDE_DIR})

include(GoogleTest)
add_subdirectory(tests)
add_subdirectory(src/app)
