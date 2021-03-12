# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

add_executable(huddling huddling.cpp)

target_compile_definitions(huddling PUBLIC FLT=float)
target_link_libraries(huddling softmats)
