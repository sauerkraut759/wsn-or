cmake_minimum_required(VERSION 3.15)
project(wsn_optimizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Python y Pybind11
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)
find_package(ortools REQUIRED CONFIG)

# .so
pybind11_add_module(wsn_core src/wsn_core.cpp)

# ortools linking
target_link_libraries(wsn_core PRIVATE ortools::ortools)
