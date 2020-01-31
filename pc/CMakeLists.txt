# Write out the pc file (in wherever the build directory is)
file(WRITE ${CMAKE_BINARY_DIR}/pc/libmorphologica.pc "prefix=${CMAKE_INSTALL_PREFIX}\n")
file(APPEND ${CMAKE_BINARY_DIR}/pc/libmorphologica.pc "libdir=\${prefix}/lib\n")
file(APPEND ${CMAKE_BINARY_DIR}/pc/libmorphologica.pc "includedir=\${prefix}/include\n\n")
file(APPEND ${CMAKE_BINARY_DIR}/pc/libmorphologica.pc "Name: ${PROJECT_NAME}\n")
file(APPEND ${CMAKE_BINARY_DIR}/pc/libmorphologica.pc "Description: ${PROJECT_NAME} library from the Wilson lab\n")
file(APPEND ${CMAKE_BINARY_DIR}/pc/libmorphologica.pc "Version: ${VERSION}\n")

if(APPLE)
  file(APPEND ${CMAKE_BINARY_DIR}/pc/libmorphologica.pc "Libs: \${libdir}/libmorphologica.dylib\n")
else()
  file(APPEND ${CMAKE_BINARY_DIR}/pc/libmorphologica.pc "Libs: -L\${libdir} -lmorphologica\n")
endif(APPLE)
file(APPEND ${CMAKE_BINARY_DIR}/pc/libmorphologica.pc "Requires: hdf5 jsoncpp lapack\n")

file(APPEND ${CMAKE_BINARY_DIR}/pc/libmorphologica.pc "Cflags: -I\${includedir}\n")

# Install
install(FILES ${CMAKE_BINARY_DIR}/pc/libmorphologica.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
