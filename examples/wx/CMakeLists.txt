#
# wxWindows/wxWidgets setup. This CMakeLists.txt is processed only if the top-level
# find_package(wxWidgets...) call found what it was looking for.
#

# Ensure we can #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

# wxWidgets has some additional compiler flags...
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${wxWidgets_CXX_FLAGS}")
# ...and include directories
include_directories(${wxWidgets_INCLUDE_DIRS})

# The WX widgets example executable
add_executable (main-wx main-wx.cpp)
target_compile_definitions(main-wx PRIVATE ${wxWidgets_DEFINITIONS}) # Possibly also ${wxWidgets_DEFINITIONS_DEBUG}
target_link_libraries(main-wx ${wxWidgets_LIBRARIES} OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(main-wx GLEW::GLEW)
endif()
