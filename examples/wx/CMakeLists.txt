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

# The WX widgets example executable
add_executable (main-wx main-wx.cpp)
target_compile_definitions(main-wx PRIVATE ${wxWidgets_DEFINITIONS}) # Possibly also ${wxWidgets_DEFINITIONS_DEBUG}
target_link_libraries(main-wx ${wxWidgets_LIBRARIES} OpenGL::GL Freetype::Freetype GLEW::GLEW)

# Example from https://github.com/lszl84/wx_opengl_tutorial/blob/main/src/main.cpp
add_executable (wx-example wx-example.cpp)
target_compile_definitions(wx-example PRIVATE
"${wxWidgets_DEFINITIONS} -Wno-unused-parameter")
target_link_libraries(wx-example ${wxWidgets_LIBRARIES} OpenGL::GL Freetype::Freetype GLEW::GLEW)

# Pulling frame and canvas into viswx.h
add_executable (wx-example2 wx-example2.cpp)
target_compile_definitions(wx-example2 PRIVATE
"${wxWidgets_DEFINITIONS} -Wno-unused-parameter")
target_link_libraries(wx-example2 ${wxWidgets_LIBRARIES} OpenGL::GL Freetype::Freetype GLEW::GLEW)

endif()
