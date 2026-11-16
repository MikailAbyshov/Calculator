cmake_minimum_required(VERSION 3.16)
project(Calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CALC_INCLUDE_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PLUGINS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/plugins")

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(calc
        src/main.cpp
        src/PluginManager.cpp
        src/ExpressionParser.cpp
        src/Logger.cpp
)

target_include_directories(calc PRIVATE "${CALC_INCLUDE_DIR}")

if(NOT WIN32)
    target_link_libraries(calc PRIVATE ${CMAKE_DL_LIBS})
endif()

add_subdirectory(plugins)