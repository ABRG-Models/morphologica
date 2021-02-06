cmake_minimum_required (VERSION 2.6)

project (softmax)

# Following `cmake --help-policy CMP0072`
set(OpenGL_GL_PREFERENCE "GLVND")

# Required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_package(glew REQUIRED)
find_package(Armadillo REQUIRED)
find_package(OpenCV REQUIRED)
find_package(LAPACK REQUIRED)

include_directories(
	${GLFW_INCLUDE}
	${GLM_INCLUDE}
	${INCLUDE_INCLUDE}
	${ARMADILLO_INCLUDE_DIR}
        ${OpenCV_INCLUDE_DIRS}
)

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
	src/*.h
	src/visual/*.cpp
	src/visual/*.h
	src/core/*.cpp
	src/core/*.h
	src/util/*.cpp
	src/util/*.h
	src/collisions/*.cpp
	src/collisions/*.h
)

set(targets colliding)

foreach(X IN LISTS targets)
	message("Configuring ${X}")
	add_executable(${X} ${SOURCES} ${X}.cpp )
	target_link_libraries(${X} ${ALL_LIBS})
	add_custom_command( TARGET ${X} POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
					"${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/shaders")
endforeach()

message("Copying the shaders from")
message("${CMAKE_CURRENT_SOURCE_DIR}/shaders/")
message("to")
message("${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/")
