#
# This file controls the compilation and linking of libmorphologica,
# along with a library of the older visualisation code, called
# libmorphdisplay0
#
# It also controls the compilation of the executable testboundary,
# which is a utility program which reads an svg file, then creates a
# HexGrid and shows the boundary. It's useful to demonstrate that the
# morphologica code can read an Adobe Illustrator or Inkscape file.
#

# Old display code:
set(morphdisplay0src display.cpp)

# Define which cpp files will be compiled into libmorphologica:
set(morphlibsrc sockserve.cpp tools.cpp world.cpp ReadCurves.cpp HexGrid.cpp HdfData.cpp Process.cpp)

# If we have glfw3 then attempt to compile in Seb's modern opengl visualization code
if (${glfw3_FOUND})
  message (INFO ": libglfw3 was found, compiling in Visual.cpp, etc")
  set(morphlibsrc ${morphlibsrc} Visual.cpp)
endif()

if(APPLE)
  link_directories(/usr/X11R6/lib)
endif(APPLE)

if (NOT "${MORPH_ARMADILLO_LIBPATH}" STREQUAL "")
  message (INFO ": Adding MORPH_ARMADILLO_LIBPATH: ${MORPH_ARMADILLO_LIBPATH} to link directories")
  link_directories(${MORPH_ARMADILLO_LIBPATH})
endif()

#
# Compile shared library morphologica
#
add_library(morphologica SHARED ${morphlibsrc})
# Should add link_directories stuff here, to make sure all the libs WILL link
# Linking common to all platforms
target_link_libraries(morphologica OpenGL::GL ${HDF5_C_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} jsoncpp_static)
# Optional glfw3 linking
if (${glfw3_FOUND})
  message (INFO ": GLFW3 was found linking ${GLFW_STATIC_LDFLAGS}")
  if(APPLE)
    # This brings in the links to the Cocoa, IOKit and CoreFoundation frameworks
    target_link_libraries (morphologica glfw)
  else(APPLE)
    # On Linux, glfw requires X11 libs, at least on my Ubuntu. May not be so on Wayland based systems in future.
    target_link_libraries (morphologica ${GLFW_LIBRARIES} ${X11_X11_LIB}) # or glfw
  endif(APPLE)
  if (USE_GLEW)
    target_link_libraries (morphologica ${GLEW_LIBRARIES})
  endif (USE_GLEW)
endif()
install(TARGETS morphologica LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

#
# Link the old display code separately - morphdisplay0
#
add_library(morphdisplay0 SHARED ${morphdisplay0src})
# Common links
target_link_libraries(morphdisplay0 OpenGL::GL ${GLUT_LIBRARY})
if(APPLE)
  # Note hard-coded linking of libX11.dylib, to avoid linking to one
  # in /opt/local. There's probably a better way to do this; ideally,
  # find_package(X11) should be pointed towards the X11 located in
  # /opt/X11. Also hard-coded GL here.
  target_link_libraries(morphdisplay0 GL ${OpenCV_LIBS} /opt/X11/lib/libX11.dylib)
else() # Linux
  target_link_libraries(morphdisplay0 ${X11_X11_LIB})
endif(APPLE)
install(TARGETS morphdisplay0 LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

#
# Static library morphstatic
#
add_library(morphstatic STATIC ${morphlibsrc})
# Give static library same name as shared (ok on Linux & Mac?)
set_target_properties(morphstatic PROPERTIES OUTPUT_NAME morphologica)
# Common linking for Apple and Linux
target_link_libraries(morphstatic OpenGL::GL ${HDF5_C_LIBRARIES} ${OpenCV_LIBS} jsoncpp_static)
# The glfw3 link is common for both platforms, too, but only if found:
if (${glfw3_FOUND})
  if(APPLE)
    target_link_libraries (morphstatic glfw)
  else(APPLE)
    target_link_libraries (morphstatic glfw) # FIXME: If this works on Linux, can remove the if(APPLE)
  endif(APPLE)
endif()
# Then any platform-specific links:
target_link_libraries(morphstatic ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES})
install(TARGETS morphstatic
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/morph
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/morph
  )

# Header installation
install(
  FILES display.h Quaternion.h sockserve.h tools.h world.h BezCoord.h BezCurve.h BezCurvePath.h ReadCurves.h AllocAndRead.h MorphDbg.h MathConst.h MathAlgo.h MathImpl.h number_type.h Hex.h HexGrid.h HdfData.h Process.h RD_Base.h DirichVtx.h DirichDom.h ShapeAnalysis.h RD_Plot.h NM_Simplex.h Config.h Vector.h vVector.h TransformMatrix.h ColourMap.h ColourMap_Lists.h Scale.h Random.h RecurrentNetworkTools.h RecurrentNetwork.h Winder.h expression_sfinae.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph
  )

if (${glfw3_FOUND})
  install( FILES VisualBase.h Visual.h VisualModel.h VisualDataModel.h CoordArrows.h HexGridVisual.h QuadsVisual.h PointRowsVisual.h ScatterVisual.h QuiverVisual.h RecurrentNetworkModel.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph)
endif()

# The testboundary utility program
add_executable(testboundary testboundary.cpp)
target_link_libraries(testboundary morphologica morphdisplay0)
install(TARGETS testboundary DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
