# GLFW

find_package( OpenGL REQUIRED )

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL  "" FORCE)

add_subdirectory(vendor/GLFW)

# ToyEngine

file(GLOB_RECURSE TOYENGINE_CC src/*.cpp src/*.hpp )
add_library(toyengine STATIC ${TOYENGINE_CC})
target_link_libraries(toyengine PUBLIC OpenGL::GL glfw)

target_include_directories(toyengine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_precompile_headers(toyengine PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/tepch.hpp)
