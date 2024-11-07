# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

add_executable(firstcomponents first_example.cpp)
target_link_libraries(firstcomponents OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(firstcomponents GLEW::GLEW)
endif()