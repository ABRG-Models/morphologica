#version 310 es

// highp: 16 bit  (float -/+2^62); mediump: 10 bit (float -/+2^14); lowp: 8 bit (float -2 to 2)
// Setting a default precision for float:
precision highp float;
// Setting a default precision for image2D:
precision highp image2D;

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 1) buffer InputBlock { float my_input[]; };

layout (binding = 0, rgba32f) writeonly uniform image2D imgOutput;

// Specifying precision for an individual image2D:
layout (binding = 1, rgba32f) writeonly highp uniform image2D imgOutput2;

layout (location = 2) uniform float t;

void main()
{
    vec4 value = vec4 (0.0, 0.0, 0.0, 1.0);
    uint idx = gl_GlobalInvocationID.x + uint(256) * gl_GlobalInvocationID.y;
    ivec2 texelCoord = ivec2 (gl_GlobalInvocationID.xy);

    // Copy value of SSBO input into the texture so that it renders. Can then start working on the numbers...
    //value.r = my_input[idx];
    value.r = my_input[idx];
    //value.b = my_input[idx];

    // A red version
    vec4 value2 = vec4 (0.0, 0.0, 0.0, 1.0);
    value2.g = my_input[idx];


    imageStore (imgOutput, texelCoord, value);

    imageStore (imgOutput2, texelCoord, value2);
}
