file(GLOB_RECURSE SANDBOX_CC src/*.cpp src/*.hpp )

add_executable(sandbox ${SANDBOX_CC})
add_dependencies(sandbox toyengine)
target_link_libraries(sandbox toyengine)

