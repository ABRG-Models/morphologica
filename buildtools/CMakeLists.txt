if(WIN32)
  # incbin tool
  add_executable(incbin incbin.c)
endif()

# Permit #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})
if(NOT WIN32)
  # Colourmap processing is not Windows compatible
  add_executable(process_colourmap process_colourmap.cpp)
  # Using std::format in process_colourtables
  check_include_file_cxx(format HAVE_STD_FORMAT)
  if(HAVE_STD_FORMAT)
    add_executable(process_colourtables process_colourtables.cpp)
    set_property(TARGET process_colourtables PROPERTY CXX_STANDARD 20)
  endif()
endif()
