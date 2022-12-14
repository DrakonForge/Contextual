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
        ${LIB_SOURCES}
        Main.cpp
        TestContextTable.cpp
        TestSpeechTokenizer.cpp
        TestSpeechGenerator.cpp)

add_executable(${TEST_EXECUTABLE} ${TEST_SOURCES})
target_include_directories(${TEST_EXECUTABLE} PUBLIC ${LIB_INCLUDE_DIR})
target_link_libraries(${TEST_EXECUTABLE} gtest_main)
add_test(NAME unit_tests COMMAND ${TEST_EXECUTABLE})
