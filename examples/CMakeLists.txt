# Implements 'Subbarrel Patterns in Somatosensory Cortical Barrels Can
# Emerge from Local Dynamic Instabilities', Ermentrout, Simons and
# Land, PLOS Comp Biol 2009
add_subdirectory(Ermentrout2009)

# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

# Basic morph::Visual class example
add_executable(exampleVisual exampleVisual.cpp)
target_link_libraries(exampleVisual ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(hexConvolve hexConvolve.cpp)
target_link_libraries(hexConvolve ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

# Plots a surface, counts (and shows) the frames per second
add_executable(fps fps.cpp)
target_link_libraries(fps ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(cartgrid cartgrid.cpp)
target_link_libraries(cartgrid ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
