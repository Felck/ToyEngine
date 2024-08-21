# ToyEngine

file(GLOB_RECURSE TE_SOURCE src/*.cpp )
add_library(toyengine STATIC ${TE_SOURCE})

target_include_directories(toyengine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_precompile_headers(toyengine PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/tepch.hpp)


# vulkan

find_package(Vulkan REQUIRED)
target_compile_definitions(toyengine PRIVATE
    VULKAN_HPP_NO_CONSTRUCTORS
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    $<$<CONFIG:Debug>:ENABLE_VALIDATION_LAYERS>
)
target_link_libraries(toyengine Vulkan::Vulkan)

# vma
set_source_files_properties(
  "src/vendor/vma.cpp"
  PROPERTIES
  COMPILE_FLAGS "-w"
)
add_subdirectory(vendor/vma)
target_link_libraries(toyengine GPUOpen::VulkanMemoryAllocator)

# GLFW

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL  "" FORCE)
set(GLFW_VULKAN_STATIC ON CACHE BOOL  "" FORCE)

target_compile_definitions(toyengine PRIVATE GLFW_INCLUDE_VULKAN)

add_subdirectory(vendor/GLFW)
target_compile_options(glfw PRIVATE -w) # disable warnings for glfw build
target_link_libraries(toyengine glfw)


# ImGui

set(IMGUI_DIR "vendor/ImGui")
file(GLOB IMGUI_SOURCE  ${IMGUI_DIR}/*.cpp)
add_library(imgui STATIC ${IMGUI_SOURCE} ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp)
target_include_directories(imgui PUBLIC ${IMGUI_DIR})
target_link_libraries(imgui glfw Vulkan::Vulkan)
target_link_libraries(toyengine imgui)

# glm
add_subdirectory(vendor/glm)
target_compile_definitions(toyengine PRIVATE
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_LEFT_HANDED
)
target_link_libraries(toyengine glm)
