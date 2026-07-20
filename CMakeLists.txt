cmake_minimum_required(VERSION 3.12)

project(
    argparser
    VERSION 1.0
    DESCRIPTION "Command-line argument parsing library"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(lib)
add_subdirectory(bin)

enable_testing()
add_subdirectory(tests)
