# Define which cpp files will be compiled
set(morphlibsrc display.cpp sockserve.cpp tools.cpp world.cpp BezCurve.cpp ReadCurves.cpp HexGrid.cpp HdfData.cpp Process.cpp)

# If we have glfw3 then attempt to compile Seb's modern opengl visualization code
if (${glfw3_FOUND})
  message ("libglfw3 found, compiling in Visual.cpp, etc")
  set(morphlibsrc ${morphlibsrc} Visual.cpp HexGridVisual.cpp)
  message ("morphlibsrc: ${morphlibsrc}")
endif()

if(APPLE)
  link_directories(/usr/X11R6/lib)
endif(APPLE)

if (NOT "${MORPH_ARMADILLO_LIBPATH}" STREQUAL "")
  message ("You have MORPH_ARMADILLO_LIBPATH set to: ${MORPH_ARMADILLO_LIBPATH}")
  link_directories(${MORPH_ARMADILLO_LIBPATH})
endif()

# Compile shared library
add_library(morphologica SHARED ${morphlibsrc})

# Library linking
if(APPLE)
  # Note hard-coded linking of libX11.dylib, to avoid linking to one in /opt/local. There's
  # probably a better way to do this; ideally, find_package(X11) should be pointed towards the
  # X11 located in /opt/X11.
  target_link_libraries(
    morphologica
    armadillo GL ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} /opt/X11/lib/libX11.dylib ${HDF5_C_LIBRARY_hdf5} ${LAPACK_LIBRARIES} /usr/local/lib/libjsoncpp.a
    )
else()
  target_link_libraries(
    morphologica
    ${ARMADILLO_LIBRARY} ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_X11_LIB} ${HDF5_C_${HDF_LIB_TYPE}_LIBRARY} ${LAPACK_LIBRARIES} /usr/local/lib/libjsoncpp.a
    )
endif(APPLE)

if (${glfw3_FOUND})
  target_link_libraries (morphologica glfw3)
endif()

install(TARGETS morphologica
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  )

# Static library
add_library(morphstatic STATIC ${morphlibsrc})
# Give static library same name as shared (ok on Linux & Mac?)
set_target_properties(morphstatic PROPERTIES OUTPUT_NAME morphologica)
if(APPLE)
target_link_libraries(
  morphstatic
  armadillo GL ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} /opt/X11/lib/libX11.dylib ${HDF5_C_LIBRARY_hdf5} /usr/local/lib/libjsoncpp.a
  )
else()
target_link_libraries(
  morphstatic
  ${ARMADILLO_LIBRARY} ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_X11_LIB} ${HDF5_C_${HDF_LIB_TYPE}_LIBRARY} ${LAPACK_LIBRARIES} /usr/local/lib/libjsoncpp.a
  )
endif(APPLE)

if (${glfw3_FOUND})
  target_link_libraries (morphstatic glfw3)
endif()

install(TARGETS morphstatic
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/morph
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/morph
  )

# Header installation
install(
  FILES display.h Quaternion.h sockserve.h tools.h world.h BezCoord.h BezCurve.h BezCurvePath.h ReadCurves.h AllocAndRead.h MorphDbg.h MathConst.h MathAlgo.h Hex.h HexGrid.h HdfData.h Process.h RD_Base.h DirichVtx.h DirichDom.h ShapeAnalysis.h RD_Plot.h NM_Simplex.h Config.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph
  )

if (${glfw3_FOUND})
  install( FILES Visual.h HexGridVisual.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph)
endif()

# The testboundary utility program
add_executable(testboundary testboundary.cpp)
target_link_libraries(testboundary morphologica)
install(TARGETS testboundary DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
