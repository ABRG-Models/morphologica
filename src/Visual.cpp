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

#include "tools.h"

using morph::ShaderInfo;

// Include the character constants containing the default shaders
#include "VisualDefaultShaders.h"

#include "Vector4.h"
using morph::Vector4;

// imwrite() from OpenCV is used in saveImage()
#include <opencv2/opencv.hpp>

morph::Visual::Visual(int width, int height, const string& title)
    : window_w(width)
    , window_h(height)
{
  GLenum err = glewInit();
  if (GLEW_OK != err) {
      /* Problem: glewInit failed, something is seriously wrong. */
      fprintf (stderr, "Error: %s\n", glewGetErrorString(err));
  }
  fprintf (stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
 
    if (!glfwInit()) {
        // Initialization failed
        cerr << "GLFW initialization failed!" << endl;
    }

    // Set up error callback
    glfwSetErrorCallback (morph::Visual::errorCallback);

    // See https://www.glfw.org/docs/latest/monitor_guide.html
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(primary, &xscale, &yscale);
    cout << "Monitor xscale: " << xscale << ", monitor yscale: " << yscale << endl;

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
    glfwSetWindowSizeCallback (this->window,  VisualBase::window_size_callback_dispatch);
    glfwSetScrollCallback (this->window,  VisualBase::scroll_callback_dispatch);

    glfwMakeContextCurrent (this->window);

    // Swap as fast as possible (fixes lag of scene with mouse movements)
    glfwSwapInterval (0);

    // Load up the shaders
    ShaderInfo shaders[] = {
        {GL_VERTEX_SHADER, "Visual.vert.glsl" },
        {GL_FRAGMENT_SHADER, "Visual.frag.glsl" },
        {GL_NONE, NULL }
    };

    this->shaderprog = this->LoadShaders (shaders);

    // shaderprog is bound here, and never unbound
    glUseProgram (this->shaderprog);

    // Now client code can set up HexGridVisuals.
    glEnable (GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glDisable(GL_DEPTH_TEST);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    this->coordArrows = new CoordArrows(this->shaderprog, coordArrowsOffset, coordArrowsLength);
}

morph::Visual::~Visual()
{
    // FIXME: delete hgv_float, hgv_double and coordArrows.
    delete this->coordArrows;
    glfwDestroyWindow (this->window);
    glfwTerminate();
}

void
morph::Visual::saveImage (const string& filename)
{
    glfwMakeContextCurrent (this->window);
    GLubyte* bits; // RGB bits
    GLint viewport[4]; // current viewport
    glGetIntegerv (GL_VIEWPORT, viewport);
    int w = viewport[2];
    int h = viewport[3];
    bits = new GLubyte[w*h*3];
    glFinish(); // finish all commands of OpenGL
    glPixelStorei (GL_PACK_ALIGNMENT,1);
    glPixelStorei (GL_PACK_ROW_LENGTH, 0);
    glPixelStorei (GL_PACK_SKIP_ROWS, 0);
    glPixelStorei (GL_PACK_SKIP_PIXELS, 0);
    glReadPixels (0, 0, w, h, GL_BGR_EXT, GL_UNSIGNED_BYTE, bits);
    cv::Mat capImg (h, w, CV_8UC3); // 3 channels, 8 bits
    cv::Vec3b triplet;
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            triplet[0] = (unsigned char)(bits[(h-i-1)*3*w + j*3+0]);
            triplet[1] = (unsigned char)(bits[(h-i-1)*3*w + j*3+1]);
            triplet[2] = (unsigned char)(bits[(h-i-1)*3*w + j*3+2]);
            capImg.at<cv::Vec3b>(i,j) = triplet;
        }
    }
    imwrite (filename.c_str(), capImg);
    delete[] bits;
}

void
morph::Visual::setPerspective (void)
{
    // Calculate aspect ratio
    float aspect = float(this->window_w) / float(this->window_h ? this->window_h : 1);
    // Reset projection
    this->projection.setToIdentity();
    // Set perspective projection
    this->projection.perspective (this->fov, aspect, this->zNear, this->zFar);
    // Compute the inverse projection matrix
    this->invproj = this->projection.invert();
}

#ifdef PROFILE_RENDER
// Rendering takes 16 ms if (that's 60 Hz). With no vsync it's <200 us and typically
// 130 us on corebeast (i9 and GTX1080).
#include <chrono>
using namespace std::chrono;
using std::chrono::steady_clock;
#endif

void
morph::Visual::keepOpen (void)
{
    while (this->readyToFinish == false) {
        glfwWaitEventsTimeout (0.01667); // 16.67 ms ~ 60 Hz
        this->render();
    }
}

void
morph::Visual::render (void)
{
#ifdef PROFILE_RENDER
    steady_clock::time_point renderstart = steady_clock::now();
#endif
    // Can avoid this by getting window size into members only when window size changes.
    const double retinaScale = 1; // devicePixelRatio()?

    // Can't do this in a new thread:
    glViewport (0, 0, this->window_w * retinaScale, this->window_h * retinaScale);

    // Set the perspective from the width/height
    this->setPerspective();

    // rotmat is the translation/rotation for the entire scene.
    //
    // Calculate model view transformation - transforming from "model space" to "worldspace".
    TransformMatrix<float> sceneview;
    // This line translates from model space to world space. In future may need one
    // model->world for each HexGridVisual.
    sceneview.translate (this->scenetrans); // send backwards into distance
    // And this rotation completes the transition from model to world
    sceneview.rotate (this->rotation);

    // Clear color buffer and **also depth buffer**
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Can set a background colour:
    //static const float white[] = { 1.0f, 1.0f, 1.0f, 0.5f };
    //glClearBufferfv (GL_COLOR, 0, white); // This line works...

    // Render it.

    // First, the coordinates thing.

#if 0 // Find out the location of the bottom left of the screen and make the coord
      // arrows stay put there.
    Vector2<float> p0_coord;
    p0_coord.x = 0.4f;
    p0_coord.y = 0.4f;

    // Add the depth at which the object lies.  Use forward projection to determine
    // the correct z coordinate for the inverse projection. This assumes only one
    // object.
    array<float, 4> point =  { 0.0, 0.0, this->scenetrans.z, 1.0 };
    array<float, 4> pp = this->projection * point;
    float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.

    cout << "Putting coords at coord-z: " << coord_z << endl;

    // Construct the point for the location of the coord arrows
    array<float, 4> p0 = { p0_coord.x, p0_coord.y, coord_z, 1.0 };

    // Inverse project
    array<float, 4> v0 = this->invproj * p0;

    // Apply to view matrix...
    this->coordArrows->viewmatrix.setToIdentity();
    this->coordArrows->viewmatrix.translate (v0);
    this->coordArrows->viewmatrix.rotate (this->rotation);

    TransformMatrix<float> vp_coords = this->projection * this->coordArrows->viewmatrix;
#else
    TransformMatrix<float> vp_coords = this->projection * sceneview * this->coordArrows->viewmatrix;
#endif

    GLint loc = glGetUniformLocation (this->shaderprog, (const GLchar*)"mvp_matrix");
    if (loc != -1) {
        glUniformMatrix4fv (loc, 1, GL_FALSE, vp_coords.mat.data());
    }
    this->coordArrows->render();

    typename vector<HexGridVisual<float>*>::iterator hgvf = this->hgv_float.begin();
    while (hgvf != this->hgv_float.end()) {
        // For each different HexGridVisual, I can CHANGE the uniform. Right? Right.
        TransformMatrix<float> viewproj = this->projection * sceneview * (*hgvf)->viewmatrix;
        GLint loc = glGetUniformLocation (this->shaderprog, (const GLchar*)"mvp_matrix");
        if (loc == -1) {
            cout << "No mvp_matrix? loc: " << loc << endl;
        } else {
            // Set the uniform:
            glUniformMatrix4fv (loc, 1, GL_FALSE, viewproj.mat.data());
        }

        (*hgvf)->render();
        ++hgvf;
    }
    typename vector<HexGridVisual<double>*>::iterator hgvd = this->hgv_double.begin();
    while (hgvd != this->hgv_double.end()) {
        TransformMatrix<float> viewproj = this->projection * sceneview * (*hgvd)->viewmatrix;
        GLint loc = glGetUniformLocation (this->shaderprog, (const GLchar*)"mvp_matrix");
        if (loc == -1) {
            cout << "No mvp_matrix? loc: " << loc << endl;
        } else {
            // Set the uniform:
            glUniformMatrix4fv (loc, 1, GL_FALSE, viewproj.mat.data());
        }
        (*hgvd)->render();
        ++hgvd;
    }
    typename vector<QuadsVisual<float>*>::iterator qvf = this->qv_float.begin();
    while (qvf != this->qv_float.end()) {
        TransformMatrix<float> viewproj = this->projection * sceneview * (*qvf)->viewmatrix;
        GLint loc = glGetUniformLocation (this->shaderprog, (const GLchar*)"mvp_matrix");
        if (loc == -1) {
            cout << "No mvp_matrix? loc: " << loc << endl;
        } else {
            // Set the uniform:
            glUniformMatrix4fv (loc, 1, GL_FALSE, viewproj.mat.data());
        }

        (*qvf)->render();
        ++qvf;
    }

    glfwSwapBuffers (this->window);

#ifdef PROFILE_RENDER
    steady_clock::time_point renderend = steady_clock::now();
    steady_clock::duration time_span = renderend - renderstart;
    cout << "Render took " << duration_cast<microseconds>(time_span).count() << " us" << endl;
#endif
}

void
morph::Visual::updateHexGridVisual (const unsigned int gridId,
                                    const vector<float>& data,
                                    const array<float, 4> scale)
{
    unsigned int idx = gridId & 0xffff;
    this->hgv_float[idx]->updateData (&data, scale);
}

void
morph::Visual::updateHexGridVisual (const unsigned int gridId,
                                    const vector<double>& data,
                                    const array<double, 4> scale)
{
    unsigned int idx = gridId & 0xffff;
    this->hgv_double[idx]->updateData (&data, scale);
}

unsigned int
morph::Visual::addHexGridVisual (const HexGrid* hg,
                                 const array<float, 3> offset,
                                 const vector<float>& data,
                                 const array<float, 4> scale)
{
    // Copy x/y positions from the HexGrid and make a copy of the data as vertices.
    HexGridVisual<float>* hgv1 = new HexGridVisual<float>(this->shaderprog, hg, offset, &data, scale);
    this->hgv_float.push_back (hgv1);
    // Create the return ID
    unsigned int rtn = 0x10000; // 0x10000 denotes "member of hgv_float"
    rtn |= (this->hgv_float.size()-1);
    return rtn;
}

unsigned int
morph::Visual::addHexGridVisual (const HexGrid* hg,
                                 const array<float, 3> offset,
                                 const vector<double>& data,
                                 const array<double, 4> scale)
{
    // Double precision version of the above
    HexGridVisual<double>* hgv1 = new HexGridVisual<double>(this->shaderprog, hg, offset, &data, scale);
    this->hgv_double.push_back (hgv1);
    unsigned int rtn = 0x20000; // 0x20000 denotes "member of hgv_double"
    rtn |= (this->hgv_double.size()-1);
    return rtn;
}

unsigned int
morph::Visual::addQuadsVisual (const vector<array<float, 12>>* quads,
                               const array<float, 3> offset,
                               const vector<float>& data,
                               const array<float, 2> scale)
{
    // Copy x/y positions from the HexGrid and make a copy of the data as vertices.
    QuadsVisual<float>* qv1 = new QuadsVisual<float>(this->shaderprog, quads, offset, &data, scale);
    this->qv_float.push_back (qv1);
    // Create the return ID
    unsigned int rtn = 0x30000; // 0x10000 denotes "member of qv_float"
    rtn |= (this->qv_float.size()-1);
    return rtn;
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

const GLchar*
morph::Visual::ReadDefaultShader (const char* shadercontent)
{
    int len = strlen (shadercontent);
    GLchar* source = new GLchar[len+1];

    memcpy ((void*)source, (void*)shadercontent, len);
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
        cerr << "Shader compiler NOT present!" << endl;
    } else {
        cout << "Shader compiler present" << endl;
    }

    ShaderInfo* entry = shaders;
    while (entry->type != GL_NONE) {
        GLuint shader = glCreateShader (entry->type);
        entry->shader = shader;
        // Test entry->filename. If this GLSL file can be read, then do so, otherwise,
        // compile the default version from VisualDefaultShaders.h
        const GLchar* source;
        if (morph::Tools::fileExists (string(entry->filename))) {
            cout << "Using shader from the file " << entry->filename << endl;
            source = morph::Visual::ReadShader (entry->filename);
        } else {
            if (entry->type == GL_VERTEX_SHADER) {
                cout << "Using compiled-in vertex shader" << endl;
                source = morph::Visual::ReadDefaultShader (defaultVtxShader);
            } else if (entry->type == GL_FRAGMENT_SHADER) {
                cout << "Using compiled-in fragment shader" << endl;
                source = morph::Visual::ReadDefaultShader (defaultFragShader);
            } else {
                cerr << "Visual::LoadShaders: Unknown shader entry->type..." << endl;
                source = NULL;
            }
        }
        if (source == NULL) {
            for (entry = shaders; entry->type != GL_NONE; ++entry) {
                glDeleteShader (entry->shader);
                entry->shader = 0;
            }
            return 0;
#ifdef DEBUG
        } else {
            cout << "Compiling this shader: " << endl << "-----" << endl;
            cout << source << "-----" << endl;
#endif
        }
        GLint slen = (GLint)strlen (source);
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
            cout << "compiled is GL_FALSE. log length is " << len
                 << " compiled has value " << compiled <<  endl;
            if (len > 0) {
                GLchar* log = new GLchar[len+1];
                glGetShaderInfoLog (shader, len, &len, log);
                std::cerr << "Shader compilation failed: " << (char*)log << std::endl;
                delete [] log;
            }
            return 0;
#ifdef DEBUG
        } else {
            cout << "shader compiled" << endl;
#endif
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
    // Exit action
    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        cout << "User requested exit." << endl;
        this->readyToFinish = true;
    }

    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        this->rotateModMode = !this->rotateModMode;
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        this->saveImage ("./picture.png");
        cout << "Took a snap" << endl;
    }

    // Reset view to default
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        cout << "Reset to default view" << endl;
        // Reset translation
        this->scenetrans = this->scenetrans_default;
        // Reset rotation
        Quaternion<float> rt;
        this->rotation = rt;

        this->render();
    }

    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        this->fov -= 2;
        if (this->fov < 1.0) {
            this->fov = 2.0;
        }
        cout << "FOV reduced to " << this->fov << endl;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        this->fov += 2;
        if (this->fov > 179.0) {
            this->fov = 178.0;
        }
        cout << "FOV increased to " << this->fov << endl;
    }
    if (key == GLFW_KEY_U && action == GLFW_PRESS) {
        this->zNear /= 2;
        cout << "zNear reduced to " << this->zNear << endl;
    }
    if (key == GLFW_KEY_I && action == GLFW_PRESS) {
        this->zNear *= 2;
        cout << "zNear increased to " << this->zNear << endl;
    }
}

void
morph::Visual::mouse_button_callback (GLFWwindow* window, int button, int action, int mods)
{
    // button is the button number, action is either key press (1) or key release (0)
    // cout << "button: " << button << " action: " << (action==1?("press"):("release")) << endl;

    // Record the position at which the button was pressed
    if (action == 1) { // Button down
        this->mousePressPosition = this->cursorpos;
        // Save the rotation at the start of the mouse movement
        this->savedRotation = this->rotation;
        // Get the scene's rotation at the start of the mouse movement:
        this->scene.setToIdentity();
        this->scene.rotate (this->savedRotation);
        this->invscene = this->scene.invert();
    }

#if 0
    if (action == 0 && button == 0) {
        // This is release of the rotation button
        this->mousePressPosition = this->cursorpos;
    }
#endif

    if (button == 0) { // Primary button means rotate
        this->rotateMode = (action == 1);
    } else if (button == 1) { // Secondary button means translate
        this->translateMode = (action == 1);
    }
}

void
morph::Visual::cursor_position_callback (GLFWwindow* window, double x, double y)
{
    this->cursorpos.x = static_cast<float>(x);
    this->cursorpos.y = static_cast<float>(y);

    Vector3<float> mouseMoveWorld;

    // This is "rotate the scene" model. Will need "rotate one visual" mode.
    if (this->rotateMode) {
        // Convert mousepress/cursor positions (in pixels) to the range -1 -> 1:
        Vector2<float> p0_coord = this->mousePressPosition;
        p0_coord.x -= this->window_w/2.0;
        p0_coord.x /= this->window_w/2.0;
        p0_coord.y -= this->window_h/2.0;
        p0_coord.y /= this->window_h/2.0;
        Vector2<float> p1_coord = this->cursorpos;
        p1_coord.x -= this->window_w/2.0;
        p1_coord.x /= this->window_w/2.0;
        p1_coord.y -= this->window_h/2.0;
        p1_coord.y /= this->window_h/2.0;

        // DON'T update mousePressPosition until user releases button.
        // this->mousePressPosition = this->cursorpos;

        // Add the depth at which the object lies.  Use forward projection to determine
        // the correct z coordinate for the inverse projection. This assumes only one
        // object.
        array<float, 4> point =  { 0.0, 0.0, this->scenetrans.z, 1.0 };
        array<float, 4> pp = this->projection * point;
        float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.

        // Construct two points for the start and end of the mouse movement
        array<float, 4> p0 = { p0_coord.x, p0_coord.y, coord_z, 1.0 };
        array<float, 4> p1 = { p1_coord.x, p1_coord.y, coord_z, 1.0 };

        // Apply the inverse projection to get two points in the world frame of
        // reference for the mouse movement
        array<float, 4> v0 = this->invproj * p0;
        array<float, 4> v1 = this->invproj * p1;

        // This computes the difference betwen v0 and v1, the 2 mouse positions in the
        // world space. Note the swap between x and y
        if (this->rotateModMode) {
            // Sort of "rotate the page" mode.
            mouseMoveWorld.z = -((v1[1]/v1[3]) - (v0[1]/v0[3])) + ((v1[0]/v1[3]) - (v0[0]/v0[3]));
        } else {
            mouseMoveWorld.y = -((v1[0]/v1[3]) - (v0[0]/v0[3]));
            mouseMoveWorld.x = -((v1[1]/v1[3]) - (v0[1]/v0[3]));
        }

        // Rotation axis is perpendicular to the mouse position difference vector
        // BUT we have to project into the model frame to determine how to rotate the model!
        float rotamount = mouseMoveWorld.length() * 40.0;
        // Calculate new rotation axis as weighted sum
        this->rotationAxis = (mouseMoveWorld * rotamount);
        this->rotationAxis.renormalize();

        // Now inverse apply the rotation of the scene to the rotation axis, so that we
        // rotate the model the right way.
        this->rotationAxis = this->invscene * this->rotationAxis;

        // Update rotation from the saved position.
        this->rotation = this->savedRotation;
        Quaternion<float> rotationQuaternion;
        rotationQuaternion.initFromAxisAngle (this->rotationAxis, rotamount);
        this->rotation.premultiply (rotationQuaternion); // combines rotations
        this->render(); // updates viewproj; uses this->rotation

    } else if (this->translateMode) { // allow only rotate OR translate for a single mouse movement

        // Convert mousepress/cursor positions (in pixels) to the range -1 -> 1:
        Vector2<float> p0_coord = this->mousePressPosition;
        p0_coord.x -= this->window_w/2.0;
        p0_coord.x /= this->window_w/2.0;
        p0_coord.y -= this->window_h/2.0;
        p0_coord.y /= this->window_h/2.0;
        Vector2<float> p1_coord = this->cursorpos;
        p1_coord.x -= this->window_w/2.0;
        p1_coord.x /= this->window_w/2.0;
        p1_coord.y -= this->window_h/2.0;
        p1_coord.y /= this->window_h/2.0;

        this->mousePressPosition = this->cursorpos;

        // Add the depth at which the object lies.  Use forward projection to determine
        // the correct z coordinate for the inverse projection. This assumes only one
        // object.
        array<float, 4> point =  { 0.0, 0.0, this->scenetrans.z, 1.0 };
        array<float, 4> pp = this->projection * point;
        float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.

        // Construct two points for the start and end of the mouse movement
        array<float, 4> p0 = { p0_coord.x, p0_coord.y, coord_z, 1.0 };
        array<float, 4> p1 = { p1_coord.x, p1_coord.y, coord_z, 1.0 };
        // Apply the inverse projection to get two points in the world frame of reference:
        array<float, 4> v0 = this->invproj * p0;
        array<float, 4> v1 = this->invproj * p1;
        // This computes the difference betwen v0 and v1, the 2 mouse positions in the world
        mouseMoveWorld.x = (v1[0]/v1[3]) - (v0[0]/v0[3]);
        mouseMoveWorld.y = (v1[1]/v1[3]) - (v0[1]/v0[3]);
        //mouseMoveWorld.z = (v1[2]/v1[3]) - (v0[2]/v0[3]);// unmodified

        // This is "translate the scene" mode. Could also have a "translate one
        // HexGridVisual" mode, to adjust relative positions.
        this->scenetrans.x += mouseMoveWorld.x;
        this->scenetrans.y -= mouseMoveWorld.y;
        this->render(); // updates viewproj; uses this->scenetrans
    }
}

void
morph::Visual::window_size_callback (GLFWwindow* window, int width, int height)
{
    this->window_w = width;
    this->window_h = height;
    this->render();
}

void
morph::Visual::scroll_callback (GLFWwindow* window, double xoffset, double yoffset)
{
    // x and y can be +/- 1
    this->scenetrans.x -= xoffset * this->scenetrans_stepsize;
    if (this->translateMode) {
        this->scenetrans.y/*z really*/ += yoffset * this->scenetrans_stepsize;
        cout << "scenetrans.y = " << this->scenetrans.y << endl;
    } else {
        this->scenetrans.z += yoffset * this->scenetrans_stepsize;
    }
    this->render();
}
//@}
