add_executable(qtgraph1 main.cpp)
target_link_libraries(qtgraph1 Qt5::Gui Qt5::Widgets Qt5::Core ${MORPH_LIBS_GL})

add_executable(qtgraph1_mx main_mx.cpp)
target_link_libraries(qtgraph1_mx Qt5::Gui Qt5::Widgets Qt5::Core ${MORPH_LIBS_GL})
# Need multicontext for qt programs
target_compile_definitions(qtgraph1_mx PUBLIC USE_GLAD_MX=1)
