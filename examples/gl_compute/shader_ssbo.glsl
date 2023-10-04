#version 450 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 1) buffer InputBlock { float my_input[]; };

layout (rgba32f, binding = 0) uniform image2D imgOutput;

layout (rgba32f, binding = 2) uniform image2D imgOutput2;

layout (location = 0) uniform float t;

void main()
{
    vec4 value = vec4 (0.0, 0.0, 0.0, 1.0);
    uint idx = gl_GlobalInvocationID.x + 256 * gl_GlobalInvocationID.y;
    ivec2 texelCoord = ivec2 (gl_GlobalInvocationID.xy);

    // Copy value of SSBO input into the texture so that it renders. Can then start working on the numbers...
    //value.r = my_input[idx];
    value.g = my_input[idx];
    //value.b = my_input[idx];
    imageStore (imgOutput, texelCoord, value);

    // A red version
    vec4 value2 = vec4 (0.0, 0.0, 0.0, 1.0);
    value2.r = my_input[idx];
    imageStore (imgOutput2, texelCoord, value2);
}
