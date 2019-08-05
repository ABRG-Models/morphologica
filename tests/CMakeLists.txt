# CTests

include_directories(../src)

if(APPLE)
  link_directories(/usr/X11R6/lib)
endif(APPLE)

# BezCurve class
set(TARGETTEST1 testbez CACHE TYPE STRING)
set(SOURCETEST1 testbez.cpp)
add_executable(${TARGETTEST1} ${SOURCETEST1})
#target_compile_features(${TARGETTEST1} PUBLIC)
target_link_libraries(${TARGETTEST1} morphologica)
add_test(testbez ${TARGETTEST1})

# Test two curves connected together
set(TARGETTEST2 twocurves CACHE TYPE STRING)
set(SOURCETEST2 twocurves.cpp)
add_executable(${TARGETTEST2} ${SOURCETEST2})
target_link_libraries(${TARGETTEST2} morphologica)
add_test(twocurves ${TARGETTEST2})

# Test reading trial.svg
set(TARGETTEST3 testreadcurves CACHE TYPE STRING)
set(SOURCETEST3 testreadcurves.cpp)
add_executable(${TARGETTEST3} ${SOURCETEST3})
target_link_libraries(${TARGETTEST3} morphologica)
add_test(testreadcurves ${TARGETTEST3})

# Test display. Note also linking to static library here, to prove it works.
set(TARGETTEST4 testdisplay CACHE TYPE STRING)
set(SOURCETEST4 testdisplay.cpp)
add_executable(${TARGETTEST4} ${SOURCETEST4})
target_link_libraries(${TARGETTEST4} morphstatic)
add_test(testdisplay ${TARGETTEST4})

# Test hexgrid
set(TARGETTEST5 testhexgrid CACHE TYPE STRING)
set(SOURCETEST5 testhexgrid.cpp)
add_executable(${TARGETTEST5} ${SOURCETEST5})
target_link_libraries(${TARGETTEST5} morphologica)
add_test(testhexgrid ${TARGETTEST5})

# Test hexgrid2
add_executable(testhexgrid2 testhexgrid2.cpp)
target_link_libraries(testhexgrid2 morphologica)
add_test(testhexgrid2 testhexgrid2)

# Test hexgrid3 (hexgrid2 with visualisation)
add_executable(testhexgrid3 testhexgrid3.cpp)
target_link_libraries(testhexgrid3 morphologica)
add_test(testhexgrid3 testhexgrid3)

# Test hexgridsave (HexGrid's save feature)
add_executable(testhexgridsave testhexgridsave.cpp)
target_link_libraries(testhexgridsave morphologica)
add_test(testhexgridsave testhexgridsave)

# Test boundary pgram
add_executable(testdom_pgram testdom_pgram.cpp)
target_link_libraries(testdom_pgram morphologica)
add_test(testdom_pgram testdom_pgram)

# Test boundary rect
add_executable(testdom_rect testdom_rect.cpp)
target_link_libraries(testdom_rect morphologica)
add_test(testdom_rect testdom_rect)

# Test boundary hex
add_executable(testdom_hex testdom_hex.cpp)
target_link_libraries(testdom_hex morphologica)
add_test(testdom_hex testdom_hex)

# Test boundary bound
add_executable(testdom_bound testdom_bound.cpp)
target_link_libraries(testdom_bound morphologica)
add_test(testdom_bound testdom_bound)

# Test boundary sub-parallelograms
#add_executable(testdom_subp testdom_subp.cpp)
#target_link_libraries(testdom_subp morphologica)
#add_test(testdom_subp testdom_subp)

# Test distance to boundary
add_executable(testhexbounddist testhexbounddist.cpp)
target_link_libraries(testhexbounddist morphologica)
add_test(testhexbounddist testhexbounddist)

# Test HDF file access
add_executable(testhdfdata1 testhdfdata1.cpp)
target_link_libraries(testhdfdata1 morphologica)
add_test(testhdfdata1 testhdfdata1)

# Test ellipse code (not written yet)
#add_executable(test_ellipseboundary test_ellipseboundary.cpp)
#target_link_libraries(test_ellipseboundary morphologica)
#add_test(test_ellipseboundary test_ellipseboundary)

if(${glfw3_FOUND})
  # Test morph::Visual class
  add_executable(testvis testVisual.cpp)
  target_link_libraries(testvis morphologica)
  #add_test(testvis testvis)
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
