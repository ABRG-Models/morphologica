#
# WxWindows setup. This CMakeLists.txt is processed only if the top-level
# find_package (Wx... call found the WxWindows development libraries.
#

# Ensure we can #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

# wxWidgets has some additional compiler flags and we have to set the include dir, too
# wxWidgets can't compile with the full warning flags
#set(WXWIDGET_WARNING_FLAGS "-Wall -Wextra -Wpedantic -pedantic-errors -Wno-deprecated-copy -Werror")
#set(CMAKE_CXX_FLAGS "${MORPH_HOST_DEFINITION} -g ${WXWIDGET_WARNING_FLAGS} -march=native -O3 ${wxWidgets_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy ${wxWidgets_CXX_FLAGS}")
include_directories(${wxWidgets_INCLUDE_DIRS})


# The WX widgets example
add_executable (main-wx main-wx.cpp)
target_compile_definitions(main-wx PRIVATE ${wxWidgets_DEFINITIONS}) # "$<$<CONFIG:DEBUG>:${wxWidgets_DEFINITIONS_DEBUG}>")
target_link_libraries(main-wx ${wxWidgets_LIBRARIES} OpenGL::GL glfw Freetype::Freetype)
if(USE_GLEW)
  target_link_libraries(main-wx GLEW::GLEW)
endif()
