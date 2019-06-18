#version 450

// ProjMatrix * RotnMatrix operation carried out on CPU in this
// program, so only one mvp_matrix.
uniform mat4 mvp_matrix;

layout(location = 0) in vec4 position; // Attrib location 0
layout(location = 1) in vec4 normalin; // Attrib location 1
layout(location = 2) in vec4 color;    // Attrib location 2
out VERTEX
{
    vec4 normal;
    vec4 color;
} vertex;

void main (void)
{
    gl_Position = (mvp_matrix * position);
    vertex.color = color;
    // Normals are all automatically computed, so there's no need for
    // this line and the cube program doesn't bother to pass in the
    // normals. Maybe required only for lighting?
    vertex.normal = mvp_matrix * normalin;
}
