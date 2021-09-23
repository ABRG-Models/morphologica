# Implements 'Subbarrel Patterns in Somatosensory Cortical Barrels Can
# Emerge from Local Dynamic Instabilities', Ermentrout, Simons and
# Land, PLOS Comp Biol 2009
add_subdirectory(Ermentrout2009)

# The Lotka-Volterra population model. See Pau, 1992, p654.
add_subdirectory(LotkaVolterra)

# The Schnakenberg RD model.
add_subdirectory(schnakenberg)

# A retinotectal model published by Simpson and Goodhill, 2011
add_subdirectory(SimpsonGoodhill)

# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

# Basic morph::Visual class example
add_executable(visual visual.cpp)
target_link_libraries(visual ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(helloworld helloworld.cpp)
target_link_libraries(helloworld ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(convolve convolve.cpp)
target_link_libraries(convolve ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

# Adaptive Simulated Annealing algo on a made up objective
add_executable(anneal_asa anneal_asa.cpp)
target_link_libraries(anneal_asa ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

# Adaptive Simulated Annealing algo on objective from Bohachevsky
add_executable(anneal_boha anneal_boha.cpp)
target_link_libraries(anneal_boha ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

# Plots a surface, counts (and shows) the frames per second
add_executable(fps fps.cpp)
if(APPLE AND OpenMP_CXX_FOUND)
  target_link_libraries(fps OpenMP::OpenMP_CXX ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
else()
  target_link_libraries(fps ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
endif()

add_executable(cartgrid cartgrid.cpp)
target_link_libraries(cartgrid ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(hexgrid hexgrid.cpp)
target_link_libraries(hexgrid ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(hexgrid_pgram hexgrid_pgram.cpp)
target_link_libraries(hexgrid_pgram ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(hexgrid_hexdom hexgrid_hexdom.cpp)
target_link_libraries(hexgrid_hexdom ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(logisticmap logisticmap.cpp)
target_link_libraries(logisticmap ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(quiver quiver.cpp)
target_link_libraries(quiver ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(scatter scatter.cpp)
target_link_libraries(scatter ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(duochrome duochrome.cpp)
target_link_libraries(duochrome ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(graph1 graph1.cpp)
target_link_libraries(graph1 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(graph2 graph2.cpp)
target_link_libraries(graph2 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(graph3 graph3.cpp)
target_link_libraries(graph3 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(graph4 graph4.cpp)
target_link_libraries(graph4 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(graph_bar graph_bar.cpp)
target_link_libraries(graph_bar ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(linregr linregr.cpp)
target_link_libraries(linregr ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(twowindows twowindows.cpp)
target_link_libraries(twowindows ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(rod rod.cpp)
target_link_libraries(rod ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(tri tri.cpp)
target_link_libraries(tri ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(quads quads.cpp)
target_link_libraries(quads ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(pointrows pointrows.cpp)
target_link_libraries(pointrows ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(pointrows_mesh pointrows.cpp)
target_compile_definitions(pointrows_mesh PUBLIC MESH=1)
target_link_libraries(pointrows_mesh ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(rosenbrock rosenbrock.cpp)
target_compile_definitions(rosenbrock PUBLIC FLT=float)
target_link_libraries(rosenbrock ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(jsonconfig jsonconfig.cpp)
target_link_libraries(jsonconfig jsoncpp_lib)

add_executable(hdfdata hdfdata.cpp)
target_link_libraries(hdfdata ${HDF5_C_LIBRARIES})

add_executable(randvec randvec.cpp)
target_link_libraries(randvec ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)

add_executable(rng rng.cpp)
target_link_libraries(rng)

# Utility prog to test your .svg boundary files
add_executable(show_svg_boundary show_svg_boundary.cpp)
target_link_libraries(show_svg_boundary ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
