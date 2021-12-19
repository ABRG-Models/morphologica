# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

add_executable(sg sg.cpp)
target_compile_definitions(sg PUBLIC FLT=float)
if(APPLE AND OpenMP_CXX_FOUND)
  target_link_libraries(sg OpenMP::OpenMP_CXX ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype)
else()
  target_link_libraries(sg ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype)
endif()
if(USE_GLEW)
  target_link_libraries (sg GLEW::GLEW)
endif()
