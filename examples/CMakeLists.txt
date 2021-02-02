# Implements 'Subbarrel Patterns in Somatosensory Cortical Barrels Can
# Emerge from Local Dynamic Instabilities', Ermentrout, Simons and
# Land, PLOS Comp Biol 2009
add_subdirectory(Ermentrout2009)

# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

# Basic morph::Visual class example
add_executable(visual visual.cpp)
target_link_libraries(visual ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(convolve convolve.cpp)
target_link_libraries(convolve ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

# Plots a surface, counts (and shows) the frames per second
add_executable(fps fps.cpp)
target_link_libraries(fps ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(cartgrid cartgrid.cpp)
target_link_libraries(cartgrid ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(hexgrid hexgrid.cpp)
target_link_libraries(hexgrid ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
