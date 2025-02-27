# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

if(HDF5_FOUND AND ARMADILLO_FOUND)
  add_executable(lv lv.cpp)
  target_compile_definitions(lv PUBLIC FLT=float COMPILE_PLOTTING)
  if(APPLE AND OpenMP_CXX_FOUND)
    target_link_libraries(lv OpenMP::OpenMP_CXX ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype ${HDF5_C_LIBRARIES})
  else()
    target_link_libraries(lv ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype ${HDF5_C_LIBRARIES})
  endif()
endif()
