# CTests

# All #includes in test programs have to be #include "morph/header.h"
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

if(ARMADILLO_FOUND)
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

  # Testing Bezier derivatives
  if(${glfw3_FOUND})
    set(TARGETTEST1_8 testbezderiv3)
    set(SOURCETEST1_8 testbezderiv3.cpp)
    add_executable(${TARGETTEST1_8} ${SOURCETEST1_8})
    target_compile_definitions(${TARGETTEST1_8} PUBLIC FLT=double)
    target_link_libraries(${TARGETTEST1_8} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype)
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
endif(ARMADILLO_FOUND)

if(HDF5_FOUND)
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

endif(HDF5_FOUND)

if(${glfw3_FOUND})
  if(ARMADILLO_FOUND)
    # Test hexgrid3 (hexgrid2 with visualisation)
    add_executable(testhexgrid3 testhexgrid3.cpp)
    target_link_libraries(testhexgrid3 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype)

    if(HDF5_FOUND)
      # Test hexgridsave (HexGrid's save feature)
      add_executable(testhexgridsave testhexgridsave.cpp)
      target_link_libraries(testhexgridsave ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${HDF5_C_LIBRARIES} OpenGL::GL glfw Freetype::Freetype)
    endif(HDF5_FOUND)

    add_executable(testbighexgrid testbighexgrid.cpp)
    target_link_libraries(testbighexgrid OpenGL::GL glfw Freetype::Freetype)
  endif(ARMADILLO_FOUND)

  add_executable(testVisCoordArrows testVisCoordArrows.cpp)
  target_link_libraries(testVisCoordArrows OpenGL::GL glfw Freetype::Freetype)

  add_executable(testVisRemoveModel testVisRemoveModel.cpp)
  target_link_libraries(testVisRemoveModel OpenGL::GL glfw Freetype::Freetype)

  if(ARMADILLO_FOUND)
    # Test elliptical HexGrid code (visualized with morph::Visual)
    add_executable(test_ellipseboundary test_ellipseboundary.cpp)
    target_link_libraries(test_ellipseboundary OpenGL::GL glfw Freetype::Freetype)

    # Test circular HexGrid code
    add_executable(test_circleboundary test_circleboundary.cpp)
    target_link_libraries(test_circleboundary OpenGL::GL glfw Freetype::Freetype)

    if(HDF5_FOUND)
      # Test Dirichlet code
      add_executable(testDirichlet testDirichlet.cpp)
      target_link_libraries(testDirichlet OpenGL::GL glfw Freetype::Freetype)

      add_executable(testDirichlet2 testDirichlet2.cpp)
      target_link_libraries(testDirichlet2 OpenGL::GL glfw Freetype::Freetype)

      add_executable(testDirichlet4 testDirichlet4.cpp)
      target_link_libraries(testDirichlet4 OpenGL::GL glfw Freetype::Freetype)

      add_executable(testDirichlet5 testDirichlet5.cpp)
      target_link_libraries(testDirichlet5 OpenGL::GL glfw Freetype::Freetype)
    endif(HDF5_FOUND)
  endif(ARMADILLO_FOUND)

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
add_test(testConfig testConfig)

# Test morph::quaternion
add_executable(testQuaternion testQuaternion.cpp)
add_test(testQuaternion testQuaternion)

# Test quaternion and mat44 rotations in float and double precision
add_executable(testRotations_float testRotations.cpp)
target_compile_definitions(testRotations_float PUBLIC FLT=float)
add_test(testRotations_float testRotations_float)

add_executable(testRotations_double testRotations.cpp)
target_compile_definitions(testRotations_double PUBLIC FLT=double)
add_test(testRotations_double testRotations_double)

add_executable(testRotationsQuat_float testRotationsQuat.cpp)
target_compile_definitions(testRotationsQuat_float PUBLIC FLT=float)
add_test(testRotationsQuat_float testRotationsQuat_float)

add_executable(testRotationsQuat_double testRotationsQuat.cpp)
target_compile_definitions(testRotationsQuat_double PUBLIC FLT=double)
add_test(testRotationsQuat_double testRotationsQuat_double)

if(NOT APPLE)
  add_executable(testQuat_constexpr testQuat_constexpr.cpp)
  add_test(testQuat_constexpr testQuat_constexpr)
endif()

# test morph::vec.
add_executable(testvec testvec.cpp)
add_test(testvec testvec)

if(NOT APPLE)
  # constexpr functionality in morph::vec
  # Need C++20 for the way I use constexpr here so that it is legal to
  # have "a definition of a variable for which no initialization is
  # performed".
  add_executable(testvec_constexpr testvec_constexpr.cpp)
  add_test(testvec_constexpr testvec_constexpr)
endif()

add_executable(testvec_casting testvec_casting.cpp)
add_test(testvec_casting testvec_casting)

add_executable(testvec_asmapkey testvec_asmapkey.cpp)
add_test(testvec_asmapkey testvec_asmapkey)

add_executable(testvec_asunordmapkey testvec_asunordmapkey.cpp)
add_test(testvec_asunordmapkey testvec_asunordmapkey)

add_executable(testarray_asmapkey testarray_asmapkey.cpp)
add_test(testarray_asmapkey testarray_asmapkey)

# test morph::vec elemental operations
add_executable(testvecElementOps testvecElementOps.cpp)
add_test(testvecElementOps testvecElementOps)

# Test the "mathematic vector made from an std::vector" code (this vvec is better for neural networks)
add_executable(testvvec testvvec.cpp)
add_test(testvvec testvvec)

add_executable(testvvecofvecs testvvecofvecs.cpp)
add_test(testvvecofvecs testvvecofvecs)

add_executable(testvvec_operatormult testvvec_operatormult.cpp)
add_test(testvvec_operatormult testvvec_operatormult)

add_executable(testvvec_operatordiv testvvec_operatordiv.cpp)
add_test(testvvec_operatordiv testvvec_operatordiv)

add_executable(testvvec_operatoradd testvvec_operatoradd.cpp)
add_test(testvvec_operatoradd testvvec_operatoradd)

add_executable(testvvec_convolutions testvvec_convolutions.cpp)
add_test(testvvec_convolutions testvvec_convolutions)

add_executable(testvvec_differentiation testvvec_differentiation.cpp)
add_test(testvvec_differentiation testvvec_differentiation)

add_executable(testvvec_zerocross testvvec_zerocross.cpp)
add_test(testvvec_zerocross testvvec_zerocross)

add_executable(testvvec_prune testvvec_prune.cpp)
add_test(testvvec_prune testvvec_prune)

add_executable(testvvec_threshold testvvec_threshold.cpp)
add_test(testvvec_threshold testvvec_threshold)

add_executable(testvvec_elementcompare testvvec_elementcompare.cpp)
add_test(testvvec_elementcompare testvvec_elementcompare)

add_executable(testvvec_rescale testvvec_rescale.cpp)
add_test(testvvec_rescale testvvec_rescale)

add_executable(testvvec_set_from testvvec_set_from.cpp)
add_test(testvvec_set_from testvvec_set_from)

add_executable(testvvec_at_signed testvvec_at_signed.cpp)
add_test(testvvec_at_signed testvvec_at_signed)

add_executable(test_trait_tests test_trait_tests.cpp)
add_test(test_trait_tests test_trait_tests)

add_executable(testarange testarange.cpp)
add_test(testarange testarange)

# It's possible to modify testvec.cpp to be c++-11 or c++-14 friendly:
add_executable(testvec14 testvec14.cpp)
target_compile_features(testvec14 PUBLIC cxx_std_14)
add_test(testvec14 testvec14)
add_executable(testvec11 testvec14.cpp)
target_compile_features(testvec11 PUBLIC cxx_std_11)
add_test(testvec11 testvec11)

# Test the Scaling of vecs
add_executable(testScaleVector testScaleVector.cpp)
add_test(testScaleVector testScaleVector)

# Test scaling complex numbers
add_executable(testScale_complex testScale_complex.cpp)
add_test(testScale_complex testScale_complex)

# Test morph::TransformMatrix (4x4 matrix)
add_executable(testTransformMatrix testTransformMatrix.cpp)
add_test(testTransformMatrix testTransformMatrix)

if(NOT APPLE)
  # constexpr TransformMatrix
  add_executable(testTransMat_constexpr testTransMat_constexpr.cpp)
  add_test(testTransMat_constexpr testTransMat_constexpr)
endif()

# Test morph::Matrix33 (3x3 matrix)
add_executable(testMatrix33 testMatrix33.cpp)
add_test(testMatrix33 testMatrix33)

# Test morph::Matrix22 (2x2 matrix)
add_executable(testMatrix22 testMatrix22.cpp)
add_test(testMatrix22 testMatrix22)

# Test Hex::get_vertex_coord()
add_executable(testHexVertexPos testHexVertexPos.cpp)
add_test(testHexVertexPos testHexVertexPos)

# Test Hex user flags code
add_executable(testHexUserFlags testHexUserFlags.cpp)
add_test(testHexUserFlags testHexUserFlags)

# An example program which is slow compiled on Visual Studio with the /MDd "Debug build" flag
add_executable(profileListHexminErase profileListHexminErase.cpp)
add_test(profileListHexminErase profileListHexminErase)

# Compile with stdc++ lib debug mode (but don't run as a test, as it's
# very slow) This is equivalent to the default debug compilation that
# Visual Studio uses (/MDd or /MTd).
add_executable(profileListHexminEraseDbg profileListHexminErase.cpp)
target_compile_definitions(profileListHexminEraseDbg PUBLIC _GLIBCXX_DEBUG)

# Test MathAlgo code
add_executable(testMathAlgo testMathAlgo.cpp)
add_test(testMathAlgo testMathAlgo)

add_executable(testMathAlgo2 testMathAlgo2.cpp)
add_test(testMathAlgo2 testMathAlgo2)

# Test the scaling code
add_executable(testScale testScale.cpp)
add_test(testScale testScale)

add_executable(testrange testrange.cpp)
add_test(testrange testrange)

add_executable(testrange_constexpr testrange_constexpr.cpp)
add_test(testrange_constexpr testrange_constexpr)

add_executable(testrange_complex testrange_complex.cpp)
add_test(testrange_complex testrange_complex)

# Test the colour mapping
add_executable(testColourMap testColourMap.cpp)
add_test(testColourMap testColourMap)

add_executable(testrgbhsv testrgbhsv.cpp)
add_test(testrgbhsv testrgbhsv)

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

# Test bootstrapping
add_executable(testbootstrap testbootstrap.cpp)
# Test disabled - statistical fluctuations can make this fail sometimes
# add_test(testbootstrap testbootstrap)

# Neural nets

# Test morph::nn::ElmanNet
add_executable(test_elman test_elman.cpp)
add_test(test_elman test_elman)

add_executable(ff_debug ff_debug.cpp)
add_test(ff_debug ff_debug)

add_executable(testdirs testdirs.cpp)
add_test(testdirs testdirs)

#
# Boolean gene nets. Fixme: These are not unit tests, but I've thrown
# them in here for now. Perhaps need a 'bn' directory to build these
# programs.
#

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES x86_64) # Intel architecture required for AVX instructions
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

  if(NOT WIN32)
    # testGradGenome tries to create random num generator with width <
    # 16 bits, not strictly allowed and enforced by VS2019
    add_executable(testGradGenome testGradGenome.cpp)
    if (APPLE)
      target_compile_options(testGradGenome PUBLIC "-mavx")
    endif()
    #add_test(testGradGenome testGradGenome )
  endif()

endif() # intel architecture test

if(ARMADILLO_FOUND)
  # Test Cartesian (rectangular) grid
  add_executable(testcartgrid testcartgrid.cpp)
  add_test(testcartgrid testcartgrid)

  # Test shiftIndicies function
  add_executable(testCartGridShiftCoords testCartGridShiftCoords.cpp)
  add_test(testCartGridShiftCoords testCartGridShiftCoords)

  # Test shiftIndicies function
  add_executable(testCartGridShiftIndiciesByMetric testCartGridShiftIndiciesByMetric.cpp)
  add_test(testCartGridShiftIndiciesByMetric testCartGridShiftIndiciesByMetric)

endif()

# morph::tools
add_executable(testTools testTools.cpp)
add_test(testTools testTools)

add_executable(testboxfilter testboxfilter.cpp)
add_test(testboxfilter testboxfilter)

if(ARRAY_LITERAL_TEMPL_ARGS_ARE_OK)
  add_executable(testGridct testGridct.cpp)
  add_test(testGridct testGridct)

  add_executable(profileGrid profileGrid.cpp)

  add_executable(testGridctRowCol testGridctRowCol.cpp)
  add_test(testGridctRowCol testGridctRowCol)

  add_executable(testGridctNeighbours testGridctNeighbours.cpp)
  add_test(testGridctNeighbours testGridctNeighbours)
endif()

add_executable(testGrid testGrid.cpp)
add_test(testGrid testGrid)

add_executable(testGrid_suggest_dims testGrid_suggest_dims.cpp)
add_test(testGrid_suggest_dims testGrid_suggest_dims)

add_executable(testGridRowCol testGridRowCol.cpp)
add_test(testGridRowCol testGridRowCol)

add_executable(testGrid_shiftindex testGrid_shiftindex.cpp)
add_test(testGrid_shiftindex testGrid_shiftindex)

add_executable(testGridIndexLookup testGridIndexLookup.cpp)
add_test(testGridIndexLookup testGridIndexLookup)

add_executable(testGridNeighbours testGridNeighbours.cpp)
add_test(testGridNeighbours testGridNeighbours)

add_executable(testGrid_getabscissae testGrid_getabscissae.cpp)
add_test(testGrid_getabscissae testGrid_getabscissae)

add_executable(testGrid_profile testGrid_profile.cpp)
add_test(testGrid_profile testGrid_profile)

add_executable(testloadpng testloadpng.cpp)
add_test(testloadpng testloadpng)

add_executable(test_histo test_histo.cpp)
add_test(test_histo test_histo)

add_executable(test_number_type test_number_type.cpp)
add_test(test_number_type test_number_type)

add_executable(test_flags test_flags.cpp)
add_test(test_flags test_flags)

if(NOT APPLE)
add_executable(testcmath testcmath.cpp)
add_test(testcmath testcmath)
endif()

add_executable(testgraphnumberformat testgraphnumberformat.cpp)
add_test(testgraphnumberformat testgraphnumberformat)

add_executable(testgraphnumberformat_ss testgraphnumberformat.cpp)
target_compile_definitions(testgraphnumberformat_ss PUBLIC FORCE_NON_FORMAT)
add_test(testgraphnumberformat_ss testgraphnumberformat_ss)

add_executable(testmaketicks testmaketicks.cpp)
add_test(testmaketicks testmaketicks)

add_executable(testsigfigs testsigfigs.cpp)
add_test(testsigfigs testsigfigs)
