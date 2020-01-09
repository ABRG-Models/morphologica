// This file is included by Vertex.cpp.

// The default vertex shader. To study this GLSL, see Visual.vert.glsl, which has some
// code comments.
const char* defaultVtxShader = "#version 450\n"
    "uniform mat4 mvp_matrix;\n"
    "layout(location = 0) in vec4 position;\n"
    "layout(location = 1) in vec4 normalin;\n"
    "layout(location = 2) in vec4 color;\n"
    "out VERTEX\n"
    "{\n"
    "    vec4 normal;\n"
    "    vec4 color;\n"
    "} vertex;\n"
    "void main (void)\n"
    "{\n"
    "    gl_Position = (mvp_matrix * position);\n"
    "    vertex.color = color;\n"
    "    vertex.normal = mvp_matrix * normalin;\n"
    "}";

// Default fragment shader. To study this GLSL, see Visual.frag.glsl.
const char* defaultFragShader = "#version 450\n"
    "in VERTEX\n"
    "{\n"
    "    vec4 normal;\n"
    "    vec4 color;\n"
    "} vertex;\n"
    "out vec4 finalcolor;\n"
    "void main() {\n"
    "    finalcolor = vertex.color;\n"
    "}";
