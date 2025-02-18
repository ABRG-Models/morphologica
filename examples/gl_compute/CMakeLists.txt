include_directories(BEFORE ${PROJECT_SOURCE_DIR})

# Note: Using a C++-20 construct in these programs
add_executable(shadercompute shadercompute.cpp)
#set_target_properties(shadercompute PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
target_link_libraries(shadercompute OpenGL::GL glfw)

if (OpenGL_EGL_FOUND)
  add_executable(shader_ssbo shader_ssbo.cpp)
#  set_target_properties(shader_ssbo PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
  target_link_libraries(shader_ssbo OpenGL::EGL glfw)

  add_executable(shader_naive_scan shader_naive_scan.cpp)
#  set_target_properties(shader_naive_scan PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
  target_link_libraries(shader_naive_scan OpenGL::EGL glfw)

  add_executable(shader_naive_scan_cli shader_naive_scan_cli.cpp)
#  set_target_properties(shader_naive_scan_cli PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
  target_link_libraries(shader_naive_scan_cli OpenGL::EGL gbm)

  add_executable(seq_naive_scan naive_scan.cpp)

endif (OpenGL_EGL_FOUND)
