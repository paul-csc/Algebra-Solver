cmake_minimum_required(VERSION 3.20)
project(Algebra-Solver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

file(GLOB_RECURSE PROJECT_SOURCES
     CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/src/*.cpp"
     "${CMAKE_SOURCE_DIR}/src/*.hpp"
     "${CMAKE_SOURCE_DIR}/src/*.h"
)

add_executable(Algebra-Solver ${PROJECT_SOURCES})

target_include_directories(Algebra-Solver PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_compile_options(Algebra-Solver PRIVATE    
    /EHsc
    /Zc:preprocessor
    /Zc:__cplusplus
    $<$<CONFIG:Debug>:/Od /Zi /W4>
    $<$<CONFIG:Release>:/O2 /GL /W3>
)

target_link_options(Algebra-Solver PRIVATE
    $<$<CONFIG:Release>:/LTCG>
)