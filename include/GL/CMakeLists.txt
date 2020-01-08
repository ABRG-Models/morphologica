# Install the GL headers in the morph include dir.
install(
  FILES glcorearb.h  glext.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph/GL
  )
