#include "Visual.h"

#include "GL3/gl3.h"
#include "GL/glext.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <cstring>
using std::strlen;

using morph::ShaderInfo;

void
morph::Visual::errorCallback (int error, const char* description)
{
    cerr << "Error: " << description << " (code "  << error << ")" << endl;
}

morph::Visual::Visual(int width, int height, const string& title)
{
    if (!glfwInit()) {
        // Initialization failed
        cerr << "GLFW initialization failed!" << endl;
    }

    glfwSetErrorCallback (morph::Visual::errorCallback);


    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 5);
    this->window = glfwCreateWindow (width, height, title.c_str(), NULL, NULL);
    if (!this->window) {
        // Window or OpenGL context creation failed
        cerr << "GLFW window creation failed!" << endl;
    }

    // Load up the shaders
    ShaderInfo shaders[] = {
        {GL_VERTEX_SHADER, "Visual.vert.glsl" },
        {GL_FRAGMENT_SHADER, "Visual.frag.glsl" },
        {GL_NONE, NULL }
    };
    this->shaderprog = this->LoadShaders (shaders);

    glUseProgram (this->shaderprog);

    // Now client code can set up HexGridVisuals.
    glEnable (GL_DEPTH_TEST);
}

morph::Visual::~Visual()
{
    glfwDestroyWindow (this->window);
    glfwTerminate();
}

void
morph::Visual::render (void)
{
    // Render it.
    vector<HexGridVisual*>::iterator hgvi = this->hexGridVis.begin();
    while (hgvi != this->hexGridVis.end()) {
        (*hgvi)->render();
        ++hgvi;
    }
}

void
morph::Visual::updateHexGridVisual (const unsigned int gridId,
                                    const vector<float>& data)
{
    // Replace grids[gridId].data
}

unsigned int
morph::Visual::addHexGridVisual (const HexGrid* hg,
                                 const vector<float>& data,
                                 const array<float, 3> offset)
{
    // Copy x/y positions from the HexGrid and make a copy of the data as vertices.
    HexGridVisual* hgv1 = new HexGridVisual(this, hg, &data, offset);
    this->hexGridVis.push_back (hgv1);

    return 0;
}

const GLchar*
morph::Visual::ReadShader (const char* filename)
{
    FILE* infile = fopen (filename, "rb");

    if (!infile) {
        cerr << "Unable to open file '" << filename << "'" << std::endl;
        return NULL;
    }

    fseek (infile, 0, SEEK_END);
    int len = ftell (infile);
    fseek (infile, 0, SEEK_SET);

    GLchar* source = new GLchar[len+1];

    fread (source, 1, len, infile);
    fclose (infile);

    source[len] = 0;

    return const_cast<const GLchar*>(source);
}

GLuint
morph::Visual::LoadShaders (ShaderInfo* shaders)
{
    if (shaders == NULL) { return 0; }

    GLuint program = glCreateProgram();

    GLboolean shaderCompilerPresent = GL_TRUE;
    glGetBooleanv (GL_SHADER_COMPILER, &shaderCompilerPresent);
    if (shaderCompilerPresent == GL_FALSE) {
        cerr << "shader compiler NOT present" << endl;
    }

    ShaderInfo* entry = shaders;
    while (entry->type != GL_NONE) {
        GLuint shader = glCreateShader (entry->type);
        entry->shader = shader;

        const GLchar* source = morph::Visual::ReadShader (entry->filename);
        if (source == NULL) {
            for (entry = shaders; entry->type != GL_NONE; ++entry) {
                glDeleteShader (entry->shader);
                entry->shader = 0;
            }
            return 0;
        } else {
            cout << "Compiling this shader: " << endl << "-----" << endl;
            cout << source << "-----" << endl;
        }
        GLint slen = (GLint)strlen (source);
        cout << "Shader length: " << slen << endl;
        glShaderSource (shader, 1, &source, &slen);
        delete [] source;

        glCompileShader (shader);
        GLenum shaderError = glGetError();
        if (shaderError == GL_INVALID_VALUE) {
            cout << "Shader compilation resulted in GL_INVALID_VALUE" << endl;
        } else if (shaderError == GL_INVALID_OPERATION) {
            cout << "Shader compilation resulted in GL_INVALID_OPERATION" << endl;
        } // shaderError is 0

        GLint compiled = GL_FALSE;
        glGetShaderiv (shader, GL_COMPILE_STATUS, &compiled);
        if (compiled != GL_TRUE) {
            GLsizei len = 0;
            glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &len);
            cout << "compiled is GL_FALSE. log length is " << len << " compiled has value " << compiled <<  endl;
            if (len > 0) {
                GLchar* log = new GLchar[len+1];
                glGetShaderInfoLog (shader, len, &len, log);
                std::cerr << "Shader compilation failed: " << (char*)log << std::endl;
                delete [] log;
            }
            return 0;
        }

        glAttachShader (program, shader);

        ++entry;
    }

    glLinkProgram (program);

    GLint linked;
    glGetProgramiv (program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLsizei len;
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &len );
        GLchar* log = new GLchar[len+1];
        glGetProgramInfoLog( program, len, &len, log );
        std::cerr << "Shader linking failed: " << log << std::endl;
        delete [] log;

        for (entry = shaders; entry->type != GL_NONE; ++entry) {
            glDeleteShader (entry->shader);
            entry->shader = 0;
        }

        return 0;
    }

    return program;
}
