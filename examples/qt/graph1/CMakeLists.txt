add_executable(qtgraph1 main.cpp)
target_link_libraries(qtgraph1 Qt5::Gui Qt5::Widgets Qt5::Core ${MORPH_LIBS_GL})
# Need multicontext for qt programs
target_compile_definitions(qtgraph1 PUBLIC GLAD_OPTION_MX=1)
