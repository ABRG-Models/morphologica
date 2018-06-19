add_library(morphologica SHARED display.cpp sockserve.cpp tools.cpp world.cpp BezCurve.cpp)

target_link_libraries(morphologica armadillo ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_X11_LIB})

install(TARGETS morphologica LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/morph)
install(
  FILES display.h sockserve.h tools.h world.h BezCurve.h BezCoord.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph
  )
