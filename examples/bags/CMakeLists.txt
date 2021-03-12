# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

add_executable(bags bags.cpp)

target_compile_definitions(bags PUBLIC FLT=float)
target_link_libraries(bags softmats)
