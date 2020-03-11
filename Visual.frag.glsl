// The coded-in shaders tell non-Mac platforms that they use OpenGL 4.5, but Mac limited to 4.1
#version 410
in VERTEX
{
    vec4 normal;
    vec4 color;
} vertex;

out vec4 finalcolor;
void main() {
    finalcolor = vertex.color;
}
