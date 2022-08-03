cmake_minimum_required(VERSION 3.22)
project(Contextual)

set(CMAKE_CXX_STANDARD 17)

include_directories(BEFORE include)
add_executable(Contextual src/Main.cpp src/SymbolTable.cpp src/ContextManager.cpp src/ContextTable.cpp)
