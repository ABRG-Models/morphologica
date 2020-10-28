// The coded-in shaders tell non-Mac platforms that they use OpenGL 4.5, but Mac limited to 4.1
#version 410
in VERTEX
{
    vec4 normal;
    vec3 color;
    float alpha;
} vertex;

//in vec2 TexCoords;

//uniform sampler2D text; // That's the bitmap
//uniform vec3 textColour;

out vec4 finalcolor;
void main()
{
    //vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    //finalcolor = vec4(textColour, vertex.alpha) * sampled;
    // or:
    finalcolor = vec4(vertex.color, vertex.alpha); // vertex.color gives the background
}
