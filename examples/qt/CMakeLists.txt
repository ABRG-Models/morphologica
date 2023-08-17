#
# Qt setup
#

# setup for qt packages (see http://www.kdab.com/using-cmake-with-qt-5/)
# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Also switch these on as they're on in QtCreator-generated projects
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Gui Core Widgets REQUIRED)

#
# All the library finding is now done. It's time to compile the program.
#

include_directories(${qtmorph_SOURCE_DIR})
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG_INIT} -D_DEBUG -D_DRAWGI -Wno-deprecated-declarations")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_INIT} -D_SECURE_SCL=0 -Wno-deprecated-declarations")

# Ensure we can #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

# Ensure our programs will get the correct links
set(MORPH_LIBS_GL OpenGL::GL glfw Freetype::Freetype)
set(MORPH_LIBS_CORE ${HDF5_C_LIBRARIES})

# Just about the simplest possible Qt morphologica program - draws the
# graph1 example in the applications single QWidget
add_subdirectory (graph1)

# A more useful MainWindow based Qt desktop application example that
# shows a HexGrid in a QWidget that has been laid out using QCreator's
# designer.
add_subdirectory (hexgrid)

# A MainWindow based Qt desktop application that shows a dynamic
# surface. Equivalent to examples/fps.cpp
add_subdirectory (fps)
