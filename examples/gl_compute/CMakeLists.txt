include_directories(BEFORE ${PROJECT_SOURCE_DIR})

# Note: Using a C++-20 construct in these programs
add_executable(shadercompute shadercompute.cpp)
# As we're not loading GL with glad2, we have to set these defines
target_compile_definitions (shadercompute PUBLIC GL3_PROTOTYPES GL_GLEXT_PROTOTYPES)
target_link_libraries(shadercompute OpenGL::GL glfw)

if (OpenGL_EGL_FOUND)
  add_executable(shader_ssbo shader_ssbo.cpp)
  target_link_libraries(shader_ssbo OpenGL::EGL glfw)

  add_executable(shader_naive_scan shader_naive_scan.cpp)
  target_link_libraries(shader_naive_scan OpenGL::EGL glfw)

  add_executable(shader_naive_scan_cli shader_naive_scan_cli.cpp)
  target_link_libraries(shader_naive_scan_cli OpenGL::EGL gbm)

  add_executable(seq_naive_scan naive_scan.cpp)
endif (OpenGL_EGL_FOUND)
