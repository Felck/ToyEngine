# ToyEngine

file(GLOB_RECURSE TE_SOURCE src/*.cpp )
add_library(toyengine STATIC ${TE_SOURCE})

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

# ImGui

set(IMGUI_DIR "vendor/ImGui")
file(GLOB IMGUI_SOURCE  ${IMGUI_DIR}/*.cpp)
add_library(imgui STATIC ${IMGUI_SOURCE} ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC ${IMGUI_DIR})
target_link_libraries(imgui glfw)

# Linking

target_link_libraries(toyengine PUBLIC glfw glad imgui)
