cmake_minimum_required(VERSION 3.10)
project(Swiq)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(swiq
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/interpreter.cpp
    src/value.cpp
)

target_compile_definitions(swiq PRIVATE SWIQ_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(swiq PRIVATE src)
