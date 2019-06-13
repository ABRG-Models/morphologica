#include "Visual.h"

#include "GL3/gl3.h"
#include "GL/glext.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <cstring>
using std::strlen;

#include "Quaternion.h"
using morph::Quaternion;

using morph::ShaderInfo;

morph::Visual::Visual(int width, int height, const string& title)
{
    if (!glfwInit()) {
        // Initialization failed
        cerr << "GLFW initialization failed!" << endl;
    }

    // Set up error callback
    glfwSetErrorCallback (morph::Visual::errorCallback);

    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 5);
    this->window = glfwCreateWindow (width, height, title.c_str(), NULL, NULL);
    if (!this->window) {
        // Window or OpenGL context creation failed
        cerr << "GLFW window creation failed!" << endl;
    }

    // Fix the event handling for benefit of static functions.
    this->setEventHandling();

    // Set up callbacks
    glfwSetKeyCallback (this->window,         VisualBase::key_callback_dispatch);
    glfwSetMouseButtonCallback (this->window, VisualBase::mouse_button_callback_dispatch);
    glfwSetCursorPosCallback (this->window,   VisualBase::cursor_position_callback_dispatch);
    // May not need these:
    //glfwSetWindowSizeCallback (this->window,  VisualBase::window_size_callback_dispatch);

    glfwMakeContextCurrent (this->window);

    // Load up the shaders
#if 0
    ShaderInfo shaders[] = {
        {GL_VERTEX_SHADER, "Visual.vert.glsl" },
        {GL_FRAGMENT_SHADER, "Visual.frag.glsl" },
        {GL_NONE, NULL }
    };
#endif
    ShaderInfo shaders[] = {
        {GL_VERTEX_SHADER, "triangles.vert" },
        {GL_FRAGMENT_SHADER, "triangles.frag" },
        //{GL_FRAGMENT_SHADER, "Visual.frag.glsl" },
        {GL_NONE, NULL }
    };
    this->shaderprog = this->LoadShaders (shaders);

    glUseProgram (this->shaderprog);

    // Now client code can set up HexGridVisuals.
    //glEnable (GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glDisable(GL_DEPTH_TEST);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

morph::Visual::~Visual()
{
    glfwDestroyWindow (this->window);
    glfwTerminate();
}

void
morph::Visual::mousePressEvent (void)
{
    // Save mouse press position
    this->mousePressPosition.x = static_cast<float>(this->cursorx);
    this->mousePressPosition.y = static_cast<float>(this->cursory);
}

void morph::Visual::mouseReleaseEvent (void)
{
    // Mouse release position - mouse press position
    Vector2<float> diff (static_cast<float>(this->cursorx),
                         static_cast<float>(this->cursory));

    diff -= this->mousePressPosition;
    //cout << "diff: " << diff.x << "," << diff.y << endl;

    // Rotation axis is perpendicular to the mouse position difference vector
    Vector3<float> n(diff.y, diff.x, 0.0f);
    n.renormalize();
    cout << "n = ";
    n.output();

    // Accelerate angular speed relative to the length of the mouse sweep
    float acc = diff.length() / 100.0;

    // Calculate new rotation axis as weighted sum
    this->rotationAxis = (this->rotationAxis * this->angularSpeed) + (n * acc);
    this->rotationAxis.renormalize();
    //cout << "acc = " << acc << ", rotationAxis = ";
    //this->rotationAxis.output();

    // Increase angular speed
    this->angularSpeed += acc;
}

void morph::Visual::timerEvent ()
{
    // Decrease angular speed (friction)
    this->angularSpeed *= 0.95;

    // Stop rotation when speed goes below threshold
    if (this->angularSpeed < 0.01) {
        this->angularSpeed = 0.0;
    } else {
        // Update rotation
        Quaternion<float> rotationQuaternion;
        rotationQuaternion.initFromAxisAngle (this->rotationAxis, this->angularSpeed);
        this->rotation.premultiply (rotationQuaternion);
        //this->rotation.output();

        // Request an update
        this->render();
    }
}

void
morph::Visual::setPerspective (void)
{
    // Obtain window size
    int w, h;
    glfwGetWindowSize (this->window, &w, &h);
    // Calculate aspect ratio
    float aspect = float(w) / float(h ? h : 1);
    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const float zNear = 0.5, zFar = 10.0, fov = 65.0;
    // Reset projection
    this->projection.setToIdentity();
    // Set perspective projection
    this->projection.perspective (fov, aspect, zNear, zFar);
    //this->projection.output();
}

void
morph::Visual::render (void)
{
    const double retinaScale = 1; // devicePixelRatio()?
    int w, h;
    glfwGetWindowSize (this->window, &w, &h);
    glViewport (0, 0, w * retinaScale, h * retinaScale);

    // Set the perspective from the width/height
    this->setPerspective();

    // Calculate model view transformation
    TransformMatrix<float> rotmat;
    rotmat.translate (0.0, 0.0, -3.50); // send backwards into distance
    rotmat.rotate (this->rotation);

    // Bind shader program...
    //this->shaderProg->bind();

    // Set modelview-projection matrix
    TransformMatrix<float> pr = this->projection * rotmat;
    //cout << "Query shaderprog "  << this->shaderprog << " for mvp_matrix location" << endl;
    GLint loc = glGetUniformLocation (this->shaderprog, (const GLchar*)"mvp_matrix");
    if (loc == -1) {
        cout << "No mvp_matrix? loc: " << loc << endl;
    } else {
        //cout << "mvp_matrix loc: " << loc << endl;
#if 0
        Quaternion<float> dummy;
        dummy.initFromAxisAngle (Vector3<float>(0,1,0), 23);
        dummy.renormalize();
        array<float, 16> arr;
        dummy.rotationMatrix (arr);
        cout << "| " << arr[0] << " , " << arr[4] << " , " << arr[8] << " , " << arr[12] << " |\n";
        cout << "| " << arr[1] << " , " << arr[5] << " , " << arr[9] << " , " << arr[13] << " |\n";
        cout << "| " << arr[2] << " , " << arr[6] << " , " << arr[10] << " , " << arr[14] << " |\n";
        cout << "| " << arr[3] << " , " << arr[7] << " , " << arr[11] << " , " << arr[15] << " |\n";
        cout << "arr.data()[0]: " << arr.data()[0] << endl;
        glUniformMatrix4fv (loc, 1, GL_FALSE, arr.data());
#endif
#if 0
        TransformMatrix<float> dummy;
        dummy.translate(0, 0, -3);
        glUniformMatrix4fv (loc, 1, GL_FALSE, dummy.mat.data());
#endif

        // Original:
        //glUniformMatrix4fv (loc, 1, GL_FALSE, pr.mat.data());
    }

    // Clear color buffer and **also depth buffer**
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //static const float white[] = { 0.0f, 1.0f, 1.0f, 0.5f };
    //glClearBufferfv (GL_COLOR, 0, white); // This line works...

    // Render it.
#if 0
    vector<HexGridVisual*>::iterator hgvi = this->hexGridVis.begin();
    while (hgvi != this->hexGridVis.end()) {
        (*hgvi)->render();
        ++hgvi;
    }
#endif

    vector<TriangleVisual*>::iterator tvi = this->triangleVis.begin();
    while (tvi != this->triangleVis.end()) {
        (*tvi)->render();
        ++tvi;
    }

    glfwSwapBuffers (this->window);

    // release shader if it was bound
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

unsigned int
morph::Visual::addTriangleVisual (void)
{
    // Copy x/y positions from the HexGrid and make a copy of the data as vertices.
    TriangleVisual* tv1 = new TriangleVisual(this->shaderprog);
    this->triangleVis.push_back (tv1);

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

    GLboolean shaderCompilerPresent = GL_FALSE;
    glGetBooleanv (GL_SHADER_COMPILER, &shaderCompilerPresent);
    if (shaderCompilerPresent == GL_FALSE) {
        cerr << "shader compiler NOT present: " << shaderCompilerPresent << endl;
    } else {
        cout << "shader compiler present: " << shaderCompilerPresent << endl;
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
        } else {
            cout << "shader compiled" << endl;
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
    } else {
        cout << "Good, shader is linked." << endl;
    }

    return program;
}

/*!
 * GLFW callback functions
 */
//@{

void
morph::Visual::errorCallback (int error, const char* description)
{
    cerr << "Error: " << description << " (code "  << error << ")" << endl;
}

void
morph::Visual::key_callback (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        cout << "User requested exit." << endl;
        this->readyToFinish = true;
    }
    // Could have several other actions:
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        cout << "Autoscale?" << endl;
    }
}

void
morph::Visual::mouse_button_callback (GLFWwindow* window, int button, int action, int mods)
{
    // button is the button number, action is either key press (1) or key release (0)
    cout << "button " << button;
    //if (button == 0) { // Do we care which button? Not for now.
    if (action == 1) {
        // press means start a drag
        cout << " mouse press" << endl;
        this->mousePressEvent();
    } else if (action == 0) {
        // release
        cout << " mouse release" << endl;
        this->mouseReleaseEvent();
    }
    //}
}

void
morph::Visual::cursor_position_callback (GLFWwindow* window, double x, double y)
{
    this->cursorx = x;
    this->cursory = y;
}

void
morph::Visual::window_size_callback (GLFWwindow* window, int width, int height)
{
    cout << "window size" << endl;
}

//@}
