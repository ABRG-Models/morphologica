#
# OpenGL 3.1 ES examples
#
# Make MORPH_LIBS_GLES varable
set(MORPH_LIBS_GLES OpenGL::EGL glfw Freetype::Freetype)
if (HAVE_LODEPNG)
  set(MORPH_LIBS_GLES ${MORPH_LIBS_GLES} lodepng)
endif()

# Ensure we can #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

if(ARMADILLO_FOUND)
  add_executable(convolve_pi convolve.cpp)
  target_link_libraries(convolve_pi ${MORPH_LIBS_GLES})
  if(USE_GLEW)
    target_link_libraries(convolve_pi GLEW::GLEW)
  endif()

  add_executable(hexgrid_pi hexgrid.cpp)
  target_link_libraries(hexgrid_pi ${MORPH_LIBS_GLES})
  if(USE_GLEW)
    target_link_libraries(hexgrid_pi GLEW::GLEW)
  endif()
endif(ARMADILLO_FOUND)

add_executable(graph1_pi graph1.cpp)
target_link_libraries(graph1_pi ${MORPH_LIBS_GLES})
if(USE_GLEW)
  target_link_libraries(graph1_pi GLEW::GLEW)
endif()
