# Define which cpp files will be compiled
set(morphlibsrc display.cpp sockserve.cpp tools.cpp world.cpp BezCurve.cpp ReadCurves.cpp HexGrid.cpp HdfData.cpp Process.cpp)

# If we have glfw3 then attempt to compile Seb's modern opengl visualization code
if (${glfw3_FOUND})
  message ("GLFW3 found, compiling in Visual.cpp, etc")
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
    armadillo GL ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} /opt/X11/lib/libX11.dylib ${HDF5_C_LIBRARY_hdf5} ${LAPACK_LIBRARIES}
    )
else()
  target_link_libraries(
    morphologica
    ${ARMADILLO_LIBRARY} ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_X11_LIB} ${HDF5_C_${HDF_LIB_TYPE}_LIBRARY} ${LAPACK_LIBRARIES}
    )
endif(APPLE)

if (${glfw3_FOUND})
  target_link_libraries (morphologica ${GLFW3_LDFLAGS})
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
  armadillo GL ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} /opt/X11/lib/libX11.dylib ${HDF5_C_LIBRARY_hdf5}
  )
else()
target_link_libraries(
  morphstatic
  ${ARMADILLO_LIBRARY} ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_X11_LIB} ${HDF5_C_${HDF_LIB_TYPE}_LIBRARY} ${LAPACK_LIBRARIES}
  )
endif(APPLE)

if (${glfw3_FOUND})
  target_link_libraries (morphstatic ${GLFW3_STATIC_LDFLAGS})
endif()

install(TARGETS morphstatic
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/morph
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/morph
  )

# Header installation
install(
  FILES display.h Quaternion.h sockserve.h tools.h world.h BezCoord.h BezCurve.h BezCurvePath.h ReadCurves.h AllocAndRead.h MorphDbg.h Hex.h HexGrid.h HdfData.h Process.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph
  )

if (${glfw3_FOUND})
  install( FILES Visual.h HexGridVisual.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph)
endif()

# The testboundary utility program
add_executable(testboundary testboundary.cpp)
target_link_libraries(testboundary morphologica)
install(TARGETS testboundary DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
