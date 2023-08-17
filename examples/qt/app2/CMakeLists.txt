add_executable(app2 main.cpp mainwindow.cpp)
target_link_libraries(app2 Qt5::Gui Qt5::Widgets Qt5::Core ${MORPH_LIBS_CORE} ${MORPH_LIBS_GL})
