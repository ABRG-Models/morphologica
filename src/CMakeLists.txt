# Define which cpp files will be compiled
set(morphlibsrc display.cpp sockserve.cpp tools.cpp world.cpp BezCurve.cpp ReadCurves.cpp)

if(APPLE)
  link_directories(/usr/X11R6/lib)
endif(APPLE)

# Shared library
add_library(morphologica SHARED ${morphlibsrc})
if(APPLE)
  target_link_libraries(
    morphologica
    armadillo GL ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_X11_LIB}
    )
else()
  target_link_libraries(
    morphologica
    armadillo ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_X11_LIB}
    )
endif(APPLE)
install(TARGETS morphologica
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  )

# Static library
add_library(morphstatic STATIC ${morphlibsrc})
# Give static library same name as shared (ok on Linux & Mac?)
set_target_properties(morphstatic PROPERTIES OUTPUT_NAME morphologica)
target_link_libraries(
  morphstatic
  armadillo ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_X11_LIB}
  )
install(TARGETS morphstatic
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/morph
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/morph
  )

# Header installation
install(
  FILES display.h sockserve.h tools.h world.h BezCurve.h BezCoord.h ReadCurves.h AllocAndRead.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph
  )
