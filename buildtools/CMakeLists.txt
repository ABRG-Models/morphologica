if(WIN32)
  # incbin tool
  add_executable(incbin incbin.c)
endif()

# Permit #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})
if(NOT WIN32)
  # Colourmap processing is not Windows compatible
  add_executable(process_colourmap process_colourmap.cpp)
  add_executable(process_colourtables process_colourtables.cpp)
endif()
