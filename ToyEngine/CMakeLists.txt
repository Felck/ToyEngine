file(GLOB_RECURSE TOYENGINE_CC src/*.cpp src/*.hpp )

add_library(toyengine STATIC ${TOYENGINE_CC})

target_include_directories(toyengine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)

set(toyengine_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set_property(TARGET toyengine APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${toyengine_INCLUDE_DIR})