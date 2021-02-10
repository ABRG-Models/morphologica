# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

add_executable(sg simpsongoodhill.cpp)
target_compile_definitions(sg PUBLIC FLT=float)
if(APPLE AND OpenMP_CXX_FOUND)
  target_link_libraries(sg OpenMP::OpenMP_CXX ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib ${HDF5_C_LIBRARIES})
else()
  target_link_libraries(sg ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib ${HDF5_C_LIBRARIES})
endif()
