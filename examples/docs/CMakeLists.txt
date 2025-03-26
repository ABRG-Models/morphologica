if(HAVE_STD_FORMAT)
  add_executable(dgraph_line_simple graph_line.cpp)
  target_compile_definitions(dgraph_line_simple PUBLIC GRAPH_LINE_SIMPLE)
  target_link_libraries(dgraph_line_simple OpenGL::GL glfw Freetype::Freetype)

  add_executable(dgraph_line_full graph_line.cpp)
  target_link_libraries(dgraph_line_full OpenGL::GL glfw Freetype::Freetype)
endif()
