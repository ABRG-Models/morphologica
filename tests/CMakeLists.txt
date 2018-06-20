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
