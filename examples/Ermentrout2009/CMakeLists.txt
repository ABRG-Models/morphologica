# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

add_executable(erm erm.cpp)
target_compile_definitions(erm PUBLIC FLT=float)
if(APPLE)
  target_link_libraries(erm OpenMP::OpenMP_CXX ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib ${HDF5_C_LIBRARIES})
else()
  target_link_libraries(erm ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib ${HDF5_C_LIBRARIES})
endif()
