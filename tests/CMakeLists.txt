# CTests

# All #includes in test programs have to be #include "morph/header.h"
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

# BezCurve class
set(TARGETTEST1 testbez)
set(SOURCETEST1 testbez.cpp)
add_executable(${TARGETTEST1} ${SOURCETEST1})
target_link_libraries(${TARGETTEST1} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES})
add_test(testbez ${TARGETTEST1})

# BezCurve class
set(TARGETTEST1_1 testbez2)
set(SOURCETEST1_1 testbez2.cpp)
add_executable(${TARGETTEST1_1} ${SOURCETEST1_1})
target_link_libraries(${TARGETTEST1_1} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES})
add_test(testbez2 ${TARGETTEST1_1})

# BezCurvePath class
set(TARGETTEST1_2 testbezcurves)
set(SOURCETEST1_2 testbezcurves.cpp)
add_executable(${TARGETTEST1_2} ${SOURCETEST1_2})
target_link_libraries(${TARGETTEST1_2} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES})
add_test(testbezcurves ${TARGETTEST1_2})

# Testing matrix representation of Bezier curves
set(TARGETTEST1_3 testbezmatrix)
set(SOURCETEST1_3 testbezmatrix.cpp)
add_executable(${TARGETTEST1_3} ${SOURCETEST1_3})
# Disable warning -Wunused-but-set-variable:
#target_compile_options(${TARGETTEST1_3} PRIVATE -Wno-unused-but-set-variable)
target_compile_definitions(${TARGETTEST1_3} PUBLIC FLT=float)
target_link_libraries(${TARGETTEST1_3} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES})
add_test(testbezmatrix ${TARGETTEST1_3})

# Testing auto-fitting a Bezier curve
set(TARGETTEST1_4 testbezfit)
set(SOURCETEST1_4 testbezfit.cpp)
add_executable(${TARGETTEST1_4} ${SOURCETEST1_4})
target_compile_definitions(${TARGETTEST1_4} PUBLIC FLT=float)
target_link_libraries(${TARGETTEST1_4} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES})
add_test(testbezfit ${TARGETTEST1_4})

# Testing splitting of Bezier curves
set(TARGETTEST1_5 testbezsplit)
set(SOURCETEST1_5 testbezsplit.cpp)
add_executable(${TARGETTEST1_5} ${SOURCETEST1_5})
target_compile_definitions(${TARGETTEST1_5} PUBLIC FLT=float)
target_link_libraries(${TARGETTEST1_5} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES})
add_test(testbezsplit ${TARGETTEST1_5})

if(${OpenCV_FOUND})
  # Testing Bezier derivatives (though really testing curve joining)
  set(TARGETTEST1_6 testbezderiv)
  set(SOURCETEST1_6 testbezderiv.cpp)
  add_executable(${TARGETTEST1_6} ${SOURCETEST1_6})
  target_compile_definitions(${TARGETTEST1_6} PUBLIC FLT=double)
  target_link_libraries(${TARGETTEST1_6} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${OpenCV_LIBS})
  add_test(testbezderiv ${TARGETTEST1_6})

  # Testing Bezier derivatives
  set(TARGETTEST1_7 testbezderiv2)
  set(SOURCETEST1_7 testbezderiv2.cpp)
  add_executable(${TARGETTEST1_7} ${SOURCETEST1_7})
  target_compile_definitions(${TARGETTEST1_7} PUBLIC FLT=double)
  target_link_libraries(${TARGETTEST1_7} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${OpenCV_LIBS})
  add_test(testbezderiv2 ${TARGETTEST1_7})

  # Testing Bezier derivatives
  set(TARGETTEST1_8 testbezderiv3)
  set(SOURCETEST1_8 testbezderiv3.cpp)
  add_executable(${TARGETTEST1_8} ${SOURCETEST1_8})
  target_compile_definitions(${TARGETTEST1_8} PUBLIC FLT=double)
  target_link_libraries(${TARGETTEST1_8} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${OpenCV_LIBS})
  add_test(testbezderiv3 ${TARGETTEST1_8})
endif()

# Test two curves connected together
set(TARGETTEST2 twocurves)
set(SOURCETEST2 twocurves.cpp)
add_executable(${TARGETTEST2} ${SOURCETEST2})
target_link_libraries(${TARGETTEST2} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES})
add_test(twocurves ${TARGETTEST2})

# Test reading trial.svg
set(TARGETTEST3 testreadcurves)
set(SOURCETEST3 testreadcurves.cpp)
add_executable(${TARGETTEST3} ${SOURCETEST3})
target_link_libraries(${TARGETTEST3} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES})
add_test(testreadcurves ${TARGETTEST3})

# Test reading whiskerbarrels_withcentres.svg
set(TARGETTEST31 testreadcurves_circles)
set(SOURCETEST31 testreadcurves_circles.cpp)
add_executable(${TARGETTEST31} ${SOURCETEST31})
target_link_libraries(${TARGETTEST31} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES})
add_test(testreadcurves_circles ${TARGETTEST31})

# Test hexgrid
set(TARGETTEST5 testhexgrid)
set(SOURCETEST5 testhexgrid.cpp)
add_executable(${TARGETTEST5} ${SOURCETEST5})
add_test(testhexgrid ${TARGETTEST5})

# Test hexgrid2
add_executable(testhexgrid2 testhexgrid2.cpp)
target_link_libraries(testhexgrid2 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES})
add_test(testhexgrid2 testhexgrid2)

# Test distance to boundary
add_executable(testhexbounddist testhexbounddist.cpp)
target_link_libraries(testhexbounddist ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES})
add_test(testhexbounddist testhexbounddist)

# Test HDF file access
add_executable(testhdfdata1 testhdfdata1.cpp)
# Note how if you only use HDF, you only have to link to libhdf
target_link_libraries(testhdfdata1 ${HDF5_C_LIBRARIES})
add_test(testhdfdata1 testhdfdata1)

# More HDF data tests
add_executable(testhdfdata2 testhdfdata2.cpp)
target_link_libraries(testhdfdata2 ${HDF5_C_LIBRARIES})
add_test(testhdfdata2 testhdfdata2)

add_executable(testhdfdata3f testhdfdata3.cpp)
target_compile_definitions(testhdfdata3f PUBLIC FLT=float)
target_link_libraries(testhdfdata3f ${HDF5_C_LIBRARIES})
add_test(testhdfdata3f testhdfdata3f)

add_executable(testhdfdata3d testhdfdata3.cpp)
target_compile_definitions(testhdfdata3d PUBLIC FLT=float)
target_link_libraries(testhdfdata3d ${HDF5_C_LIBRARIES})
add_test(testhdfdata3d testhdfdata3d)

add_executable(testhdfdata4f testhdfdata4.cpp)
target_compile_definitions(testhdfdata4f PUBLIC FLT=float)
target_link_libraries(testhdfdata4f ${HDF5_C_LIBRARIES})
add_test(testhdfdata4f testhdfdata4f)

if(${glfw3_FOUND})
  # Test hexgrid3 (hexgrid2 with visualisation)
  add_executable(testhexgrid3 testhexgrid3.cpp)
  target_link_libraries(testhexgrid3 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(testhexgrid3 GLEW::GLEW)
  endif()

  # Test hexgridsave (HexGrid's save feature)
  add_executable(testhexgridsave testhexgridsave.cpp)
  target_link_libraries(testhexgridsave ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${HDF5_C_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(testhexgridsave GLEW::GLEW)
  endif()

  # Test boundary pgram
  add_executable(testdom_pgram testdom_pgram.cpp)
  target_link_libraries(testdom_pgram ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(testdom_pgram GLEW::GLEW)
  endif()

  # Test boundary rect
  add_executable(testdom_rect testdom_rect.cpp)
  target_link_libraries(testdom_rect ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(testdom_rect GLEW::GLEW)
  endif()

  # Test boundary hex
  add_executable(testdom_hex testdom_hex.cpp)
  target_link_libraries(testdom_hex ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(testdom_hex GLEW::GLEW)
  endif()

  # Test boundary bound
  add_executable(testdom_bound testdom_bound.cpp)
  target_link_libraries(testdom_bound ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(testdom_bound GLEW::GLEW)
  endif()

  add_executable(testbighexgrid testbighexgrid.cpp)
  target_link_libraries(testbighexgrid ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(testbighexgrid GLEW::GLEW)
  endif()

  add_executable(testVisCoordArrows testVisCoordArrows.cpp)
  target_link_libraries(testVisCoordArrows ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(testVisCoordArrows GLEW::GLEW)
  endif()

  add_executable(testVisRemoveModel testVisRemoveModel.cpp)
  target_link_libraries(testVisRemoveModel ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(testVisRemoveModel GLEW::GLEW)
  endif()

  # Test elliptical HexGrid code (visualized with morph::Visual)
  add_executable(test_ellipseboundary test_ellipseboundary.cpp)
  target_link_libraries(test_ellipseboundary ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(test_ellipseboundary GLEW::GLEW)
  endif()

  # Test circular HexGrid code
  add_executable(test_circleboundary test_circleboundary.cpp)
  target_link_libraries(test_circleboundary ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(test_circleboundary GLEW::GLEW)
  endif()

  # Test Dirichlet code
  add_executable(testDirichlet testDirichlet.cpp)
  target_link_libraries(testDirichlet ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(testDirichlet GLEW::GLEW)
  endif()

  add_executable(testDirichlet2 testDirichlet2.cpp)
  target_link_libraries(testDirichlet2 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(testDirichlet2 GLEW::GLEW)
  endif()

  add_executable(testDirichlet4 testDirichlet4.cpp)
  target_link_libraries(testDirichlet4 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(testDirichlet4 GLEW::GLEW)
  endif()

  add_executable(testDirichlet5 testDirichlet5.cpp)
  target_link_libraries(testDirichlet5 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype jsoncpp_lib)
  if(USE_GLEW)
    target_link_libraries(testDirichlet5 GLEW::GLEW)
  endif()
endif()

# Test morph::Process class
if(APPLE)
  message("-- NB: Omitting testProcess.cpp on Mac for now, as it doesn't work.")
elseif(WIN32)
  message("-- NB: Omitting testProcess.cpp on Windows for now, as it tests the very Unixy Process class.")  
else(APPLE)
  add_executable(testProcess testProcess.cpp)
  add_test(testProcess testProcess)
endif(APPLE)

# Test morph::Config class
add_executable(testConfig testConfig.cpp)
target_link_libraries(testConfig jsoncpp_lib)
add_test(testConfig testConfig)

# Test morph::Quaternion
add_executable(testQuaternion testQuaternion.cpp)
add_test(testQuaternion testQuaternion)

# testVector.
add_executable(testVector testVector.cpp)
add_test(testVector testVector)

# testVector elemental operations
add_executable(testVectorElementOps testVectorElementOps.cpp)
add_test(testVectorElementOps testVectorElementOps)

# Test the "Vector made from an std::vector" code (this vVector is better for neural networks)
add_executable(testvVector testvVector.cpp)
add_test(testvVector testvVector)

add_executable(testvVectorOfVectors testvVectorOfVectors.cpp)
add_test(testvVectorOfVectors testvVectorOfVectors)

# It's possible to modify testVector.cpp to be c++-11 or c++-14 friendly:
add_executable(testVector14 testVector14.cpp)
target_compile_features(testVector14 PUBLIC cxx_std_14)
add_test(testVector14 testVector14)
add_executable(testVector11 testVector14.cpp)
target_compile_features(testVector11 PUBLIC cxx_std_11)
add_test(testVector11 testVector11)

# Test the Scaling of vectors
add_executable(testScaleVector testScaleVector.cpp)
add_test(testScaleVector testScaleVector)

# Test morph::TransformMatrix (4x4 matrix)
add_executable(testTransformMatrix testTransformMatrix.cpp)
add_test(testTransformMatrix testTransformMatrix)

# Test morph::Matrix33 (3x3 matrix)
add_executable(testMatrix33 testMatrix33.cpp)
add_test(testMatrix33 testMatrix33)

# Test Hex::get_vertex_coord()
add_executable(testHexVertexPos testHexVertexPos.cpp)
add_test(testHexVertexPos testHexVertexPos)

# Test Hex user flags code
add_executable(testHexUserFlags testHexUserFlags.cpp)
add_test(testHexUserFlags testHexUserFlags)

# Test MathAlgo code
add_executable(testMathAlgo testMathAlgo.cpp)
add_test(testMathAlgo testMathAlgo)

add_executable(testMathAlgo2 testMathAlgo2.cpp)
add_test(testMathAlgo2 testMathAlgo2)

# Test the scaling code
add_executable(testScale testScale.cpp)
add_test(testScale testScale)

# Test the colour mapping
add_executable(testColourMap testColourMap.cpp)
add_test(testColourMap testColourMap)

# Test Nelder Mead algorithm
add_executable(testNMSimplex testNMSimplex.cpp)
target_compile_definitions(testNMSimplex PUBLIC FLT=float)
add_test(testNMSimplex testNMSimplex)

# Test Random number generation code
add_executable(testRandom testRandom.cpp)
add_test(testRandom testRandom)

# Test winding number code
add_executable(testWinder testWinder.cpp)
target_link_libraries(testWinder)
add_test(testWinder testWinder)

# Test RandString generation
add_executable(testRandString testRandString.cpp)
add_test(testRandString testRandString)

# Neural nets

# Test morph::nn::ElmanNet
add_executable(test_elman test_elman.cpp)
add_test(test_elman test_elman)

add_executable(ff_debug ff_debug.cpp)
add_test(ff_debug ff_debug)

#
# Boolean gene nets. Fixme: These are not unit tests, but I've thrown
# them in here for now. Perhaps need a 'bn' directory to build these
# programs.
#

add_executable(testGeneNet testGeneNet.cpp)
if (APPLE)
  target_compile_options(testGeneNet PUBLIC "-mavx")
endif()
#add_test(testGeneNet testGeneNet )

add_executable(testGeneNetKeqN testGeneNetKeqN.cpp)
if (APPLE)
  target_compile_options(testGeneNetKeqN PUBLIC "-mavx")
endif()
#add_test(testGeneNetKeqN testGeneNetKeqN)

add_executable(testGeneNetKeqNm1 testGeneNetKeqNm1.cpp)
if (APPLE)
  target_compile_options(testGeneNetKeqNm1 PUBLIC "-mavx")
endif()
#add_test(testGeneNetKeqNm1 testGeneNetKeqNm1)

add_executable(testEvolveOnegen testEvolveOnegen.cpp)
if (APPLE)
  target_compile_options(testEvolveOnegen PUBLIC "-mavx")
endif()
#add_test(testEvolveOnegen testEvolveOnegen)

add_executable(testEvolveFit testEvolveFit.cpp)
if (APPLE)
  target_compile_options(testEvolveFit PUBLIC "-mavx")
endif()
#add_test(testEvolveFit testEvolveFit)

add_executable(testEvolve testEvolve.cpp)
if (APPLE)
  target_compile_options(testEvolve PUBLIC "-mavx")
endif()
target_link_libraries(testEvolve jsoncpp_lib)

if(NOT WIN32)
  # testGradGenome tries to create random num generator with width <
  # 16 bits, not strictly allowed and enforced by VS2019
  add_executable(testGradGenome testGradGenome.cpp)
  if (APPLE)
    target_compile_options(testGradGenome PUBLIC "-mavx")
  endif()
  #add_test(testGradGenome testGradGenome )
endif()

# Test Cartesian (rectangular) grid
add_executable(testcartgrid testcartgrid.cpp)
add_test(testcartgrid testcartgrid)
