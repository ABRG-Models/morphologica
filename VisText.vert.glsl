// The coded-in shaders tell non-Mac platforms that they use OpenGL 4.5, but Mac limited to 4.1
#version 410

uniform mat4 mvp_matrix;
uniform mat4 vp_matrix;

layout(location = 0) in vec4 position; // Attrib location 0
layout(location = 1) in vec4 texture;  // texture
out vec2 TexCoords;

void main()
{
    gl_Position = mvp_matrix * position;
    TexCoords = texture.wz;
}
