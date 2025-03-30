file(GLOB_RECURSE SANDBOX_CC src/*.cpp )

add_executable(sandbox ${SANDBOX_CC})
add_dependencies(sandbox toyengine)
target_link_libraries(sandbox toyengine)

# shaders

find_package(Vulkan COMPONENTS glslangValidator REQUIRED)

file(GLOB_RECURSE GLSL_SOURCE_FILES CONFIGURE_DEPENDS "src/shaders/*")

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
    COMMAND glslangValidator -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL}
  )
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)
  
add_custom_target(shaders DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(sandbox shaders)

add_custom_command(TARGET sandbox POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:sandbox>/shaders/"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_BINARY_DIR}/shaders"
    "$<TARGET_FILE_DIR:sandbox>/shaders"
  COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/assets/" "$<TARGET_FILE_DIR:sandbox>/assets"
)