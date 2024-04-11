// The coded-in shaders tell non-Mac platforms that they use OpenGL 4.5, but Mac limited to 4.1
#version 410
in VERTEX
{
    vec4 normal;
    vec4 color;
    vec3 fragpos;
} vertex;

// To obtain the normal behaviour, set light_colour to white, ambient_intensity to 1 and
// diffuse_intensity to 0. That means I have just one shader for objects and it's easy
// to change the lighting.

uniform vec3 light_colour;       // Colour for both ambient and diffuse. Probably white.
uniform float ambient_intensity; // Ambient intensity
uniform vec3 diffuse_position;   // Positioned light
uniform float diffuse_intensity; // Diffuse light intensity

//uniform mat4 lv_matrix; // 'light' scene view matrix
//uniform mat4 p_matrix; // projection matrix

out vec4 finalcolor;

void main()
{
    vec3 norm = normalize(vec3(vertex.normal));
    //vec3 dpos_trans = vec3(p_matrix * lv_matrix * vec4(diffuse_position, 1));
    //vec3 light_dirn = normalize(dpos_trans - vertex.fragpos);
    vec3 light_dirn = normalize(diffuse_position - vertex.fragpos);
    float effective_diffuse = max(dot(norm, light_dirn), 0.0);
    vec3 diffuse = diffuse_intensity * effective_diffuse * light_colour;
    vec3 ambient = ambient_intensity * light_colour;
    vec3 result = (ambient+diffuse) * vec3(vertex.color);
    finalcolor = vec4(result, vertex.color.w);
    // Compared with simple shader:
    // finalcolor = vertex.color;
}
