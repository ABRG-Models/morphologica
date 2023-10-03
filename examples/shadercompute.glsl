#version 450 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image2D imgOutput;

void main()
{
    vec4 value = vec4 (0.0, 0.0, 0.0, 1.0);

    // uvec3 gl_GlobalInvocationID
    ivec2 texelCoord = ivec2 (gl_GlobalInvocationID.xy);

    // gl_NumWorkGroups is a uvec3. First get a proportional x value, between 0 and 1,
    // simply by using the index of the texture pixel.
    float x_prop = float(texelCoord.x)/(gl_NumWorkGroups.x);
    // Now compute a sigmoid of this, and write into value.x for the texture
    value.x = 1.0 / (1.0 + exp (-12*(x_prop-0.5)));
    // Similar for y
    float y_prop = float(texelCoord.y)/(gl_NumWorkGroups.y);
    value.y = 1.0 / (1.0 + exp (-12*(y_prop-0.5)));
    imageStore (imgOutput, texelCoord, value);
}
