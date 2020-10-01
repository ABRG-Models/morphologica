// This file is included by Vertex.cpp. It contains default vertex and fragment
// shaders which get compiled in to libmorphologica.

// Define a version string for the shaders
#ifdef __OSX__
// Mac support is fixed at OpenGL 4.1 in favour of their own graphics API.
#define OpenGL_VersionString "#version 410\n"
#else
/*
 On other platforms ALSO specify OpenGL 4.1, because only relatively simple features
 of OpenGL are in use. In future, may wish to change this, in case there's a need for
 any of the following major features released since OpenGL 4.1 (this list from the
 wikipedia OpenGL page):

OpenGL 4.2

 Release date: August 8, 2011

 Support for shaders with atomic counters and load-store-atomic read-modify-write
 operations to one level of a texture

 Drawing multiple instances of data captured from GPU vertex processing (including
 tessellation), to enable complex objects to be efficiently repositioned and
 replicated

 Support for modifying an arbitrary subset of a compressed texture, without having to
 re-download the whole texture to the GPU for significant performance improvements

OpenGL 4.3

 Release date: August 6, 2012

 Compute shaders leveraging GPU parallelism within the context of the graphics pipeline

 Shader storage buffer objects, allowing shaders to read and write buffer objects like
 image load/store from 4.2, but through the language rather than function calls.

 Image format parameter queries

 ETC2/EAC texture compression as a standard feature

 Full compatibility with OpenGL ES 3.0 APIs

 Debug abilities to receive debugging messages during application development

 Texture views to interpret textures in different ways without data replication

 Increased memory security and multi-application robustness

OpenGL 4.4

 Release date: July 22, 2013

 Enforced buffer object usage controls

 Asynchronous queries into buffer objects

 Expression of more layout controls of interface variables in shaders

 Efficient binding of multiple objects simultaneously

OpenGL 4.5

 Release date: August 11, 2014

 Direct State Access (DSA) - object accessors enable state to be queried and modified
 without binding objects to contexts, for increased application and middleware
 efficiency and flexibility.

 Flush Control - applications can control flushing of pending commands before context
 switching - enabling high-performance multithreaded applications;

 Robustness - providing a secure platform for applications such as WebGL browsers,
 including preventing a GPU reset affecting any other running applications;

 OpenGL ES 3.1 API and shader compatibility - to enable the easy development and
 execution of the latest OpenGL ES applications on desktop systems.

 OpenGL 4.6

 Release date: July 31, 2017

 more efficient, GPU-sided, geometry processing
 more efficient shader execution (AZDO)
 more information through statistics, overflow query and counters
 higher performance through no error handling contexts
 clamping of polygon offset function, solves a shadow rendering problem
 SPIR-V shaders
 Improved anisotropic filtering
*/
#define OpenGL_VersionString "#version 410\n"
#endif

// The default vertex shader. To study this GLSL, see Visual.vert.glsl, which has some
// code comments.
const char* defaultVtxShader = OpenGL_VersionString
    "uniform mat4 mvp_matrix;\n"
    "uniform float alpha;\n"
    "layout(location = 0) in vec4 position;\n"
    "layout(location = 1) in vec4 normalin;\n"
    "layout(location = 2) in vec3 color;\n"
    "out VERTEX\n"
    "{\n"
    "    vec4 normal;\n"
    "    vec3 color;\n"
    "    float alpha;\n"
    "} vertex;\n"
    "void main (void)\n"
    "{\n"
    "    gl_Position = (mvp_matrix * position);\n"
    "    vertex.color = color;\n"
    "    vertex.alpha = alpha;\n"
    "    vertex.normal = mvp_matrix * normalin;\n"
    "}";

// Default fragment shader. To study this GLSL, see Visual.frag.glsl.
const char* defaultFragShader = OpenGL_VersionString
    "in VERTEX\n"
    "{\n"
    "    vec4 normal;\n"
    "    vec3 color;\n"
    "    float alpha;\n"
    "} vertex;\n"
    "out vec4 finalcolor;\n"
    "void main() {\n"
    "    finalcolor = vec4(vertex.color, vertex.alpha);\n"
    "}";
