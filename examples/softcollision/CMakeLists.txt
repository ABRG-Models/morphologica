# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

add_executable(softcollision colliding.cpp)

target_compile_definitions(softcollision PUBLIC FLT=float)
# Because softmats is linked to all relevant libs, only softmats needs to be linked in here.
target_link_libraries(softcollision softmats)
