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

if(ARMADILLO_FOUND)
  # Basic morph::Visual class example
  add_executable(visual visual.cpp)
  target_link_libraries(visual OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(visual GLEW::GLEW)
  endif()
endif()

add_executable(helloworld helloworld.cpp)
target_link_libraries(helloworld OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(helloworld GLEW::GLEW)
endif()

add_executable(myvisual myvisual.cpp)
target_link_libraries(myvisual OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(myvisual GLEW::GLEW)
endif()

add_executable(convolve_rect convolve_rect.cpp)
target_link_libraries(convolve_rect OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(convolve_rect GLEW::GLEW)
endif()

if(ARMADILLO_FOUND)
  add_executable(convolve convolve.cpp)
  target_link_libraries(convolve OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(convolve GLEW::GLEW)
  endif()

  # Adaptive Simulated Annealing algo on a made up objective
  add_executable(anneal_asa anneal_asa.cpp)
  target_compile_definitions(anneal_asa PUBLIC VISUALISE)
  target_link_libraries(anneal_asa OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(anneal_asa GLEW::GLEW)
  endif()

  add_executable(anneal_asa_novis anneal_asa.cpp)
  target_link_libraries(anneal_asa_novis)

  # Adaptive Simulated Annealing algo on objective from Bohachevsky
  add_executable(anneal_boha anneal_asa.cpp)
  target_compile_definitions(anneal_boha PUBLIC USE_BOHACHEVSKY_FUNCTION VISUALISE)
  target_link_libraries(anneal_boha OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(anneal_boha GLEW::GLEW)
  endif()

  # Plots a surface, counts (and shows) the frames per second
  add_executable(fps fps.cpp)
  if(APPLE AND OpenMP_CXX_FOUND)
    target_link_libraries(fps OpenMP::OpenMP_CXX OpenGL::GL glfw Freetype::Freetype)
  else()
    target_link_libraries(fps OpenGL::GL glfw Freetype::Freetype)
  endif()
  if(USE_GLEW)
    target_link_libraries(fps GLEW::GLEW)
  endif()

  add_executable(cartgrid cartgrid.cpp)
  target_link_libraries(cartgrid OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(cartgrid GLEW::GLEW)
  endif()

  add_executable(hsvwheel hsvwheel.cpp)
  target_link_libraries(hsvwheel OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(hsvwheel GLEW::GLEW)
  endif()

  # ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES}
  add_executable(hexgrid hexgrid.cpp)
  target_link_libraries(hexgrid OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(hexgrid GLEW::GLEW)
  endif()

  add_executable(unicode_coordaxes unicode_coordaxes.cpp)
  target_link_libraries(unicode_coordaxes OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(unicode_coordaxes GLEW::GLEW)
  endif()

endif(ARMADILLO_FOUND)

add_executable(logisticmap logisticmap.cpp)
target_link_libraries(logisticmap OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(logisticmap GLEW::GLEW)
endif()

add_executable(quiver quiver.cpp)
target_link_libraries(quiver OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(quiver GLEW::GLEW)
endif()

add_executable(scatter scatter.cpp)
target_link_libraries(scatter OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(scatter GLEW::GLEW)
endif()

add_executable(scatter_dynamic scatter_dynamic.cpp)
target_link_libraries(scatter_dynamic OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(scatter_dynamic GLEW::GLEW)
endif()

add_executable(duochrome duochrome.cpp)
target_link_libraries(duochrome OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(duochrome GLEW::GLEW)
endif()

add_executable(graph1 graph1.cpp)
target_link_libraries(graph1 OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph1 GLEW::GLEW)
endif()

add_executable(graph2 graph2.cpp)
target_link_libraries(graph2 OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph2 GLEW::GLEW)
endif()

add_executable(graph3 graph3.cpp)
target_link_libraries(graph3 OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph3 GLEW::GLEW)
endif()

add_executable(graph4 graph4.cpp)
target_link_libraries(graph4 OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph4 GLEW::GLEW)
endif()

add_executable(graph4_rescale graph4_rescale.cpp)
target_link_libraries(graph4_rescale OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph4_rescale GLEW::GLEW)
endif()

add_executable(graph5 graph5.cpp)
target_link_libraries(graph5 OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph5 GLEW::GLEW)
endif()

add_executable(graph6 graph6.cpp)
target_link_libraries(graph6 OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph6 GLEW::GLEW)
endif()

add_executable(graph_bar graph_bar.cpp)
target_link_libraries(graph_bar OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph_bar GLEW::GLEW)
endif()

add_executable(graph_logist graph_logist.cpp)
target_link_libraries(graph_logist OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph_logist GLEW::GLEW)
endif()

add_executable(graph_logist2 graph_logist2.cpp)
target_link_libraries(graph_logist2 OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph_logist2 GLEW::GLEW)
endif()

add_executable(graph_coords graph_coords.cpp)
target_link_libraries(graph_coords OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph_coords GLEW::GLEW)
endif()

add_executable(linregr linregr.cpp)
target_link_libraries(linregr OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(linregr GLEW::GLEW)
endif()

add_executable(twowindows twowindows.cpp)
target_link_libraries(twowindows OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(twowindows GLEW::GLEW)
endif()

add_executable(rod rod.cpp)
target_link_libraries(rod OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(rod GLEW::GLEW)
endif()

add_executable(tri tri.cpp)
target_link_libraries(tri OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(tri GLEW::GLEW)
endif()

add_executable(rhombo rhombo.cpp)
target_link_libraries(rhombo OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(rhombo GLEW::GLEW)
endif()

add_executable(quads quads.cpp)
target_link_libraries(quads OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(quads GLEW::GLEW)
endif()

add_executable(quadsmesh quads.cpp)
target_compile_definitions(quadsmesh PUBLIC MESH=1)
target_link_libraries(quadsmesh OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(quadsmesh GLEW::GLEW)
endif()

add_executable(pointrows pointrows.cpp)
target_link_libraries(pointrows OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(pointrows GLEW::GLEW)
endif()

add_executable(pointrows_mesh pointrows.cpp)
target_compile_definitions(pointrows_mesh PUBLIC MESH=1)
target_link_libraries(pointrows_mesh OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(pointrows_mesh GLEW::GLEW)
endif()

if(ARMADILLO_FOUND)
  add_executable(rosenbrock rosenbrock.cpp)
  target_compile_definitions(rosenbrock PUBLIC FLT=float)
  target_link_libraries(rosenbrock OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(rosenbrock GLEW::GLEW)
  endif()

  add_executable(rosenbrock_asa rosenbrock_asa.cpp)
  target_compile_definitions(rosenbrock_asa PUBLIC FLT=float VISUALISE)
  target_link_libraries(rosenbrock_asa OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(rosenbrock_asa GLEW::GLEW)
  endif()
endif(ARMADILLO_FOUND)

add_executable(jsonconfig jsonconfig.cpp)
target_link_libraries(jsonconfig)

if(HDF5_FOUND)
  add_executable(hdfdata hdfdata.cpp)
  target_link_libraries(hdfdata ${HDF5_C_LIBRARIES})
endif()

add_executable(randvec randvec.cpp)
target_link_libraries(randvec OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(randvec GLEW::GLEW)
endif()

# Demo of morph's bootstrap implementation
add_executable(bootstrap bootstrap.cpp)
target_link_libraries(bootstrap OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(bootstrap GLEW::GLEW)
endif()

add_executable(rng rng.cpp)
target_link_libraries(rng)

if(ARMADILLO_FOUND)
  # Utility prog to test your .svg boundary files
  add_executable(show_svg_boundary show_svg_boundary.cpp)
  target_link_libraries(show_svg_boundary ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(show_svg_boundary GLEW::GLEW)
  endif()
endif(ARMADILLO_FOUND)

# HexGrid::resampleImage uses an omp call that only works on Mac with
# libomp, so avoid compiling this example on a Mac.
if(NOT APPLE)
add_executable(hexgrid_image hexgrid_image.cpp)
target_link_libraries(hexgrid_image OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(hexgrid_image GLEW::GLEW)
endif()
endif()

add_executable(scale scale.cpp)

add_executable(vvec vvec.cpp)

add_executable(vvec_convolve vvec_convolve.cpp)
target_link_libraries(vvec_convolve OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(vvec_convolve GLEW::GLEW)
endif()

add_executable(vvec_gauss vvec_gauss.cpp)
target_link_libraries(vvec_gauss OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(vvec_gauss GLEW::GLEW)
endif()

add_executable(vvec_smoothgauss vvec_smoothgauss.cpp)
target_link_libraries(vvec_smoothgauss OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(vvec_smoothgauss GLEW::GLEW)
endif()
