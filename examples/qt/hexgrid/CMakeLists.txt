add_executable(qthexgrid main.cpp mainwindow.cpp)
target_link_libraries(qthexgrid Qt5::Gui Qt5::Widgets Qt5::Core ${MORPH_LIBS_CORE} ${MORPH_LIBS_GL})
