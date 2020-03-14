# Schnakenberg RD system
add_executable(schnakenberg schnakenberg.cpp)
target_compile_definitions(schnakenberg PUBLIC FLOATTYPE=float COMPILE_PLOTTING)
target_link_libraries(schnakenberg morphologica)

# Gierer-Meinhardt RD system
#add_executable(gierermeinhardt gierermeinhardt.cpp)
#target_compile_definitions(gierermeinhardt PUBLIC FLOATTYPE=float COMPILE_PLOTTING)
#target_link_libraries(gierermeinhardt ${MORPH_LDFLAGS} ${HDF5LIBLINK} ${JSONLIBLINK})
