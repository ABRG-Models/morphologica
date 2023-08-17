add_executable(app1 main.cpp)
target_link_libraries(app1 Qt5::Gui Qt5::Widgets Qt5::Core ${MORPH_LIBS_GL})
