#
# OpenGL 3.1 ES examples
#

# Ensure we can #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

if(ARMADILLO_FOUND)
  add_executable(convolve_pi convolve.cpp)
  target_link_libraries(convolve_pi OpenGL::EGL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(convolve_pi GLEW::GLEW)
  endif()

  add_executable(hexgrid_pi hexgrid.cpp)
  target_link_libraries(hexgrid_pi OpenGL::EGL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(hexgrid_pi GLEW::GLEW)
  endif()
endif(ARMADILLO_FOUND)

add_executable(graph1_pi graph1.cpp)
target_link_libraries(graph1_pi OpenGL::EGL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph1_pi GLEW::GLEW)
endif()
