#
# This file controls the installation of the header files.
#

# Header installation (list from ls *.h* -1, then re-ordered into functionality groups)
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
  colour.h
  ColourMap.h
  ColourMap_Lists.h
  colourmaps_cet.h
  colourmaps_crameri.h
  Config.h
  constexpr_math.h
  crc32.h
  debug.h
  DirichDom.h
  DirichVtx.h
  flags.h
  geometry.h
  Gridct.h
  GridFeatures.h
  Grid.h
  HdfData.h
  HexGrid.h
  Hex.h
  hexyhisto.h
  histo.h
  keys.h
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
  Process.h
  quaternion.h
  Random.h
  range.h
  RD_Base.h
  ReadCurves.h
  Rect.h
  rngd.h
  rng.h
  rngs.h
  scale.h
  ShapeAnalysis.h
  tools.h
  trait_tests.h
  unicode.h
  vec.h
  version.h
  vvec.h
  Winder.h

  DatasetStyle.h
  graphstyles.h
  TextFeatures.h
  TextGeometry.h

  VisualCommon.h
  VisualFont.h
  VisualDefaultShaders.h

  VisualFaceBase.h
  VisualFace.h
  VisualFaceMX.h

  VisualResourcesBase.h
  VisualResources.h
  VisualResourcesMX.h

  VisualGlfw.h

  VisualBase.h
  VisualOwnable.h
  VisualOwnableMX.h
  VisualNoMX.h
  VisualMX.h
  Visual.h
  VisualCompoundRay.h

  VisualModelBase.h
  VisualModelImpl.h
  VisualModelImplMX.h
  VisualModel.h

  VisualDataModel.h

  VisualTextModelBase.h
  VisualTextModelImpl.h
  VisualTextModelImplMX.h
  VisualTextModel.h

  CartGridVisual.h
  ColourBarVisual.h
  ConeVisual.h
  ConfigVisual.h
  CoordArrows.h
  CurvyTellyVisual.h
  CyclicColourVisual.h
  GeodesicVisualCE.h
  GeodesicVisual.h
  GraphVisual.h
  GratingVisual.h
  GridctVisual.h
  GridVisual.h
  HealpixVisual.h
  HexGridVisual.h
  HSVWheelVisual.h
  IcosaVisual.h
  LengthscaleVisual.h
  PointRowsMeshVisual.h
  PointRowsVisual.h
  PolygonVisual.h
  QuadsMeshVisual.h
  QuadsVisual.h
  QuiverVisual.h
  RectangleVisual.h
  RhomboVisual.h
  RingVisual.h
  RodVisual.h
  ScatterVisual.h
  SphereVisual.h
  TriangleVisual.h
  TriaxesVisual.h
  TriFrameVisual.h
  TxtVisual.h
  VectorVisual.h
  VoronoiVisual.h

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
# wxWindows
add_subdirectory(wx)

# Install the EXPORT so that morphologica has its own .cmake file and find_package(morphologica) should work
install(FILES morphologica-config.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/morphologica)
#install(EXPORT morphologica DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/morphologica)
