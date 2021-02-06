#version 430

// Input data
in vec2 tc;
in vec4 varyingColor;

// Output data
out vec4 color;

struct PositionalLight{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec3 position;
};

struct Material{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform Material material;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
uniform int type;

layout (binding=0) uniform sampler2D samp;

void main( void )
{
    if( type == 0 )
        color = texture( samp, tc );
    else
        color = varyingColor;
}
