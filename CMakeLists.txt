cmake_minimum_required(VERSION 3.27)
project(toyengine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ToyEngine

add_subdirectory("ToyEngine")

# Sanbox

add_subdirectory("Sandbox")