#
# wxWindows/wxWidgets setup. This CMakeLists.txt is processed only if the top-level
# find_package(wxWidgets...) call found what it was looking for.
#
if(USE_GLEW) # USE_GLEW *should* be on for the wxWidgets examples

# Ensure we can #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

# wxWidgets has some additional compiler flags...
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${wxWidgets_CXX_FLAGS}")
# ...and include directories
include_directories(${wxWidgets_INCLUDE_DIRS})

set(MORPH_LIBS_WX OpenGL::GL Freetype::Freetype)
if (HAVE_LODEPNG)
  set(MORPH_LIBS_WX ${MORPH_LIBS_WX} lodepng)
endif()

# morph::Visual graph1 example with morph/wx/viswx.h
add_executable (wx-graph1 wx-graph1.cpp)
target_compile_definitions(wx-graph1 PRIVATE
"${wxWidgets_DEFINITIONS} -Wno-unused-parameter")
target_link_libraries(wx-graph1 ${wxWidgets_LIBRARIES} ${MORPH_LIBS_WX} GLEW::GLEW)

# morph::Visual graph6 example with morph/wx/viswx.h
add_executable (wx-graph6 wx-graph6.cpp)
target_compile_definitions(wx-graph6 PRIVATE
"${wxWidgets_DEFINITIONS} -Wno-unused-parameter")
target_link_libraries(wx-graph6 ${wxWidgets_LIBRARIES} ${MORPH_LIBS_WX} GLEW::GLEW)

# Triaxes
add_executable (wx-triaxes wx-triaxes.cpp)
target_compile_definitions(wx-triaxes PRIVATE
"${wxWidgets_DEFINITIONS} -Wno-unused-parameter")
target_link_libraries(wx-triaxes ${wxWidgets_LIBRARIES} ${MORPH_LIBS_WX} GLEW::GLEW)

endif()
