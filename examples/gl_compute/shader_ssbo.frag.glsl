#version 310 es

// highp 16bit; mediump 10bit.
precision highp float;

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D tex;

void main()
{
    vec3 texCol = texture(tex, TexCoords).rgb;
    FragColor = vec4(texCol, 1.0);
}
