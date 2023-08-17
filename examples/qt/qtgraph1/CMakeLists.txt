add_executable(qtgraph1 main.cpp)
target_link_libraries(qtgraph1 Qt5::Gui Qt5::Widgets Qt5::Core ${MORPH_LIBS_GL})
