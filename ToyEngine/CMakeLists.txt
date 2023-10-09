# ToyEngine

file(GLOB_RECURSE TOYENGINE_CC src/*.cpp src/*.hpp )
add_library(toyengine STATIC ${TOYENGINE_CC})

target_include_directories(toyengine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_precompile_headers(toyengine PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/tepch.hpp)


# GLFW

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL  "" FORCE)

target_compile_definitions(toyengine PRIVATE GLFW_INCLUDE_NONE)

add_subdirectory(vendor/GLFW)

# glad

add_subdirectory(vendor/glad)

# Linking

target_link_libraries(toyengine PUBLIC glfw glad)
