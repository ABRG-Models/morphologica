
#find_package(glm REQUIRED)
#find_package(GLEW REQUIRED)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	${GLM_LIBRARY}
	GLEW
	armadillo
	openblas
	${LAPACK_LIBRARIES}
)

file(GLOB SOURCES
    main.cpp
	geometry/*.cpp
	core/*.cpp
	collision/*.cpp
	views/*.cpp
	utilities/*.cpp
)

file(GLOB HEADERS
	geometry/*.h
	core/*.h
	collision/*.h
	views/*.h
	utilities/*.h
	views/*.hpp
)


# Link to the special armadillo libpath, if requested
if (NOT "${MORPH_ARMADILLO_LIBPATH}" STREQUAL "")
  message ("-- Adding MORPH_ARMADILLO_LIBPATH: ${MORPH_ARMADILLO_LIBPATH} to link directories")
  link_directories(${MORPH_ARMADILLO_LIBPATH})
endif()

#
# Link the old display code separately - morphdisplay0
#
message("Sources: ${SOURCES}")
add_library(animats SHARED ${SOURCES})

target_include_directories(animats PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}> # so include "morph/blah.h" works
  $<INSTALL_INTERFACE:include>  # <prefix>/include so that include <morph/blah.h> works
)

# Common links
target_link_libraries(animats ${ALL_LIBS}}) # Animat specific links

install(TARGETS animats EXPORT morphologica LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Header installation
install(
  FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph/animats
  )
  

#if (${glfw3_FOUND})
#  install( FILES VisualBase.h Visual.h VisualModel.h VisualDataModel.h CoordArrows.h HexGridVisual.h QuadsVisual.h PointRowsVisual.h ScatterVisual.h QuiverVisual.h VisualDefaultShaders.h RecurrentNetworkModel.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph)
#endif()


# The testboundary utility program
#add_executable(jsontest json_test.cpp)
#target_link_libraries(testboundary morphdisplay0 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${LAPACK_LIBRARIES} ${OpenCV_LIBS})
#install(TARGETS testboundary DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

#add_executable(animats ${SOURCES})
#target_include_directories(animats PRIVATE
#	"${PROJECT_SOURCE_DIR}/src/"
#)

#target_link_libraries(animats
#	${ALL_LIBS}
#)

message("Copying the shaders from")
message("${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/")
message("to")
message("${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/")

add_custom_command( TARGET animats POST_BUILD 
					COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders" 
					"${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/shaders")





