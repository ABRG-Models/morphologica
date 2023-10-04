#version 450 core

layout (local_size_x = 10, local_size_y = 10, local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image2D imgOutput;

layout (location = 0) uniform float t;

void main()
{
    vec4 value = vec4 (0.0, 0.0, 0.0, 1.0);

    // uvec3 gl_GlobalInvocationID
    ivec2 texelCoord = ivec2 (gl_GlobalInvocationID.xy);

    float speed = 0.1;
    // the width of the texture
    float width = 1000;

    // gl_NumWorkGroups is a uvec3. First get a proportional x value, between 0 and 1,
    // simply by using the index of the texture pixel.
    float x_prop = mod(float(texelCoord.x) + t * speed, width)/(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
    // Now compute a sigmoid of this, and write into value.x for the texture
    value.x = 1.0 / (1.0 + exp (-12*(x_prop-0.5)));
    // Similar for y
    float y_prop = float(texelCoord.y)/(gl_NumWorkGroups.y*gl_WorkGroupSize.y);
    value.y = 1.0 / (1.0 + exp (-12*(y_prop-0.5)));

    imageStore (imgOutput, texelCoord, value);
}
