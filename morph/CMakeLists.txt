#
# This file controls the compilation and linking of the older
# visualisation code, called libmorphdisplay0, and the installation of
# the header files.
#
# It also controls the compilation of the executable testboundary,
# which is a utility program which reads an svg file, then creates a
# HexGrid and shows the boundary. It's useful to demonstrate that the
# morphologica ReadCurves code can read an Adobe Illustrator or
# Inkscape file.
#

# Old display code (and world and sockserv):
set(morphdisplay0src display.cpp world.cpp sockserve.cpp)

# Link to the special armadillo libpath, if requested
if (NOT "${MORPH_ARMADILLO_LIBPATH}" STREQUAL "")
  message ("-- Adding MORPH_ARMADILLO_LIBPATH: ${MORPH_ARMADILLO_LIBPATH} to link directories")
  link_directories(${MORPH_ARMADILLO_LIBPATH})
endif()

#
# Link the old display code separately - morphdisplay0
#
add_library(morphdisplay0 SHARED ${morphdisplay0src})
target_include_directories(morphdisplay0 PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}> # so include "morph/blah.h" works
  $<INSTALL_INTERFACE:include>  # <prefix>/include so that include <morph/blah.h> works
)
# Common links
target_link_libraries(morphdisplay0 OpenGL::GLU)
if(APPLE)
  # Note hard-coded linking of libX11.dylib and libGL.dylib
  target_link_libraries(morphdisplay0 /opt/X11/lib/libGL.dylib ${OpenCV_LIBS} /opt/X11/lib/libX11.dylib)
else() # Linux
  target_link_libraries(morphdisplay0 ${X11_X11_LIB})
endif(APPLE)
install(TARGETS morphdisplay0 EXPORT morphologica LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Header installation
install(
  FILES display.h Quaternion.h sockserve.h tools.h world.h BezCoord.h BezCurve.h BezCurvePath.h ReadCurves.h AllocAndRead.h MorphDbg.h MathConst.h MathAlgo.h MathImpl.h number_type.h Hex.h HexGrid.h HdfData.h Process.h RD_Base.h DirichVtx.h DirichDom.h ShapeAnalysis.h RD_Plot.h NM_Simplex.h Config.h Vector.h vVector.h TransformMatrix.h ColourMap.h ColourMap_Lists.h Scale.h Random.h RecurrentNetworkTools.h RecurrentNetwork.h Winder.h expression_sfinae.h
Mnist.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph
  )
# There are also headeres in sub directories
add_subdirectory(nn) # 'nn' for neural network code

if (${glfw3_FOUND})
  install( FILES VisualCommon.h VisualBase.h Visual.h VisualModel.h VisualDataModel.h VisualTextModel.h VisualResources.h VisualFace.h CoordArrows.h HexGridVisual.h QuadsVisual.h PointRowsVisual.h ScatterVisual.h QuiverVisual.h VisualDefaultShaders.h RecurrentNetworkModel.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph)
endif()

# Install the EXPORT so that morphologica has its own .cmake file and find_package(morphologica) should work
install(FILES morphologica-config.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/morphologica)
install(EXPORT morphologica DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/morphologica)

# The testboundary utility program
add_executable(testboundary testboundary.cpp)
target_link_libraries(testboundary morphdisplay0 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS})
install(TARGETS testboundary DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
