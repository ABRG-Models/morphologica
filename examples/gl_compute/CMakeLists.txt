include_directories(BEFORE ${PROJECT_SOURCE_DIR})

add_executable(shadercompute shadercompute.cpp)
target_link_libraries(shadercompute OpenGL::GL glfw)

if (OpenGL_EGL_FOUND)
  add_executable(shader_ssbo shader_ssbo.cpp)
  target_link_libraries(shader_ssbo OpenGL::EGL glfw)
endif (OpenGL_EGL_FOUND)
