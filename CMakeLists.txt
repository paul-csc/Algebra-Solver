cmake_minimum_required(VERSION 3.20)
project(Algebra-Solver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/main.cpp
    src/parser.cpp
    src/solver.cpp
    src/tokenizer.cpp
    src/utils.cpp

    src/parser.h
    src/solver.h
    src/tokenizer.h
    src/utils.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Match VS filters to directory structure on disk
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})