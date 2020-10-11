cmake_minimum_required(VERSION 3.0)

project (fecs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(fecs main.cpp)

set_property(TARGET fecs PROPERTY CXX_STANDARD 20)
# target_compile_options(fecs PUBLIC -fconcepts-diagnostics-depth=20)
target_include_directories(fecs PUBLIC include/)

