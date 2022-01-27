#
# This file controls the installation of the header files.
#

# Header installation
install(
  FILES Quaternion.h tools.h BezCoord.h BezCurve.h BezCurvePath.h ReadCurves.h AllocAndRead.h MorphDbg.h MathConst.h MathAlgo.h MathImpl.h number_type.h Hex.h HexGrid.h HdfData.h Process.h RD_Base.h DirichVtx.h DirichDom.h ShapeAnalysis.h NM_Simplex.h Anneal.h Config.h Vector.h vVector.h TransformMatrix.h colour.h ColourMap.h ColourMap_Lists.h Scale.h Random.h RecurrentNetworkTools.h RecurrentNetwork.h Winder.h expression_sfinae.h
Mnist.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph
  )
# There are also headers in sub directories
add_subdirectory(nn) # 'nn' for neural network code

# And the nlohmann/json.hpp header
add_subdirectory(nlohmann)

if (${glfw3_FOUND})
  install( FILES VisualCommon.h Visual.h lodepng.h VisualModel.h VisualDataModel.h VisualTextModel.h VisualResources.h VisualFace.h unicode.h CoordArrows.h HexGridVisual.h CartGridVisual.h QuadsVisual.h GraphVisual.h histo.h PointRowsVisual.h ScatterVisual.h QuiverVisual.h RodVisual.h PolygonVisual.h VisualDefaultShaders.h RecurrentNetworkModel.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph)
endif()

# Install the EXPORT so that morphologica has its own .cmake file and find_package(morphologica) should work
install(FILES morphologica-config.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/morphologica)
#install(EXPORT morphologica DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/morphologica)
