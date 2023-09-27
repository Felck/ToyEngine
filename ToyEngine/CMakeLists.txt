file(GLOB_RECURSE TOYENGINE_CC src/*.cpp src/*.hpp )

add_library(toyengine STATIC ${TOYENGINE_CC})

target_include_directories(toyengine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)

target_precompile_headers(toyengine PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/tepch.hpp)