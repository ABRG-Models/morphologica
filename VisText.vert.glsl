// The coded-in shaders tell non-Mac platforms that they use OpenGL 4.5, but Mac limited to 4.1
#version 410

uniform mat4 m_matrix;
uniform mat4 v_matrix;
uniform mat4 p_matrix;

layout(location = 0) in vec4 position; // Attrib location 0 is vertex position
layout(location = 1) in vec4 vnormal;  // Attrib location 1 is vertex normal
layout(location = 2) in vec4 vcolor;   // Attrib location 2 is vertex colour
layout(location = 3) in vec4 texture;  // Attrib location 3 is texture map location

out vec2 TexCoords;

void main()
{
    gl_Position = p_matrix * v_matrix * m_matrix * position;
    TexCoords = texture.xy;
}
