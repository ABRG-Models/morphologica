include_directories(BEFORE ${PROJECT_SOURCE_DIR})

add_executable(shadercompute shadercompute.cpp)
target_link_libraries(shadercompute OpenGL::GL glfw)

if (OpenGL_EGL_FOUND)
  add_executable(shader_ssbo shader_ssbo.cpp)
  target_link_libraries(shader_ssbo OpenGL::EGL glfw)

  add_executable(shader_naive_scan shader_naive_scan.cpp)
  target_link_libraries(shader_naive_scan OpenGL::EGL glfw)

  add_executable(seq_naive_scan naive_scan.cpp)

endif (OpenGL_EGL_FOUND)
