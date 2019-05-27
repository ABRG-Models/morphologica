#version 450
in VERTEX
{
    vec4 normal;
    vec4 color;
} vertex;

out vec4 finalcolor;
void main() {
    finalcolor = vertex.color;
}
