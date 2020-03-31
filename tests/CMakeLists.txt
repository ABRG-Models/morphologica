# CTests

# In tests allow #include <morph/hdr.h> or <hdr.h>
include_directories(..)
include_directories(../morph)

if(APPLE)
  link_directories(/usr/X11R6/lib)
endif(APPLE)

# BezCurve class
set(TARGETTEST1 testbez) # On Lana's Apple got this warning:
# Cmake warning (dev) at tests/CMakeLists.txt:12 (set):
# implicitly converting 'TYPE' to 'STRING' type.
# This warning is for project developers...
set(SOURCETEST1 testbez.cpp)
add_executable(${TARGETTEST1} ${SOURCETEST1})
#target_compile_features(${TARGETTEST1} PUBLIC)
target_link_libraries(${TARGETTEST1} morphologica)
add_test(testbez ${TARGETTEST1})

# BezCurve class
set(TARGETTEST1_1 testbez2)
set(SOURCETEST1_1 testbez2.cpp)
add_executable(${TARGETTEST1_1} ${SOURCETEST1_1})
target_link_libraries(${TARGETTEST1_1} morphologica)
add_test(testbez2 ${TARGETTEST1_1})

# BezCurvePath class
set(TARGETTEST1_2 testbezcurves)
set(SOURCETEST1_2 testbezcurves.cpp)
add_executable(${TARGETTEST1_2} ${SOURCETEST1_2})
target_link_libraries(${TARGETTEST1_2} morphologica)
add_test(testbezcurves ${TARGETTEST1_2})

# Testing matrix representation of Bezier curves
set(TARGETTEST1_3 testbezmatrix)
set(SOURCETEST1_3 testbezmatrix.cpp)
add_executable(${TARGETTEST1_3} ${SOURCETEST1_3})
# Disable warning -Wunused-but-set-variable:
target_compile_options(${TARGETTEST1_3} PRIVATE -Wno-unused-but-set-variable)
target_compile_definitions(${TARGETTEST1_3} PUBLIC FLT=float)
target_link_libraries(${TARGETTEST1_3} morphologica)
add_test(testbezmatrix ${TARGETTEST1_3})

# Testing auto-fitting a Bezier curve
set(TARGETTEST1_4 testbezfit)
set(SOURCETEST1_4 testbezfit.cpp)
add_executable(${TARGETTEST1_4} ${SOURCETEST1_4})
target_compile_definitions(${TARGETTEST1_4} PUBLIC FLT=float)
target_link_libraries(${TARGETTEST1_4} morphologica)
add_test(testbezfit ${TARGETTEST1_4})

# Testing splitting of Bezier curves
set(TARGETTEST1_5 testbezsplit)
set(SOURCETEST1_5 testbezsplit.cpp)
add_executable(${TARGETTEST1_5} ${SOURCETEST1_5})
target_compile_definitions(${TARGETTEST1_5} PUBLIC FLT=float)
target_link_libraries(${TARGETTEST1_5} morphologica)
add_test(testbezsplit ${TARGETTEST1_5})

# Testing Bezier derivatives (though really testing curve joining)
set(TARGETTEST1_6 testbezderiv)
set(SOURCETEST1_6 testbezderiv.cpp)
add_executable(${TARGETTEST1_6} ${SOURCETEST1_6})
target_compile_definitions(${TARGETTEST1_6} PUBLIC FLT=double)
target_link_libraries(${TARGETTEST1_6} morphologica)
add_test(testbezderiv ${TARGETTEST1_6})

# Testing Bezier derivatives
set(TARGETTEST1_7 testbezderiv2)
set(SOURCETEST1_7 testbezderiv2.cpp)
add_executable(${TARGETTEST1_7} ${SOURCETEST1_7})
target_compile_definitions(${TARGETTEST1_7} PUBLIC FLT=double)
target_link_libraries(${TARGETTEST1_7} morphologica)
add_test(testbezderiv2 ${TARGETTEST1_7})

# Testing Bezier derivatives
set(TARGETTEST1_8 testbezderiv3)
set(SOURCETEST1_8 testbezderiv3.cpp)
add_executable(${TARGETTEST1_8} ${SOURCETEST1_8})
target_compile_definitions(${TARGETTEST1_8} PUBLIC FLT=double)
target_link_libraries(${TARGETTEST1_8} morphologica)
add_test(testbezderiv3 ${TARGETTEST1_8})

# Test two curves connected together
set(TARGETTEST2 twocurves)
set(SOURCETEST2 twocurves.cpp)
add_executable(${TARGETTEST2} ${SOURCETEST2})
target_link_libraries(${TARGETTEST2} morphologica)
add_test(twocurves ${TARGETTEST2})

# Test reading trial.svg
set(TARGETTEST3 testreadcurves)
set(SOURCETEST3 testreadcurves.cpp)
add_executable(${TARGETTEST3} ${SOURCETEST3})
target_link_libraries(${TARGETTEST3} morphologica)
add_test(testreadcurves ${TARGETTEST3})

# Test reading whiskerbarrels_withcentres.svg
set(TARGETTEST31 testreadcurves_circles)
set(SOURCETEST31 testreadcurves_circles.cpp)
add_executable(${TARGETTEST31} ${SOURCETEST31})
target_link_libraries(${TARGETTEST31} morphologica)
add_test(testreadcurves_circles ${TARGETTEST31})

# Test display. Note also linking to static library here, to prove it works.
set(TARGETTEST4 testdisplay)
set(SOURCETEST4 testdisplay.cpp)
add_executable(${TARGETTEST4} ${SOURCETEST4})
target_link_libraries(${TARGETTEST4} morphstatic morphdisplay0 ${LAPACK_LIBRARIES})
add_test(testdisplay ${TARGETTEST4})

# Test hexgrid
set(TARGETTEST5 testhexgrid)
set(SOURCETEST5 testhexgrid.cpp)
add_executable(${TARGETTEST5} ${SOURCETEST5})
target_link_libraries(${TARGETTEST5} morphologica)
add_test(testhexgrid ${TARGETTEST5})

# Test hexgrid2
add_executable(testhexgrid2 testhexgrid2.cpp)
target_link_libraries(testhexgrid2 morphologica morphdisplay0)
add_test(testhexgrid2 testhexgrid2)

# Test hexgrid3 (hexgrid2 with visualisation)
add_executable(testhexgrid3 testhexgrid3.cpp)
target_link_libraries(testhexgrid3 morphologica morphdisplay0)
add_test(testhexgrid3 testhexgrid3)

# Test hexgridsave (HexGrid's save feature)
add_executable(testhexgridsave testhexgridsave.cpp)
target_link_libraries(testhexgridsave morphologica morphdisplay0)
add_test(testhexgridsave testhexgridsave)

# Test boundary pgram
add_executable(testdom_pgram testdom_pgram.cpp)
target_link_libraries(testdom_pgram morphologica morphdisplay0)
add_test(testdom_pgram testdom_pgram)

# Test boundary rect
add_executable(testdom_rect testdom_rect.cpp)
target_link_libraries(testdom_rect morphologica morphdisplay0)
add_test(testdom_rect testdom_rect)

# Test boundary hex
add_executable(testdom_hex testdom_hex.cpp)
target_link_libraries(testdom_hex morphologica morphdisplay0)
add_test(testdom_hex testdom_hex)

# Test boundary bound
add_executable(testdom_bound testdom_bound.cpp)
target_link_libraries(testdom_bound morphologica morphdisplay0)
add_test(testdom_bound testdom_bound)

# Test boundary sub-parallelograms
#add_executable(testdom_subp testdom_subp.cpp)
#target_link_libraries(testdom_subp morphologica morphdisplay0)
#add_test(testdom_subp testdom_subp)

# Test distance to boundary
add_executable(testhexbounddist testhexbounddist.cpp)
target_link_libraries(testhexbounddist morphologica)
add_test(testhexbounddist testhexbounddist)

# Test HDF file access
add_executable(testhdfdata1 testhdfdata1.cpp)
target_link_libraries(testhdfdata1 morphologica)
add_test(testhdfdata1 testhdfdata1)

# More HDF data tests
add_executable(testhdfdata2 testhdfdata2.cpp)
target_link_libraries(testhdfdata2 morphologica)
add_test(testhdfdata2 testhdfdata2)

add_executable(testhdfdata3f testhdfdata3.cpp)
target_compile_definitions(testhdfdata3f PUBLIC FLT=float)
target_link_libraries(testhdfdata3f morphologica)
add_test(testhdfdata3f testhdfdata3f)

add_executable(testhdfdata3d testhdfdata3.cpp)
target_compile_definitions(testhdfdata3d PUBLIC FLT=float)
target_link_libraries(testhdfdata3d morphologica)
add_test(testhdfdata3d testhdfdata3d)

if(${glfw3_FOUND})
  # Test morph::Visual class
  add_executable(testvis testVisual.cpp)
  target_link_libraries(testvis morphologica)
  #add_test(testvis testvis)

  add_executable(testvisquads testVisQuads.cpp)
  target_link_libraries(testvisquads morphologica)

  add_executable(testvispointrows testVisPointRows.cpp)
  target_link_libraries(testvispointrows -L${morphologica_BINARY_DIR}/src morphologica)

  add_executable(testvisscatter testVisScatter.cpp)
  target_link_libraries(testvisscatter -L${morphologica_BINARY_DIR}/src morphologica)

  add_executable(testvisquiver testVisQuiver.cpp)
  target_link_libraries(testvisquiver -L${morphologica_BINARY_DIR}/src morphologica)

  # Test elliptical HexGrid code (visualized with morph::Visual)
  add_executable(test_ellipseboundary test_ellipseboundary.cpp)
  target_link_libraries(test_ellipseboundary morphologica)
  add_test(test_ellipseboundary test_ellipseboundary)

  # Test circular HexGrid code
  add_executable(test_circleboundary test_circleboundary.cpp)
  target_link_libraries(test_circleboundary morphologica)
  add_test(test_circleboundary test_circleboundary)

endif()

# Test morph::Process class
add_executable(testProcess testProcess.cpp)
target_link_libraries(testProcess morphologica)
add_test(testProcess testProcess)

# Test morph::Quaternion
add_executable(testQuaternion testQuaternion.cpp)
target_link_libraries(testQuaternion morphologica)
add_test(testQuaternion testQuaternion)

# Test morph::Vector3
add_executable(testVector3 testVector3.cpp)
target_link_libraries(testVector3 morphologica)
add_test(testVector3 testVector3)

# Test morph::Transformmatrix
add_executable(testTransformMatrix testTransformMatrix.cpp)
target_link_libraries(testTransformMatrix morphologica)
add_test(testTransformMatrix testTransformMatrix)

# Test Hex::get_vertex_coord()
add_executable(testHexVertexPos testHexVertexPos.cpp)
target_link_libraries(testHexVertexPos morphologica)
add_test(testHexVertexPos testHexVertexPos)

# Test Hex user flags code
add_executable(testHexUserFlags testHexUserFlags.cpp)
target_link_libraries(testHexUserFlags morphologica)
add_test(testHexUserFlags testHexUserFlags)

# Test Dirichlet code
add_executable(testDirichlet testDirichlet.cpp)
target_link_libraries(testDirichlet morphologica morphdisplay0)
add_test(testDirichlet testDirichlet)

add_executable(testDirichlet2 testDirichlet2.cpp)
target_link_libraries(testDirichlet2 morphologica morphdisplay0)
add_test(testDirichlet2 testDirichlet2)

add_executable(testDirichlet4 testDirichlet4.cpp)
target_link_libraries(testDirichlet4 morphologica morphdisplay0)
add_test(testDirichlet4 testDirichlet4)

add_executable(testDirichlet5 testDirichlet5.cpp)
target_link_libraries(testDirichlet5 morphologica morphdisplay0)
add_test(testDirichlet5 testDirichlet5)

# Test MathAlgo code
add_executable(testMathAlgo testMathAlgo.cpp)
target_link_libraries(testMathAlgo morphologica)
add_test(testMathAlgo testMathAlgo)

# Test Nelder Mead algorithm
add_executable(testNMSimplex testNMSimplex.cpp)
target_compile_definitions(testNMSimplex PUBLIC FLT=float)
target_link_libraries(testNMSimplex morphologica)
add_test(testNMSimplex testNMSimplex)

# Test Random number generation code
add_executable(testRandom testRandom.cpp)
target_link_libraries(testRandom morphologica)
add_test(testRandom testRandom)
