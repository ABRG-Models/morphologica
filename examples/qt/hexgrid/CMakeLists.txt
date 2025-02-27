if(ARMADILLO_FOUND)
  add_executable(qthexgrid main.cpp mainwindow.cpp)
  target_link_libraries(qthexgrid Qt5::Gui Qt5::Widgets Qt5::Core ${MORPH_LIBS_CORE} ${MORPH_LIBS_GL})
  # Need multicontext for qt programs
  target_compile_definitions(qthexgrid PUBLIC GLAD_OPTION_MX=1)
endif(ARMADILLO_FOUND)
