# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

if(ARMADILLO_FOUND)
  add_executable(sg sg.cpp)
  target_compile_definitions(sg PUBLIC FLT=float)
  if(APPLE AND OpenMP_CXX_FOUND)
    target_link_libraries(sg OpenMP::OpenMP_CXX OpenGL::GL glfw Freetype::Freetype)
  else()
    target_link_libraries(sg OpenGL::GL glfw Freetype::Freetype)
  endif()
endif()
