#
# This file controls the installation of the header files.
#

# Header installation (list from ls *.h* -1)
install(
  FILES
  AllocAndRead.h
  Anneal.h
  base64.h
  BezCoord.h
  BezCurve.h
  BezCurvePath.h
  bootstrap.h
  CartGrid.h
  CartGridVisual.h
  ColourBarVisual.h
  colour.h
  ColourMap.h
  ColourMap_Lists.h
  colourmaps_cet.h
  colourmaps_crameri.h
  ConeVisual.h
  Config.h
  ConfigVisual.h
  constexpr_math.h
  CoordArrows.h
  crc32.h
  CurvyTellyVisual.h
  CyclicColourVisual.h
  DatasetStyle.h
  debug.h
  DirichDom.h
  DirichVtx.h
  flags.h
  GeodesicVisualCE.h
  GeodesicVisual.h
  geometry.h
  graphstyles.h
  GraphVisual.h
  GratingVisual.h
  Gridct.h
  GridctVisual.h
  GridFeatures.h
  Grid.h
  GridVisual.h
  HdfData.h
  HealpixVisual.h
  HexGrid.h
  HexGridVisual.h
  Hex.h
  hexyhisto.h
  histo.h
  HSVWheelVisual.h
  IcosaVisual.h
  keys.h
  LengthscaleVisual.h
  lenthe_colormap.hpp
  loadpng.h
  lodepng.h
  mat22.h
  mat33.h
  mat44.h
  MathAlgo.h
  mathconst.h
  math.h
  MathImpl.h
  Mnist.h
  MorphDbg.h
  NM_Simplex.h
  PointRowsMeshVisual.h
  PointRowsVisual.h
  PolygonVisual.h
  Process.h
  QuadsMeshVisual.h
  QuadsVisual.h
  quaternion.h
  QuiverVisual.h
  Random.h
  range.h
  RD_Base.h
  ReadCurves.h
  RectangleVisual.h
  Rect.h
  RhomboVisual.h
  RingVisual.h
  rngd.h
  rng.h
  rngs.h
  RodVisual.h
  scale.h
  ScatterVisual.h
  ShapeAnalysis.h
  SphereVisual.h
  TextFeatures.h
  TextGeometry.h
  tools.h
  trait_tests.h
  TriangleVisual.h
  TriaxesVisual.h
  TriFrameVisual.h
  TxtVisual.h
  unicode.h
  vec.h
  VectorVisual.h
  version.h
  VisualCommon.h
  VisualCompoundRay.h
  VisualDataModel.h
  VisualDefaultShaders.h
  VisualFace.h
  Visual.h
  VisualModel.h
  VisualResources.h
  VisualTextModel.h
  VoronoiVisual.h
  vvec.h
  Winder.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph
)

if(WIN32)
  # On windows we need morph/fonts/verafonts.h
  add_subdirectory(fonts)
endif()

# There are also headers in sub directories
add_subdirectory(nn) # 'nn' for neural network code
add_subdirectory(bn) # 'bn' for boolean net code
add_subdirectory(gl) # GL common and compute code
add_subdirectory(glad) # GLAD header library
add_subdirectory(healpix) # Public, but slightly modified HEALPix headers

# And the jcvoronoi code
add_subdirectory(jcvoronoi)

# The Visual-in-a-Qt-Widget code
add_subdirectory(qt)
# WxWidgets code
add_subdirectory(wx)

# Install the EXPORT so that morphologica has its own .cmake file and find_package(morphologica) should work
install(FILES morphologica-config.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/morphologica)
#install(EXPORT morphologica DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/morphologica)
