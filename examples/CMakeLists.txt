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

# Shader compute examples. Not supported on Apple, which is limited to OpenGL 4.1.
if(NOT APPLE)
  add_subdirectory(gl_compute)
endif()

# Examples showing how to use morphologica with Qt. Compiled only if
# find_package found Qt5 and USE_GLEW is false
if(FOUND_QT_AND_HAVE_NO_GLEW)
  add_subdirectory(qt)
endif()

if(FOUND_WX_AND_GLEW)
  add_subdirectory(wx)
endif()

if (OpenGL_EGL_FOUND)
  add_subdirectory(pi)
endif()

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

add_executable(helloversion helloversion.cpp)
target_link_libraries(helloversion OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(helloversion GLEW::GLEW)
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

  # Repeated removal and re-addition of VisualModels to check memory stability
  add_executable(cartgrid_colourbar_repeat cartgrid_colourbar_repeat.cpp)
  target_link_libraries(cartgrid_colourbar_repeat OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(cartgrid_colourbar_repeat GLEW::GLEW)
  endif()

  # morph::Gridct is compile-time-configured
  if(CXX_20_AVAILABLE)
    add_executable(gridct_simple gridct_simple.cpp)
    set_property(TARGET gridct_simple PROPERTY CXX_STANDARD 20)
    target_link_libraries(gridct_simple OpenGL::GL glfw Freetype::Freetype)
    if(USE_GLEW)
      target_link_libraries(gridct_simple GLEW::GLEW)
    endif()

    add_executable(gridct_image gridct_image.cpp)
    set_property(TARGET gridct_image PROPERTY CXX_STANDARD 20)
    target_link_libraries(gridct_image OpenGL::GL glfw Freetype::Freetype)
    if(USE_GLEW)
      target_link_libraries(gridct_image GLEW::GLEW)
    endif()
  endif()

  # morph::Grid is runtime-configured
  add_executable(grid_simple grid_simple.cpp)
  target_link_libraries(grid_simple OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(grid_simple GLEW::GLEW)
  endif()

  # grid_simple, but orthographic view is the default
  add_executable(grid_ortho grid_simple.cpp)
  target_compile_definitions(grid_ortho PUBLIC ORTHOGRAPHIC=1)
  target_link_libraries(grid_ortho OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(grid_ortho GLEW::GLEW)
  endif()

  # similar to grid_simple but with a random value for the pixel which highlight better the different perspective
  # morph::Grid is runtime-configured
  add_executable(grid_simple_rand grid_simple_rand.cpp)
  target_link_libraries(grid_simple_rand OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(grid_simple_rand GLEW::GLEW)
  endif()

  # grid_simple_rand, but orthographic view is the default
  add_executable(grid_ortho_rand grid_simple_rand.cpp)
  target_compile_definitions(grid_ortho_rand PUBLIC ORTHOGRAPHIC=1)
  target_link_libraries(grid_ortho_rand OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(grid_ortho_rand GLEW::GLEW)
  endif()

  add_executable(grid_image grid_image.cpp)
  target_link_libraries(grid_image OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(grid_image GLEW::GLEW)
  endif()

  # Like HexGrid::resampleImage, this one uses an omp call that only works on Mac with
  # libomp, so avoid compiling this example on a Mac.
  if(NOT APPLE)
    add_executable(grid_resampled_image grid_resampled_image.cpp)
    target_link_libraries(grid_resampled_image OpenGL::GL glfw Freetype::Freetype)
    if(USE_GLEW)
      target_link_libraries(grid_resampled_image GLEW::GLEW)
    endif()
  endif()

  add_executable(grid_layout grid_layout.cpp)

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

  add_executable(colourbars colourbars.cpp)
  target_link_libraries(colourbars OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(colourbars GLEW::GLEW)
  endif()

  add_executable(unicode_coordaxes unicode_coordaxes.cpp)
  target_link_libraries(unicode_coordaxes OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(unicode_coordaxes GLEW::GLEW)
  endif()

endif(ARMADILLO_FOUND)

add_executable(colourmaps colourmaps.cpp)
target_link_libraries(colourmaps OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(colourmaps GLEW::GLEW)
endif()

# On MacOS 13 as a github runner, HAVE_STD_FORMAT gets set to 1, but
# the compile still fails. Odd. The build of these targets *does* work
# on MacOS 14. Nevertheless, I have excluded APPLE builds for these
# examples for now.
CHECK_INCLUDE_FILE_CXX(format HAVE_STD_FORMAT)
if(NOT APPLE AND HAVE_STD_FORMAT)
  add_executable(colourmaps_mono colourmaps_mono.cpp)
  set_property(TARGET colourmaps_mono PROPERTY CXX_STANDARD 20)
  target_link_libraries(colourmaps_mono OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(colourmaps_mono GLEW::GLEW)
  endif()

  add_executable(colourmaps_hsv1d colourmaps_hsv1d.cpp)
  set_property(TARGET colourmaps_hsv1d PROPERTY CXX_STANDARD 20)
  target_link_libraries(colourmaps_hsv1d OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(colourmaps_hsv1d GLEW::GLEW)
  endif()
endif()

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

add_executable(vectorvis vectorvis.cpp)
target_link_libraries(vectorvis OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(vectorvis GLEW::GLEW)
endif()

add_executable(scatter scatter.cpp)
set_property(TARGET scatter PROPERTY CXX_STANDARD 20)
target_link_libraries(scatter OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(scatter GLEW::GLEW)
endif()

# This sphere example uses constexpr code
if(NOT CMAKE_CXX_COMPILER_ID MATCHES Clang)
  add_executable(scatter_ico scatter_ico.cpp)
  target_link_libraries(scatter_ico OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(scatter_ico GLEW::GLEW)
  endif()

  add_executable(scatter_geodesic scatter_geodesic.cpp)
  target_link_libraries(scatter_geodesic OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(scatter_geodesic GLEW::GLEW)
  endif()
endif()

add_executable(scatter_hex_mercator scatter_hex_mercator.cpp)
target_link_libraries(scatter_hex_mercator OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(scatter_hex_mercator GLEW::GLEW)
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

add_executable(graph_dynamic_x2 graph_dynamic_x2.cpp)
target_link_libraries(graph_dynamic_x2 OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph_dynamic_x2 GLEW::GLEW)
endif()

add_executable(graph_fouraxes graph_fouraxes.cpp)
target_link_libraries(graph_fouraxes OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph_fouraxes GLEW::GLEW)
endif()

add_executable(graph_incoming_data graph_incoming_data.cpp)
target_link_libraries(graph_incoming_data OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph_incoming_data GLEW::GLEW)
endif()

add_executable(graph_incoming_data_rescale graph_incoming_data_rescale.cpp)
target_link_libraries(graph_incoming_data_rescale OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph_incoming_data_rescale GLEW::GLEW)
endif()

add_executable(graph_twinax graph_twinax.cpp)
target_link_libraries(graph_twinax OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph_twinax GLEW::GLEW)
endif()

add_executable(graph_dynamic_sine graph_dynamic_sine.cpp)
target_link_libraries(graph_dynamic_sine OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph_dynamic_sine GLEW::GLEW)
endif()

add_executable(graph_bar graph_bar.cpp)
target_link_libraries(graph_bar OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph_bar GLEW::GLEW)
endif()

add_executable(graph_histo graph_histo.cpp)
target_link_libraries(graph_histo OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(graph_histo GLEW::GLEW)
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

add_executable(threewindows threewindows.cpp)
target_link_libraries(threewindows OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(threewindows GLEW::GLEW)
endif()

add_executable(rod rod.cpp)
target_link_libraries(rod OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(rod GLEW::GLEW)
endif()

# This example uses constexpr code
if(NOT CMAKE_CXX_COMPILER_ID MATCHES Clang)
  add_executable(grating grating.cpp)
  target_link_libraries(grating OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(grating GLEW::GLEW)
  endif()
endif()

add_executable(cone cone.cpp)
target_link_libraries(cone OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(cone GLEW::GLEW)
endif()

add_executable(ring ring.cpp)
target_link_libraries(ring OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(ring GLEW::GLEW)
endif()

# This sphere example uses constexpr code
if(NOT CMAKE_CXX_COMPILER_ID MATCHES Clang)
  add_executable(sphere sphere.cpp)
  target_link_libraries(sphere OpenGL::GL glfw Freetype::Freetype)
  set_property(TARGET sphere PROPERTY CXX_STANDARD 20)
  if(USE_GLEW)
    target_link_libraries(sphere GLEW::GLEW)
  endif()
endif()

# constexpr code won't work on Clang
if(NOT CMAKE_CXX_COMPILER_ID MATCHES Clang)
  add_executable(icosahedron icosahedron.cpp)
  target_link_libraries(icosahedron OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(icosahedron GLEW::GLEW)
  endif()

  # Need C++20 for the way I use constexpr here so that it is legal to
  # have "a definition of a variable for which no initialization is
  # performed".
  add_executable(testce testce.cpp)
  set_property(TARGET testce PROPERTY CXX_STANDARD 20)

  add_executable(geodesic geodesic.cpp)
  target_link_libraries(geodesic OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(geodesic GLEW::GLEW)
  endif()

  add_executable(geodesic_ce geodesic_ce.cpp)
  set_property(TARGET geodesic_ce PROPERTY CXX_STANDARD 20)
  target_link_libraries(geodesic_ce OpenGL::GL glfw Freetype::Freetype)
  if(USE_GLEW)
    target_link_libraries(geodesic_ce GLEW::GLEW)
  endif()
endif()

add_executable(tri tri.cpp)
target_link_libraries(tri OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(tri GLEW::GLEW)
endif()

add_executable(draw_triangles draw_triangles.cpp)
target_link_libraries(draw_triangles OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(draw_triangles GLEW::GLEW)
endif()

add_executable(rectangle rectangle.cpp)
target_link_libraries(rectangle OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(rectangle GLEW::GLEW)
endif()

add_executable(rhombo rhombo.cpp)
target_link_libraries(rhombo OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(rhombo GLEW::GLEW)
endif()

add_executable(rhombo_scene rhombo_scene.cpp)
target_link_libraries(rhombo_scene OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(rhombo_scene GLEW::GLEW)
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

add_executable(hexgrid_image_rect hexgrid_image_rect.cpp)
target_link_libraries(hexgrid_image_rect OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(hexgrid_image_rect GLEW::GLEW)
endif()

add_executable(hexgrid_image_onsphere hexgrid_image_onsphere.cpp)
target_link_libraries(hexgrid_image_onsphere OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(hexgrid_image_onsphere GLEW::GLEW)
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


add_executable(vvec_rgb_gaussians vvec_rgb_gaussians.cpp)
target_link_libraries(vvec_rgb_gaussians OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(vvec_rgb_gaussians GLEW::GLEW)
endif()

add_executable(vvec_smoothgauss vvec_smoothgauss.cpp)
target_link_libraries(vvec_smoothgauss OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(vvec_smoothgauss GLEW::GLEW)
endif()

add_executable(quit_callback quit_callback.cpp)
target_link_libraries(quit_callback OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(quit_callback GLEW::GLEW)
endif()

add_executable(showcase showcase.cpp)
target_link_libraries(showcase OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(showcase GLEW::GLEW)
endif()

add_executable(izhikevich izhikevich.cpp)
target_link_libraries(izhikevich OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(izhikevich GLEW::GLEW)
endif()

add_executable(curvytelly curvytelly.cpp)
target_link_libraries(curvytelly OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(curvytelly GLEW::GLEW)
endif()

add_executable(curvytelly_pipe curvytelly_pipe.cpp)
target_link_libraries(curvytelly_pipe OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(curvytelly_pipe GLEW::GLEW)
endif()

add_executable(curvytelly_chequered_pipe curvytelly_chequered_pipe.cpp)
target_link_libraries(curvytelly_chequered_pipe OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(curvytelly_chequered_pipe GLEW::GLEW)
endif()

add_executable(lighting_test lighting_test.cpp)
target_link_libraries(lighting_test OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(lighting_test GLEW::GLEW)
endif()
