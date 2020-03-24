# Include the base directory, so that we can then #include
# <morph/header.h> either in-build, or within client code
include_directories(..)

if(APPLE)
  link_directories(/usr/X11R6/lib)
endif(APPLE)

if(${glfw3_FOUND}) # These examples require the modern OpenGL visualization stuff

  # Schnakenberg RD system
  add_executable(schnakenberg schnakenberg.cpp)
  target_compile_definitions(schnakenberg PUBLIC FLOATTYPE=float COMPILE_PLOTTING)
  target_link_libraries(schnakenberg morphologica)

  # Gierer-Meinhardt RD system
  #add_executable(gierermeinhardt gierermeinhardt.cpp)
  #target_compile_definitions(gierermeinhardt PUBLIC FLOATTYPE=float COMPILE_PLOTTING)
  #target_link_libraries(gierermeinhardt ${MORPH_LDFLAGS} ${HDF5LIBLINK} ${JSONLIBLINK})

endif()
