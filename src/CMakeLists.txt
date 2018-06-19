add_library(morphologica SHARED display.cpp sockserve.cpp tools.cpp world.cpp BezCurve.cpp)
install(TARGETS morphologica LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/morph)
install(
  FILES display.h sockserve.h tools.h world.h BezCurve.h BezCoord.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph
  )
