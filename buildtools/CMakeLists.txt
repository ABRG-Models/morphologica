if(WIN32)
  # incbin tool
  add_executable(incbin incbin.c)
endif()

# Colourmap processing
add_executable(process_colourmap process_colourmap.cpp)
