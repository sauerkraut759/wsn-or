cmake_minimum_required(VERSION 3.15)
project(wsn_optimizer)

# Python y Pybind11
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)

# .so
pybind11_add_module(wsn_core src/wsn_core.cpp)

# TODO: Linking para OR-Tools
