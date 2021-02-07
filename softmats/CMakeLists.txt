cmake_minimum_required (VERSION 2.6)

# project (softmax)

# Following `cmake --help-policy CMP0072`
set(OpenGL_GL_PREFERENCE "GLVND")

# Required packages
# find_package(OpenGL REQUIRED)
# find_package(glfw3 3.3 REQUIRED)
# find_package(glm REQUIRED)
# find_package(glew REQUIRED)
# find_package(Armadillo REQUIRED)
find_package(OpenCV REQUIRED)
# find_package(LAPACK REQUIRED)

#include_directories(
#	${GLFW_INCLUDE}
#	${GLM_INCLUDE}
#	${INCLUDE_INCLUDE}
#	${ARMADILLO_INCLUDE_DIR}
#    ${OpenCV_INCLUDE_DIRS}
#)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	${GLM_LIBRARY}
	GLEW
	armadillo
	lapack
	${OpenCV_LIBS}
)



file(GLOB SOURCES
    src/*.cpp
	src/visual/*.cpp
	src/core/*.cpp
	src/util/*.cpp
	src/collisions/*.cpp
)

file(GLOB HEADERS
	src/*.h
	src/visual/*.h
	src/core/*.h
	src/util/*.h
	src/collisions/*.h
)

add_library(softmats SHARED ${SOURCES})


target_include_directories(softmats PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}> # so include "morph/blah.h" works
  $<INSTALL_INTERFACE:include>  # <prefix>/include so that include <morph/blah.h> works
)

# Common links
link_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})

target_link_libraries(softmats ${ALL_LIBS}) # Animat specific links

install(TARGETS softmats EXPORT SoftmatsConfig LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Header installation
install(
  FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph/softmats
  )

add_custom_command( TARGET softmats POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
				"${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/shaders")


message("Copying the shaders from")
message("${CMAKE_CURRENT_SOURCE_DIR}/shaders/")
message("to")
message("${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/")
