// The coded-in shaders tell non-Mac platforms that they use OpenGL 4.5, but Mac limited to 4.1
#version 410
in VERTEX
{
    vec4 normal;
    vec3 color;
    float alpha;
} vertex;

out vec4 finalcolor;

void main() {
    finalcolor = vec4(vertex.color, vertex.alpha);
}
