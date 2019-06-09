# Define which cpp files will be compiled
set(morphlibsrc display.cpp Visual.cpp HexGridVisual.cpp sockserve.cpp tools.cpp world.cpp BezCurve.cpp ReadCurves.cpp HexGrid.cpp HdfData.cpp)

if(APPLE)
  link_directories(/usr/X11R6/lib)
endif(APPLE)

if (NOT "${MORPH_ARMADILLO_LIBPATH}" STREQUAL "")
  message ("You have MORPH_ARMADILLO_LIBPATH set to: ${MORPH_ARMADILLO_LIBPATH}")
  link_directories(${MORPH_ARMADILLO_LIBPATH})
endif()

# Find GLFW library? Or compile in place (that would be better)
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  # Use pkg-config to check GLFW3 is present:
  pkg_check_modules(GLFW3 REQUIRED glfw3)
  if (GLFW3_FOUND)
    # Good....
    message(INFO, "pkg-config found glfw3. Excellent.")
  else(GLFW3_FOUND)
    # Bad....
    message(ERROR, "glfw3 was NOT found by pkg-config. Uh oh.")
  endif(GLFW3_FOUND)
else()
  message(WARNING "There's no pkg-config on this system to check for libglfw3. You may need to try `sudo apt-get install pkg-config`")
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
    ${ARMADILLO_LIBRARY} GL ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} /opt/X11/lib/libX11.dylib ${HDF5_C_LIBRARY_hdf5} ${LAPACK_LIBRARIES} ${GLFW3_LDFLAGS}
    )
else()
  target_link_libraries(
    morphologica
    ${ARMADILLO_LIBRARY} ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_X11_LIB} ${HDF5_C_${HDF_LIB_TYPE}_LIBRARY} ${LAPACK_LIBRARIES} /usr/local/lib/libglfw3.a
    )
endif(APPLE)
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
  ${ARMADILLO_LIBRARY} GL ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} /opt/X11/lib/libX11.dylib ${HDF5_C_LIBRARY_hdf5} ${LAPACK_LIBRARIES}
  )
else()
target_link_libraries(
  morphstatic
  ${ARMADILLO_LIBRARY} ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_X11_LIB} ${HDF5_C_${HDF_LIB_TYPE}_LIBRARY} ${LAPACK_LIBRARIES} ${GLFW3_STATIC_LDFLAGS}
  )
endif(APPLE)

install(TARGETS morphstatic
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/morph
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/morph
  )

# Header installation
install(
  FILES display.h Visual.h HexGridVisual.h Quaternion.h sockserve.h tools.h world.h BezCoord.h BezCurve.h BezCurvePath.h ReadCurves.h AllocAndRead.h MorphDbg.h Hex.h HexGrid.h HdfData.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph
  )

# For debugging, you can list variables like this:
set(DEBUG_VARIABLES ON)
if(DEBUG_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif(DEBUG_VARIABLES)
