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

# Shader compute examples. Not supported on Apple, which is limited to
# OpenGL 4.1 or Windows because I want to keep the GL header inclusion
# simple on these examples.
if(NOT APPLE AND NOT WIN32)
  add_subdirectory(gl_compute)
endif()

# Examples showing how to use morphologica with Qt. Compiled only if
# find_package found Qt5
if(Qt5_FOUND)
  add_subdirectory(qt)
endif()

if(wxWidgets_FOUND)
  add_subdirectory(wx)
endif()

if (OpenGL_EGL_FOUND)
  add_subdirectory(pi)
endif()

# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

#
# Any example that uses morph::HexGrid or morph::CartGrid requires
# libarmadillo (because these classes use morph::BezCurve).
#
if(ARMADILLO_FOUND)
  # Basic morph::Visual class example
  add_executable(visual visual.cpp)
  target_link_libraries(visual OpenGL::GL glfw Freetype::Freetype)

  add_executable(convolve_rect convolve_rect.cpp)
  target_link_libraries(convolve_rect OpenGL::GL glfw Freetype::Freetype)

  add_executable(convolve convolve.cpp)
  target_link_libraries(convolve OpenGL::GL glfw Freetype::Freetype)

  # Adaptive Simulated Annealing algo on a made up objective
  if(HDF5_FOUND)
    add_executable(anneal_asa anneal_asa.cpp)
    target_compile_definitions(anneal_asa PUBLIC VISUALISE)
    target_link_libraries(anneal_asa OpenGL::GL glfw Freetype::Freetype)

    add_executable(anneal_asa_novis anneal_asa.cpp)
    target_link_libraries(anneal_asa_novis)

    # Adaptive Simulated Annealing algo on objective from Bohachevsky
    add_executable(anneal_boha anneal_asa.cpp)
    target_compile_definitions(anneal_boha PUBLIC USE_BOHACHEVSKY_FUNCTION VISUALISE)
    target_link_libraries(anneal_boha OpenGL::GL glfw Freetype::Freetype)
  endif(HDF5_FOUND)

  # Plots a surface, counts (and shows) the frames per second
  add_executable(fps fps.cpp)
  if(APPLE AND OpenMP_CXX_FOUND)
    target_link_libraries(fps OpenMP::OpenMP_CXX OpenGL::GL glfw Freetype::Freetype)
  else()
    target_link_libraries(fps OpenGL::GL glfw Freetype::Freetype)
  endif()

  add_executable(cartgrid cartgrid.cpp)
  target_link_libraries(cartgrid OpenGL::GL glfw Freetype::Freetype)

  # Repeated removal and re-addition of VisualModels to check memory stability
  add_executable(cartgrid_colourbar_repeat cartgrid_colourbar_repeat.cpp)
  target_link_libraries(cartgrid_colourbar_repeat OpenGL::GL glfw Freetype::Freetype)

  # Like HexGrid::resampleImage, this one uses an omp call that only works on Mac with
  # libomp, so avoid compiling this example on a Mac.
  if(NOT APPLE)
    add_executable(grid_resampled_image grid_resampled_image.cpp)
    target_link_libraries(grid_resampled_image OpenGL::GL glfw Freetype::Freetype)
  endif()

  add_executable(hexgrid hexgrid.cpp)
  target_link_libraries(hexgrid OpenGL::GL glfw Freetype::Freetype)

  add_executable(unicode_coordaxes unicode_coordaxes.cpp)
  target_link_libraries(unicode_coordaxes OpenGL::GL glfw Freetype::Freetype)

  add_executable(colourbar colourbar.cpp)
  target_link_libraries(colourbar OpenGL::GL glfw Freetype::Freetype)

  add_executable(scatter_hex_mercator scatter_hex_mercator.cpp)
  target_link_libraries(scatter_hex_mercator OpenGL::GL glfw Freetype::Freetype)

  add_executable(rosenbrock rosenbrock.cpp)
  target_compile_definitions(rosenbrock PUBLIC FLT=float)
  target_link_libraries(rosenbrock OpenGL::GL glfw Freetype::Freetype)

  if(HDF5_FOUND)
    add_executable(rosenbrock_asa rosenbrock_asa.cpp)
    target_compile_definitions(rosenbrock_asa PUBLIC FLT=float VISUALISE)
    target_link_libraries(rosenbrock_asa OpenGL::GL glfw Freetype::Freetype)
  endif(HDF5_FOUND)

  # Utility prog to test your .svg boundary files
  add_executable(show_svg_boundary show_svg_boundary.cpp)
  target_link_libraries(show_svg_boundary ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype)

  # HexGrid::resampleImage uses an omp call that only works on Mac with
  # libomp, so avoid compiling this example on a Mac.
  if(NOT APPLE)
    add_executable(hexgrid_image hexgrid_image.cpp)
    target_link_libraries(hexgrid_image OpenGL::GL glfw Freetype::Freetype)

    add_executable(hexgrid_image_rect hexgrid_image_rect.cpp)
    target_link_libraries(hexgrid_image_rect OpenGL::GL glfw Freetype::Freetype)

    add_executable(hexgrid_image_onsphere hexgrid_image_onsphere.cpp)
    target_link_libraries(hexgrid_image_onsphere OpenGL::GL glfw Freetype::Freetype)
  endif(NOT APPLE)

  add_executable(showcase showcase.cpp)
  target_link_libraries(showcase OpenGL::GL glfw Freetype::Freetype)

endif(ARMADILLO_FOUND)

add_executable(helloworld helloworld.cpp)
target_link_libraries(helloworld OpenGL::GL glfw Freetype::Freetype)

add_executable(helloversion helloversion.cpp)
target_link_libraries(helloversion OpenGL::GL glfw Freetype::Freetype)

add_executable(myvisual myvisual.cpp)
target_link_libraries(myvisual OpenGL::GL glfw Freetype::Freetype)

if(ARRAY_LITERAL_TEMPL_ARGS_ARE_OK)
  add_executable(gridct_simple gridct_simple.cpp)
  target_link_libraries(gridct_simple OpenGL::GL glfw Freetype::Freetype)

  add_executable(gridct_image gridct_image.cpp)
  target_link_libraries(gridct_image OpenGL::GL glfw Freetype::Freetype)
endif()

# morph::Grid is runtime-configured
add_executable(grid_simple grid_simple.cpp)
target_link_libraries(grid_simple OpenGL::GL glfw Freetype::Freetype)

add_executable(grid_mnist grid_mnist.cpp)
target_link_libraries(grid_mnist OpenGL::GL glfw Freetype::Freetype)

add_executable(grid_flat_dynamic grid_flat_dynamic.cpp)
target_link_libraries(grid_flat_dynamic OpenGL::GL glfw Freetype::Freetype)

add_executable(colourmap_test colourmap_test.cpp)
target_link_libraries(colourmap_test OpenGL::GL glfw Freetype::Freetype)

add_executable(colourmap_browser colourmap_browser.cpp)
target_link_libraries(colourmap_browser OpenGL::GL glfw Freetype::Freetype)

# grid_simple, but orthographic view is the default
add_executable(grid_ortho grid_simple.cpp)
target_compile_definitions(grid_ortho PUBLIC ORTHOGRAPHIC=1)
target_link_libraries(grid_ortho OpenGL::GL glfw Freetype::Freetype)

# similar to grid_simple but with a random value for the pixel which highlight better the different perspective
# morph::Grid is runtime-configured
add_executable(grid_simple_rand grid_simple_rand.cpp)
target_link_libraries(grid_simple_rand OpenGL::GL glfw Freetype::Freetype)

# grid_simple_rand, but orthographic view is the default
add_executable(grid_ortho_rand grid_simple_rand.cpp)
target_compile_definitions(grid_ortho_rand PUBLIC ORTHOGRAPHIC=1)
target_link_libraries(grid_ortho_rand OpenGL::GL glfw Freetype::Freetype)

add_executable(grid_image grid_image.cpp)
target_link_libraries(grid_image OpenGL::GL glfw Freetype::Freetype)

add_executable(grid_border grid_border.cpp)
target_link_libraries(grid_border OpenGL::GL glfw Freetype::Freetype)

add_executable(grid_border2 grid_border2.cpp)
target_link_libraries(grid_border2 OpenGL::GL glfw Freetype::Freetype)

add_executable(grid_layout grid_layout.cpp)

add_executable(hsvwheel hsvwheel.cpp)
target_link_libraries(hsvwheel OpenGL::GL glfw Freetype::Freetype)

add_executable(cyclic_colour cyclic_colour.cpp)
target_link_libraries(cyclic_colour OpenGL::GL glfw Freetype::Freetype)

add_executable(colourmaps_desaturating colourmaps_desaturating.cpp)
target_link_libraries(colourmaps_desaturating OpenGL::GL glfw Freetype::Freetype)

add_executable(colourmaps_other colourmaps_other.cpp)
target_link_libraries(colourmaps_other OpenGL::GL glfw Freetype::Freetype)

add_executable(colourmaps_crameri colourmaps_crameri.cpp)
target_link_libraries(colourmaps_crameri OpenGL::GL glfw Freetype::Freetype)

add_executable(colourmaps_cet colourmaps_cet.cpp)
target_link_libraries(colourmaps_cet OpenGL::GL glfw Freetype::Freetype)

add_executable(colourmaps_lenthe colourmaps_lenthe.cpp)
target_link_libraries(colourmaps_lenthe OpenGL::GL glfw Freetype::Freetype)

add_executable(colourmaps_matplotlib colourmaps_matplotlib.cpp)
target_link_libraries(colourmaps_matplotlib OpenGL::GL glfw Freetype::Freetype)

if(HAVE_STD_FORMAT)
  add_executable(colourmaps_mono colourmaps_mono.cpp)
  target_link_libraries(colourmaps_mono OpenGL::GL glfw Freetype::Freetype)

  add_executable(colourmaps_hsv1d colourmaps_hsv1d.cpp)
  target_link_libraries(colourmaps_hsv1d OpenGL::GL glfw Freetype::Freetype)
endif()

add_executable(logisticmap logisticmap.cpp)
target_link_libraries(logisticmap OpenGL::GL glfw Freetype::Freetype)

add_executable(quiver quiver.cpp)
target_link_libraries(quiver OpenGL::GL glfw Freetype::Freetype)

add_executable(vectorvis vectorvis.cpp)
target_link_libraries(vectorvis OpenGL::GL glfw Freetype::Freetype)

add_executable(scatter scatter.cpp)
target_link_libraries(scatter OpenGL::GL glfw Freetype::Freetype)

# This sphere example uses constexpr code
if(NOT APPLE)
  add_executable(scatter_ico scatter_ico.cpp)
  target_link_libraries(scatter_ico OpenGL::GL glfw Freetype::Freetype)

  add_executable(scatter_geodesic scatter_geodesic.cpp)
  target_link_libraries(scatter_geodesic OpenGL::GL glfw Freetype::Freetype)
endif()

add_executable(scatter_dynamic scatter_dynamic.cpp)
target_link_libraries(scatter_dynamic OpenGL::GL glfw Freetype::Freetype)

add_executable(duochrome duochrome.cpp)
target_link_libraries(duochrome OpenGL::GL glfw Freetype::Freetype)

add_executable(graph1 graph1.cpp)
target_link_libraries(graph1 OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_line graph_line.cpp)
target_link_libraries(graph_line OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_dynamic_x2 graph_dynamic_x2.cpp)
target_link_libraries(graph_dynamic_x2 OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_fouraxes graph_fouraxes.cpp)
target_link_libraries(graph_fouraxes OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_incoming_data graph_incoming_data.cpp)
target_link_libraries(graph_incoming_data OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_incoming_data_rescale graph_incoming_data_rescale.cpp)
target_link_libraries(graph_incoming_data_rescale OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_twinax graph_twinax.cpp)
target_link_libraries(graph_twinax OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_rightaxis graph_rightaxis.cpp)
target_link_libraries(graph_rightaxis OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_dynamic_sine graph_dynamic_sine.cpp)
target_link_libraries(graph_dynamic_sine OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_dynamic_sine_rescale graph_dynamic_sine_rescale.cpp)
target_link_libraries(graph_dynamic_sine_rescale OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_change_xaxis graph_change_xaxis.cpp)
target_link_libraries(graph_change_xaxis OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_bar graph_bar.cpp)
target_link_libraries(graph_bar OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_histo graph_histo.cpp)
target_link_libraries(graph_histo OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_logist graph_logist.cpp)
target_link_libraries(graph_logist OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_logist2 graph_logist2.cpp)
target_link_libraries(graph_logist2 OpenGL::GL glfw Freetype::Freetype)

add_executable(graph_coords graph_coords.cpp)
target_link_libraries(graph_coords OpenGL::GL glfw Freetype::Freetype)

add_executable(linregr linregr.cpp)
target_link_libraries(linregr OpenGL::GL glfw Freetype::Freetype)

add_executable(twowindows twowindows.cpp)
target_link_libraries(twowindows OpenGL::GL glfw Freetype::Freetype)

# Load the multi context glad header (the one with 'mx')
add_executable(twowindows_mx twowindows.cpp)
target_compile_definitions(twowindows_mx PUBLIC GLAD_OPTION_MX=1)
target_link_libraries(twowindows_mx OpenGL::GL glfw Freetype::Freetype)

add_executable(twowindows_mutex twowindows_mutex.cpp)
target_link_libraries(twowindows_mutex OpenGL::GL glfw Freetype::Freetype)

add_executable(threewindows threewindows.cpp)
target_link_libraries(threewindows OpenGL::GL glfw Freetype::Freetype)

add_executable(rod rod.cpp)
target_link_libraries(rod OpenGL::GL glfw Freetype::Freetype)

add_executable(rod_pan rod_pan.cpp)
target_link_libraries(rod_pan OpenGL::GL glfw Freetype::Freetype)

# This example uses constexpr code
if(NOT APPLE)
  add_executable(grating grating.cpp)
  target_link_libraries(grating OpenGL::GL glfw Freetype::Freetype)
endif()

add_executable(cone cone.cpp)
target_link_libraries(cone OpenGL::GL glfw Freetype::Freetype)

add_executable(ring ring.cpp)
target_link_libraries(ring OpenGL::GL glfw Freetype::Freetype)

# This sphere example uses constexpr code
if(NOT APPLE)
  add_executable(sphere sphere.cpp)
  target_link_libraries(sphere OpenGL::GL glfw Freetype::Freetype)
endif()

# constexpr code won't work on Apple Clang
if(NOT APPLE)
  add_executable(icosahedron icosahedron.cpp)
  target_link_libraries(icosahedron OpenGL::GL glfw Freetype::Freetype)

  add_executable(testce testce.cpp)

  add_executable(geodesic geodesic.cpp)
  target_link_libraries(geodesic OpenGL::GL glfw Freetype::Freetype)

  add_executable(geodesic_ce geodesic_ce.cpp)
  target_link_libraries(geodesic_ce OpenGL::GL glfw Freetype::Freetype)
endif()

add_executable(tri tri.cpp)
target_link_libraries(tri OpenGL::GL glfw Freetype::Freetype)

add_executable(draw_triangles draw_triangles.cpp)
target_link_libraries(draw_triangles OpenGL::GL glfw Freetype::Freetype)

add_executable(voronoi_random voronoi_random.cpp)
target_link_libraries(voronoi_random OpenGL::GL glfw Freetype::Freetype)

add_executable(voronoi_rectangular voronoi_rectangular.cpp)
target_link_libraries(voronoi_rectangular OpenGL::GL glfw Freetype::Freetype)

add_executable(voronoi_vectordata voronoi_vectordata.cpp)
target_link_libraries(voronoi_vectordata OpenGL::GL glfw Freetype::Freetype)

add_executable(voronoi_function voronoi_function.cpp)
target_link_libraries(voronoi_function OpenGL::GL glfw Freetype::Freetype)

add_executable(voronoi_function_flat voronoi_function_flat.cpp)
target_link_libraries(voronoi_function_flat OpenGL::GL glfw Freetype::Freetype)

add_executable(voronoi_fixed voronoi_fixed.cpp)
target_link_libraries(voronoi_fixed OpenGL::GL glfw Freetype::Freetype)

add_executable(voronoi_fixed_xz voronoi_fixed_xz.cpp)
target_link_libraries(voronoi_fixed_xz OpenGL::GL glfw Freetype::Freetype)

add_executable(voronoi_fixed_nearlyz voronoi_fixed_nearlyz.cpp)
target_link_libraries(voronoi_fixed_nearlyz OpenGL::GL glfw Freetype::Freetype)

add_executable(rectangle rectangle.cpp)
target_link_libraries(rectangle OpenGL::GL glfw Freetype::Freetype)

add_executable(rhombo rhombo.cpp)
target_link_libraries(rhombo OpenGL::GL glfw Freetype::Freetype)

add_executable(rhombo_scene rhombo_scene.cpp)
target_link_libraries(rhombo_scene OpenGL::GL glfw Freetype::Freetype)

add_executable(quads quads.cpp)
target_link_libraries(quads OpenGL::GL glfw Freetype::Freetype)

add_executable(quadsmesh quads.cpp)
target_compile_definitions(quadsmesh PUBLIC MESH=1)
target_link_libraries(quadsmesh OpenGL::GL glfw Freetype::Freetype)

add_executable(pointrows pointrows.cpp)
target_link_libraries(pointrows OpenGL::GL glfw Freetype::Freetype)

add_executable(pointrows_mesh pointrows.cpp)
target_compile_definitions(pointrows_mesh PUBLIC MESH=1)
target_link_libraries(pointrows_mesh OpenGL::GL glfw Freetype::Freetype)

add_executable(jsonconfig jsonconfig.cpp)
target_link_libraries(jsonconfig)

if(HDF5_FOUND)
  add_executable(hdfdata hdfdata.cpp)
  target_link_libraries(hdfdata ${HDF5_C_LIBRARIES})
endif(HDF5_FOUND)

add_executable(randvec randvec.cpp)
target_link_libraries(randvec OpenGL::GL glfw Freetype::Freetype)

# Demo of morph's bootstrap implementation
add_executable(bootstrap bootstrap.cpp)
target_link_libraries(bootstrap OpenGL::GL glfw Freetype::Freetype)

add_executable(rng rng.cpp)
target_link_libraries(rng)

add_executable(scale scale.cpp)

add_executable(vvec vvec.cpp)

add_executable(vvec_convolve vvec_convolve.cpp)
target_link_libraries(vvec_convolve OpenGL::GL glfw Freetype::Freetype)

add_executable(vvec_gauss vvec_gauss.cpp)
target_link_libraries(vvec_gauss OpenGL::GL glfw Freetype::Freetype)


add_executable(vvec_rgb_gaussians vvec_rgb_gaussians.cpp)
target_link_libraries(vvec_rgb_gaussians OpenGL::GL glfw Freetype::Freetype)

add_executable(vvec_smoothgauss vvec_smoothgauss.cpp)
target_link_libraries(vvec_smoothgauss OpenGL::GL glfw Freetype::Freetype)

add_executable(quit_callback quit_callback.cpp)
target_link_libraries(quit_callback OpenGL::GL glfw Freetype::Freetype)

add_executable(izhikevich izhikevich.cpp)
target_link_libraries(izhikevich OpenGL::GL glfw Freetype::Freetype)

add_executable(izhikevich_alt izhikevich_alt.cpp)
target_link_libraries(izhikevich_alt OpenGL::GL glfw Freetype::Freetype)

add_executable(curvytelly curvytelly.cpp)
target_link_libraries(curvytelly OpenGL::GL glfw Freetype::Freetype)

add_executable(curvytelly_pipe curvytelly_pipe.cpp)
target_link_libraries(curvytelly_pipe OpenGL::GL glfw Freetype::Freetype)

add_executable(curvytelly_chequered_pipe curvytelly_chequered_pipe.cpp)
target_link_libraries(curvytelly_chequered_pipe OpenGL::GL glfw Freetype::Freetype)

add_executable(lighting_test lighting_test.cpp)
target_link_libraries(lighting_test OpenGL::GL glfw Freetype::Freetype)

add_executable(healpix healpix.cpp)
target_link_libraries(healpix OpenGL::GL glfw Freetype::Freetype)

add_executable(lines lines.cpp)
target_link_libraries(lines OpenGL::GL glfw Freetype::Freetype)

add_executable(line line.cpp)
target_link_libraries(line OpenGL::GL glfw Freetype::Freetype)
