# Install the rapidxml headers in the morph include dir.
install(
  FILES rapidxml_iterators.hpp rapidxml_utils.hpp rapidxml.hpp rapidxml_print.hpp
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/morph
  )
