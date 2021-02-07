# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

add_executable(softcollision colliding.cpp)

target_compile_definitions(softcollision PUBLIC FLT=float)
if(APPLE)
  target_link_libraries(softcollision OpenMP::OpenMP_CXX ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw jsoncpp_lib ${HDF5_C_LIBRARIES})
else()
  target_link_libraries(softcollision softmats ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} ${OPENGL_LIBRARY} ${GLM_LIBRARY} glfw GLEW jsoncpp_lib ${HDF5_C_LIBRARIES})
endif()
