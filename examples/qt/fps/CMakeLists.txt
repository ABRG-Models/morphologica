if(ARMADILLO_FOUND)
  add_executable(qtfps main.cpp mainwindow.cpp)
  target_link_libraries(qtfps Qt5::Gui Qt5::Widgets Qt5::Core ${MORPH_LIBS_CORE} ${MORPH_LIBS_GL})
  # Need multicontext for qt programs
  target_compile_definitions(qtfps PUBLIC GLAD_OPTION_MX=1)
endif(ARMADILLO_FOUND)
