cmake_minimum_required(VERSION 3.0)

project (fecs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(fecs main.cpp)
add_executable(test test/main.cpp test/vector_store.cpp test/world.cpp)

set_property(TARGET fecs PROPERTY CXX_STANDARD 20)
set_property(TARGET test PROPERTY CXX_STANDARD 20)
target_compile_options(fecs PUBLIC -fconcepts-diagnostics-depth=20)
target_include_directories(fecs PUBLIC include/)
target_include_directories(test PUBLIC include/)

