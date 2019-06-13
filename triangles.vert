
#version 400 core

layout(location=0) in vec4 vPosition;

//uniform mat4 mvp_matrix;

void
main()
{
//    gl_Position = mvp_matrix * vPosition;
    gl_Position = vPosition;
}
