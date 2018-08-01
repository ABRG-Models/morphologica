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
#set(TARGETTEST6 testhexgrid2 CACHE TYPE STRING)
#set(SOURCETEST6 testhexgrid2.cpp)
add_executable(testhexgrid2 testhexgrid2.cpp)
target_link_libraries(testhexgrid2 morphologica)
add_test(testhexgrid2 testhexgrid2)
