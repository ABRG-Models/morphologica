#version 450 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image2D imgOutput;

void main()
{
    vec4 value = vec4 (0.0, 0.0, 0.0, 1.0);

    // uvec3 gl_GlobalInvocationID
    ivec2 texelCoord = ivec2 (gl_GlobalInvocationID.xy);

    // uvec3 gl_NumWorkGroups
    value.x = float(texelCoord.x)/(gl_NumWorkGroups.x);
    value.y = float(texelCoord.y)/(gl_NumWorkGroups.y);

    imageStore (imgOutput, texelCoord, value);
}
