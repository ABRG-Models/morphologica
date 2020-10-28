// The coded-in shaders tell non-Mac platforms that they use OpenGL 4.5, but Mac limited to 4.1
#version 410

// ProjMatrix * RotnMatrix operation carried out on CPU in this
// program, so only one mvp_matrix.
uniform mat4 mvp_matrix;

uniform float alpha;

layout(location = 0) in vec4 position; // Attrib location 0
layout(location = 1) in vec4 normalin; // Attrib location 1
layout(location = 2) in vec3 color;    // Attrib location 2
//layout(location = 3) in vec4 tex;      // Attrib location 3
out VERTEX
{
    vec4 normal;
    vec3 color;
    float alpha;
} vertex;
//out vec2 TexCoords;

void main (void)
{
    gl_Position = (mvp_matrix * position);
    vertex.color = color;
    vertex.alpha = alpha;
    // Normals are all automatically computed, so there's no need for
    // this line and the cube program doesn't bother to pass in the
    // normals. Maybe required only for lighting?
    vertex.normal = mvp_matrix * normalin;
    //TexCoords = tex.xy;

}
