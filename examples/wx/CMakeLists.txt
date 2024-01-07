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

# morph::Visual graph1 example with morph/wx/viswx.h
add_executable (wx-example3 wx-example3.cpp)
target_compile_definitions(wx-example3 PRIVATE
"${wxWidgets_DEFINITIONS} -Wno-unused-parameter")
target_link_libraries(wx-example3 ${wxWidgets_LIBRARIES} OpenGL::GL Freetype::Freetype GLEW::GLEW)

endif()
