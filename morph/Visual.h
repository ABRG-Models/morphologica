/*!
 * \file
 *
 * Graphics code. A replacement for morph::Gdisplay from display.h. Uses modern OpenGL
 * and the library GLFW for window management.
 *
 * Created by Seb James on 2019/05/01
 *
 * \author Seb James
 * \date May 2019
 */
#pragma once

#ifdef USE_GLEW
// Including glew.h and linking with libglew helps older platforms,
// such as Ubuntu 16.04. Not necessary on later platforms.
# include <GL/glew.h>
#endif

#include "morph/VisualModel.h"
#include <morph/VisTextModel.h>
#include <morph/VisualCommon.h>
// Include glfw3 AFTER VisualModel
#include <GLFW/glfw3.h>
// For GLuint and GLenum (though redundant, as already included in VisualModel
#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include "GL3/gl3.h"
#endif

#include "morph/HexGrid.h"
#include "morph/HexGridVisual.h"
#include "morph/QuadsVisual.h"
#include "morph/PointRowsVisual.h"
#include "morph/ScatterVisual.h"
#include "morph/QuiverVisual.h"
#include "morph/CoordArrows.h"
#ifdef TRIANGLE_VIS_TESTING
# include "morph/TriangleVisual.h"
#endif
#include "morph/Quaternion.h"
#include "morph/TransformMatrix.h"
#include "morph/Vector.h"
// A base class with static event handling dispatchers
#include "morph/VisualBase.h"
#include "morph/ColourMap.h"

#include <string>
#include <array>
#include <vector>

#include "morph/VisualDefaultShaders.h"
#include "morph/VisualModel.h"

// imwrite() from OpenCV is used in saveImage()
#include <opencv2/imgcodecs.hpp>

// FreeType for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

//! The default z=0 position for VisualModels
#define Z_DEFAULT -5

#ifdef PROFILE_RENDER
// Rendering takes 16 ms (that's 60 Hz). With no vsync it's <200 us and typically
// 130 us on corebeast (i9 and GTX1080).
#include <chrono>
using namespace std::chrono;
using std::chrono::steady_clock;
#endif

namespace morph {

    /*!
     * Data structure for shader info.
     *
     * LoadShaders() takes an array of ShaderFile structures, each of
     * which contains the type of the shader, and a pointer a C-style
     * character string (i.e., a NULL-terminated array of characters)
     * containing the entire shader source.
     *
     * The array of structures is terminated by a final Shader with
     * the "type" field set to GL_NONE.
     *
     * LoadShaders() returns the shader program value (as returned by
     * glCreateProgram()) on success, or zero on failure.
     */
    typedef struct
    {
        GLenum type;
        const char* filename;
        GLuint shader;
    } ShaderInfo;

    // To enable debugging, set true.
    const bool debug_shaders = false;

    /*!
     * Visual 'scene' class
     *
     * A class for visualising computational models on an OpenGL screen. Will be
     * specialised for rendering HexGrids to begin with.
     *
     * Each Visual will have its own GLFW window and is essentially a "scene" containing
     * a number of objects. One object might be the visualisation of some data expressed
     * over a HexGrid. It should be possible to translate objects with respect to each
     * other and also to rotate the entire scene, as well as use keys to generate
     * particular effects/views.
     *
     * It's possible to set the background colour of the scene (Visual::bgcolour), the
     * location of the objects in the scene (Visual::setSceneTransZ and friends) and the
     * position and field of view of the 'camera' (Visual::zNear, Visual::zFar and
     * Visual::fov).
     */
    class Visual : VisualBase
    {
    public:
        /*!
         * Construct a new visualiser. The rule is 1 window to one Visual object. So,
         * this creates a new window and a new OpenGL context.
         */
        Visual (int width, int height, const std::string& title)
            : window_w(width)
            , window_h(height)
        {
            this->init (title);
        }

        /*!
         * Construct with specified coordinate arrows offset (caOffset) lengths
         * (caLength) and thickness scaling factor (caThickness)
         */
        Visual (int width, int height, const std::string& title,
                const Vector<float> caOffset, const Vector<float> caLength, const float caThickness)
            : window_w(width)
            , window_h(height)
            , coordArrowsOffset(caOffset)
            , coordArrowsLength(caLength)
            , coordArrowsThickness(caThickness)
        {
            this->init (title);
        }

        //! Deconstructor deallocates CoordArrows and destroys GLFW windows
        ~Visual()
        {
            // Deconstructor?
            FT_Done_Face (this->face);
            FT_Done_FreeType (this->ft);

            delete this->coordArrows;
            for (unsigned int i = 0; i < this->vm.size(); ++i) {
                delete this->vm[i];
            }
            glfwDestroyWindow (this->window);
            glfwTerminate();
        }

        //! An error callback function for the GLFW windowing library
        static void errorCallback (int error, const char* description)
        {
            std::cerr << "Error: " << description << " (code "  << error << ")\n";
        }

        //! Take a screenshot of the window
        void saveImage (const std::string& img_filename)
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
            glReadPixels (0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, bits);
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
            imwrite (img_filename.c_str(), capImg);
            delete[] bits;
        }

        /*!
         * Add a VisualModel to the scene. The VisualModel* should be a pointer to a
         * visual model which has been newly allocated by the client code. Do not add
         * a pointer to the same VisualModel more than once! When this morph::Visual
         * object goes out of scope, its deconstructor will delete each VisualModel
         * that it has a pointer for.
         */
        unsigned int addVisualModel (VisualModel* model)
        {
            this->vm.push_back (model);
            unsigned int rtn = (this->vm.size()-1);
            return rtn;
        }

        /*!
         * VisualModel Getter
         *
         * For the given \a modelId, return a pointer to the visual model.
         *
         * \return VisualModel pointer
         */
        VisualModel* getVisualModel (unsigned int modelId) { return (this->vm[modelId]); }

        //! Remove the VisualModel with ID \a modelId from the scene.
        void removeVisualModel (unsigned int modelId)
        {
            delete this->vm[modelId];
            this->vm.erase (this->vm.begin() + modelId);
        }

        /*!
         * Keep on rendering until readToFinish is set true. Used to keep a window
         * open, and responsive, while displaying the result of a simulation.
         */
        void keepOpen()
        {
            while (this->readyToFinish == false) {
                glfwWaitEventsTimeout (0.01667); // 16.67 ms ~ 60 Hz
                this->render();
            }
        }

        //! Render the scene
        void render()
        {
#ifdef PROFILE_RENDER
            steady_clock::time_point renderstart = steady_clock::now();
#endif

#ifdef __OSX__
            // https://stackoverflow.com/questions/35715579/opengl-created-window-size-twice-as-large
            const double retinaScale = 2; // deals with quadrant issue on osx
#else
            const double retinaScale = 1; // Qt has devicePixelRatio() to get retinaScale.
#endif
            glUseProgram (this->shaderprog);

            // Can't do this in a new thread:
            glViewport (0, 0, this->window_w * retinaScale, this->window_h * retinaScale);

#if 0 // An alternative to the above, using glfw to get the framebuffer size (see
      // https://www.glfw.org/docs/latest/window_guide.html#window_fbsize)
            int fb_width, fb_height;
            glfwGetFramebufferSize (window, &fb_width, &fb_height);
            glViewport(0, 0, fb_width, fb_height);
#endif

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

            // Set the background colour:
            glClearBufferfv (GL_COLOR, 0, bgcolour.data());

            // Render it.

            // First, the coordinates thing. Ensure coordarrows centre sphere will be visible
            // on BG:
            this->coordArrows->setColourForBackground (this->bgcolour);
#if 0 // Find out the location of the bottom left of the screen and make the coord
      // arrows stay put there.
            Vector<float, 2> p0_coord = {0.4f, 0.4f};

            // Add the depth at which the object lies.  Use forward projection to determine
            // the correct z coordinate for the inverse projection. This assumes only one
            // object.
            Vector<float, 4> point =  { 0.0, 0.0, this->scenetrans.z(), 1.0 };
            Vector<float, 4> pp = this->projection * point;
            float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.

            std::cout << "Putting coords at coord-z: " << coord_z << std::endl;

            // Construct the point for the location of the coord arrows
            Vector<float, 4> p0 = { p0_coord.x, p0_coord.y, coord_z, 1.0 };

            // Inverse project
            Vector<float, 4> v0 = this->invproj * p0;

            // Apply to view matrix...
            this->coordArrows->viewmatrix.setToIdentity();
            this->coordArrows->viewmatrix.translate (v0);
            this->coordArrows->viewmatrix.rotate (this->rotation);

            TransformMatrix<float> vp_coords = this->projection * this->coordArrows->viewmatrix;
#else
            TransformMatrix<float> vp_coords = this->projection * sceneview * this->coordArrows->viewmatrix;
#endif

            // Set the view matrix...
            GLint loc_v = glGetUniformLocation (this->shaderprog, (const GLchar*)"v_matrix");
            if (loc_v != -1) { glUniformMatrix4fv (loc_v, 1, GL_FALSE, sceneview.mat.data()); }

            // and the projection matrix
            GLint loc_p = glGetUniformLocation (this->shaderprog, (const GLchar*)"p_matrix");
            if (loc_p != -1) { glUniformMatrix4fv (loc_p, 1, GL_FALSE, this->projection.mat.data()); }

            GLint loc_m = glGetUniformLocation (this->shaderprog, (const GLchar*)"m_matrix");

            // Render the coordinate arrows if required
            // Update the coordinate's model-view-projection matrix as a uniform in the GLSL...
            GLint loc = glGetUniformLocation (this->shaderprog, (const GLchar*)"mvp_matrix");
            if (loc != -1) {
                glUniformMatrix4fv (loc, 1, GL_FALSE, vp_coords.mat.data());
            }
            if (this->showCoordArrows == true) {
                this->coordArrows->render();
            }

            typename std::vector<VisualModel*>::iterator vmi = this->vm.begin();
            while (vmi != this->vm.end()) {
                // For each different VisualModel, I can CHANGE the uniform. Right? Right.
                TransformMatrix<float> viewproj = this->projection * sceneview * (*vmi)->viewmatrix;
                GLint loc = glGetUniformLocation (this->shaderprog, (const GLchar*)"mvp_matrix");
                if (loc != -1) {
                    glUniformMatrix4fv (loc, 1, GL_FALSE, viewproj.mat.data());
                }
                if (loc_m != -1) {
                    glUniformMatrix4fv (loc_m, 1, GL_FALSE, (*vmi)->viewmatrix.mat.data());
                }
                (*vmi)->render();
                ++vmi;
            }

            // Now switch to text shader
            glUseProgram (this->tshaderprog);

            // set mvp in tshaderprog too
            vp_coords = this->projection * sceneview * this->textModel->viewmatrix;
            GLint loct = glGetUniformLocation (this->tshaderprog, (const GLchar*)"mvp_matrix");
            if (loct != -1) {
                glUniformMatrix4fv (loct, 1, GL_FALSE, vp_coords.mat.data());
            } else {
                std::cout << "NOT Setting vp_coords in texture shader\n";
            }

            // Text rendering. This probably will be a resizable array of textobjs
            this->textModel->render();

            glfwSwapBuffers (this->window);

#ifdef PROFILE_RENDER
            steady_clock::time_point renderend = steady_clock::now();
            steady_clock::duration time_span = renderend - renderstart;
            std::cout << "Render took " << duration_cast<microseconds>(time_span).count() << " us\n";
#endif
        }

        //! The OpenGL shader program for graphical objects
        GLuint shaderprog;
        //! The text shader program, which uses textures to draw text on quads.
        GLuint tshaderprog;

        //! Set perspective based on window width and height
        void setPerspective()
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

        //! Set to true when the program should end
        bool readyToFinish = false;

        /*
         * User-settable projection values for the near clipping distance, the far
         * clipping distance and the field of view of the camera.
         */

        float zNear = 1.0;
        float zFar = 15.0;
        float fov = 30.0;

        //! Set to true to show the coordinate arrows
        bool showCoordArrows = false;

        //! How big should the steps in scene translation be when scrolling?
        float scenetrans_stepsize = 0.1;

        //! If you set this to true, then the mouse movements won't change scenetrans or rotation.
        bool sceneLocked = false;

        //! The background colour; black by default.
        std::array<float, 4> bgcolour = { 0.0f, 0.0f, 0.0f, 0.0f };

        /*
         * User can directly set bgcolour for any background colour they like, but
         * here are convenience functions:
         */

        //! Set a white background colour for the Visual scene
        void backgroundWhite() { this->bgcolour = { 1.0f, 1.0f, 1.0f, 0.5f }; }
        //! Set a black background colour for the Visual scene
        void backgroundBlack() { this->bgcolour = { 0.0f, 0.0f, 0.0f, 0.0f }; }

        //! Setter for zDefault. Sub called by Visual::setSceneTransZ().
        void setZDefault (float f)
        {
            if (f>0.0f) {
                std::cout << "WARNING setZDefault(): Normally, the default z value is negative.\n";
            }
            this->zDefault = f;
            this->scenetrans[2] = f;
        }

        //! Set the scene's x and y values at the same time.
        void setSceneTransXY (float _x, float _y)
        {
            this->scenetrans[0] = _x;
            this->scenetrans[1] = _y;
        }
        //! Set the scene's y value. Use this to shift your scene objects left or right
        void setSceneTransX (float _x) { this->scenetrans[0] = _x; }
        //! Set the scene's y value. Use this to shift your scene objects up and down
        void setSceneTransY (float _y) { this->scenetrans[1] = _y; }
        //! Set the scene's z value. Use this to bring the 'camera' closer to your scene
        //! objects (that is, your morph::VisualModel objects).
        void setSceneTransZ (float _z)
        {
            if (_z>0.0f) {
                std::cout << "WARNING setSceneTransZ(): Normally, the default z value is negative.\n";
            }
            this->setZDefault (_z);
        }

    protected:
        //! A vector of pointers to all the morph::VisualModels (HexGridVisual,
        //! ScatterVisual, etc) which are going to be rendered in the scene.
        std::vector<VisualModel*> vm;

    private:
        //! Private initialization, used by constructors. \a title sets the window title.
        void init (const std::string& title)
        {
            if (!glfwInit()) { std::cerr << "GLFW initialization failed!\n"; }

            morph::GLutil::checkError (__FILE__, __LINE__);

            // Set up error callback
            glfwSetErrorCallback (morph::Visual::errorCallback);

            // See https://www.glfw.org/docs/latest/monitor_guide.html
            GLFWmonitor* primary = glfwGetPrimaryMonitor();
            float xscale, yscale;
            glfwGetMonitorContentScale(primary, &xscale, &yscale);
            std::cout << "Monitor xscale: " << xscale << ", monitor yscale: " << yscale << std::endl;

            glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 1);
#ifdef __OSX__
            glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
            this->window = glfwCreateWindow (this->window_w, this->window_h, title.c_str(), NULL, NULL);
            if (!this->window) {
                // Window or OpenGL context creation failed
                std::cerr << "GLFW window creation failed!\n";
            }

            // Fix the event handling for benefit of static functions.
            this->setEventHandling();

            // Set up callbacks
            glfwSetKeyCallback (this->window, VisualBase::key_callback_dispatch);
            glfwSetMouseButtonCallback (this->window, VisualBase::mouse_button_callback_dispatch);
            glfwSetCursorPosCallback (this->window, VisualBase::cursor_position_callback_dispatch);
            glfwSetWindowSizeCallback (this->window, VisualBase::window_size_callback_dispatch);
            glfwSetScrollCallback (this->window, VisualBase::scroll_callback_dispatch);

            glfwMakeContextCurrent (this->window);

#ifdef USE_GLEW
            glewExperimental = GL_FALSE;
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                std::cerr << "OpenGL error: " << error << std::endl;
            }
            GLenum err = glewInit();
            if (GLEW_OK != err) {
                std::cerr << "GLEW initialization failed!" << glewGetErrorString(err) << std::endl;
            }
#endif

            // Swap as fast as possible (fixes lag of scene with mouse movements)
            glfwSwapInterval (0);

            // Load up the shaders
            ShaderInfo shaders[] = {
                {GL_VERTEX_SHADER, "Visual.vert.glsl" },
                {GL_FRAGMENT_SHADER, "Visual.frag.glsl" },
                {GL_NONE, NULL }
            };

            this->shaderprog = this->LoadShaders (shaders);

            // May need an additional shader?
            ShaderInfo tshaders[] = {
                {GL_VERTEX_SHADER, "VisText.vert.glsl" },
                {GL_FRAGMENT_SHADER, "VisText.frag.glsl" },
                {GL_NONE, NULL }
            };
            // Care - this will load default shaders in some cases
            this->tshaderprog = this->LoadShaders (tshaders);

            // shaderprog is bound here, and never unbound
            //glUseProgram (this->tshaderprog);

            // Now client code can set up HexGridVisuals.
            glEnable (GL_DEPTH_TEST);

            // Make it possible to specify alpha. This is correct for text texture
            // rendering too.
            glEnable (GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable (GL_CULL_FACE); // text example has glEnable(GL_CULL_FACE)

            morph::GLutil::checkError (__FILE__, __LINE__);

            this->coordArrows = new CoordArrows(this->shaderprog,
                                                this->coordArrowsOffset,
                                                this->coordArrowsLength,
                                                this->coordArrowsThickness);
            morph::GLutil::checkError (__FILE__, __LINE__);

            //
            // Experimental text code
            //
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
            if (FT_Init_FreeType (&this->ft)) {
                std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            }
            morph::GLutil::checkError (__FILE__, __LINE__);

            // Keep the face as a morph::Visual owned resource, shared by VisTextModels
            if (FT_New_Face (this->ft, "fonts/ttf-bitstream-vera/Vera.ttf", 0, &this->face)) {
                std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
            }
            // You have to play with this and *at the same time*, tweak the fontscale argument to VisTextModel::setupText
            FT_Set_Pixel_Sizes (this->face, 0, 192);

            // Set up just ASCII chars for now, following the example prog
            for (unsigned char c = 0; c < 128; c++) {
                // load character glyph
                if (FT_Load_Char (this->face, c, FT_LOAD_RENDER)) {
                    std::cout << "ERROR::FREETYTPE: Failed to load Glyph " << c << std::endl;
                    continue;
                }
                // generate texture
                unsigned int texture;
                glGenTextures (1, &texture);
                glBindTexture (GL_TEXTURE_2D, texture);
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RED,
                    this->face->glyph->bitmap.width,
                    this->face->glyph->bitmap.rows,
                    0,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    this->face->glyph->bitmap.buffer
                    );
                // set texture options
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Could be GL_NEAREST, but doesn't look as good.
                // now store character for later use
                Character character = {
                    texture,
                    {static_cast<int>(this->face->glyph->bitmap.width), static_cast<int>(this->face->glyph->bitmap.rows)}, // Size
                    {this->face->glyph->bitmap_left, this->face->glyph->bitmap_top}, // Bearing
                    static_cast<unsigned int>(this->face->glyph->advance.x)          // Advance
                };
#if 1
                std::cout << "Inserting character in this->Characters with info: ID:" << character.TextureID
                          << ", Size:" << character.Size << ", Bearing:" << character.Bearing
                          << ", Advance:" << character.Advance << std::endl;
#endif
                this->Characters.insert (std::pair<char, Character>(c, character));
            }
            glBindTexture(GL_TEXTURE_2D, 0);

            // At this point could FT_Done_Face() etc, I think.

            // AFTER setting up characters, can now set up text in the textMmodel
            morph::GLutil::checkError (__FILE__, __LINE__);
            this->textModel = new VisTextModel (this->tshaderprog, this->textOffset);
            morph::GLutil::checkError (__FILE__, __LINE__);
            this->textModel->setupText ("morph::Visual", this->Characters, 0.001f);
            morph::GLutil::checkError (__FILE__, __LINE__);

            //
            // Experimental text code end
            //
        }

        //! FreeType library object
        FT_Library ft;
        //! A Visual-default face
        FT_Face face;
        //! A map of char to Character info structs
        std::map<char, Character> Characters;

        //! The default z=0 position for HexGridVisual models
        float zDefault = Z_DEFAULT;

        //! Read a shader from a file.
        const GLchar* ReadShader (const char* filename)
        {
            FILE* infile = fopen (filename, "rb");

            if (!infile) {
                std::cerr << "Unable to open file '" << filename << "'\n";
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

        /*!
         * Read a default shader, stored as a const char* like ReadShader reads a file:
         * allocate some memory, copy the text into the new memory and then return a
         * (GLchar*) pointer to the memory.
         */
        const GLchar* ReadDefaultShader (const char* shadercontent)
        {
            int len = strlen (shadercontent);
            GLchar* source = new GLchar[len+1];

            memcpy ((void*)source, (void*)shadercontent, len);
            source[len] = 0;
            return const_cast<const GLchar*>(source);
        }

        //! Shader loading code
        GLuint LoadShaders (ShaderInfo* shader_info)
        {
            if (shader_info == NULL) { return 0; }

            GLuint program = glCreateProgram();

            GLboolean shaderCompilerPresent = GL_FALSE;
            glGetBooleanv (GL_SHADER_COMPILER, &shaderCompilerPresent);
            if (shaderCompilerPresent == GL_FALSE) {
                std::cerr << "Shader compiler NOT present!\n";
            } else {
                if constexpr (debug_shaders == true) {
                    std::cout << "Shader compiler present\n";
                }
            }

            ShaderInfo* entry = shader_info;
            while (entry->type != GL_NONE) {
                GLuint shader = glCreateShader (entry->type);
                entry->shader = shader;
                // Test entry->filename. If this GLSL file can be read, then do so, otherwise,
                // compile the default version from VisualDefaultShaders.h
                const GLchar* source;
                if (morph::Tools::fileExists (std::string(entry->filename))) {
                    std::cout << "Using shader from the file " << entry->filename << std::endl;
                    source = morph::Visual::ReadShader (entry->filename);
                } else {
                    if (entry->type == GL_VERTEX_SHADER) {
                        if constexpr (debug_shaders == true) {
                            std::cout << "Using compiled-in vertex shader\n";
                        }
                        source = morph::Visual::ReadDefaultShader (defaultVtxShader);
                    } else if (entry->type == GL_FRAGMENT_SHADER) {
                        if constexpr (debug_shaders == true) {
                            std::cout << "Using compiled-in fragment shader\n";
                        }
                        source = morph::Visual::ReadDefaultShader (defaultFragShader);
                    } else {
                        std::cerr << "Visual::LoadShaders: Unknown shader entry->type...\n";
                        source = NULL;
                    }
                }
                if (source == NULL) {
                    for (entry = shader_info; entry->type != GL_NONE; ++entry) {
                        glDeleteShader (entry->shader);
                        entry->shader = 0;
                    }
                    return 0;

                } else {
                    if constexpr (debug_shaders == true) {
                        std::cout << "Compiling this shader: \n" << "-----\n";
                        std::cout << source << "-----\n";
                    }
                }
                GLint slen = (GLint)strlen (source);
                glShaderSource (shader, 1, &source, &slen);
                delete [] source;

                glCompileShader (shader);

                GLint shaderCompileSuccess = GL_FALSE;
                char infoLog[512];
                glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompileSuccess);
                if (!shaderCompileSuccess) {
                    glGetShaderInfoLog(shader, 512, NULL, infoLog);
                    std::cerr << "\nShader compilation failed!";
                    std::cerr << "\n--------------------------\n";
                    std::cerr << infoLog << std::endl;
                    std::cerr << "Exiting.\n";
                    exit (2);
                }

                // Test glGetError:
                GLenum shaderError = glGetError();
                if (shaderError == GL_INVALID_VALUE) {
                    std::cerr << "Shader compilation resulted in GL_INVALID_VALUE\n";
                    exit (3);
                } else if (shaderError == GL_INVALID_OPERATION) {
                    std::cerr << "Shader compilation resulted in GL_INVALID_OPERATION\n";
                    exit (4);
                } // shaderError is 0

                if constexpr (debug_shaders == true) {
                    if (entry->type == GL_VERTEX_SHADER) {
                        std::cout << "Successfully compiled vertex shader!\n";
                    } else if (entry->type == GL_FRAGMENT_SHADER) {
                        std::cout << "Successfully compiled fragment shader!\n";
                    } else {
                        std::cout << "Successfully compiled shader!\n";
                    }
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
                std::cerr << "Shader linking failed: " << log << std::endl << "Exiting.\n";
                delete [] log;
                for (entry = shader_info; entry->type != GL_NONE; ++entry) {
                    glDeleteShader (entry->shader);
                    entry->shader = 0;
                }
                exit (5);

            } else {
                if constexpr (debug_shaders == true) {
                    if (entry->type == GL_VERTEX_SHADER) {
                        std::cout << "Successfully linked vertex shader!\n";
                    } else if (entry->type == GL_FRAGMENT_SHADER) {
                        std::cout << "Successfully linked fragment shader!\n";
                    } else {
                        std::cout << "Successfully linked shader!\n";
                    }
                }
            }

            return program;
        }

        //! The window (and OpenGL context) for this Visual
        GLFWwindow* window;

        //! Current window width
        int window_w;
        //! Current window height
        int window_h;

        //! The user's 'selected visual model'. For model specific changes to alpha and possibly colour
        unsigned int selectedVisualModel = 0;

        //! A little model of the coordinate axes.
        CoordArrows* coordArrows;

        //! Position and length of coordinate arrows. Configurable at morph::Visual construction.
        Vector<float> coordArrowsOffset = {0.0f, 0.0f, 0.0f};
        Vector<float> coordArrowsLength = {1.0f, 1.0f, 1.0f};
        float coordArrowsThickness = 1.0f;

        VisTextModel* textModel;
        Vector<float> textOffset = {0.0f, -0.1f, 0.0f};
        /*
         * Variables to manage projection and rotation of the object
         */

        //! Current cursor position
        Vector<float,2> cursorpos = {0.0f, 0.0f};

        //! Holds the translation coordinates for the current location of the entire scene
        Vector<float> scenetrans = {0.0, 0.0, Z_DEFAULT};

        //! Default for scenetrans. This is a scene position that can be reverted to, to 'reset the view'.
        const Vector<float> scenetrans_default = {0.0, 0.0, Z_DEFAULT};

        //! When true, cursor movements induce rotation of scene
        bool rotateMode = false;

        //! When true, rotations about the third axis are possible.
        bool rotateModMode = false;

        //! When true, cursor movements induce translation of scene
        bool translateMode = false;

        //! Screen coordinates of the position of the last mouse press
        Vector<float,2> mousePressPosition = {0.0f, 0.0f};

        //! The current rotation axis. World frame?
        Vector<float> rotationAxis = {0.0f, 0.0f, 0.0f};

        //! A rotation quaternion. You could have guessed that, right?
        Quaternion<float> rotation;

        //! The projection matrix is a member of this class
        TransformMatrix<float> projection;

        //! The inverse of the projection
        TransformMatrix<float> invproj;

        //! A scene transformation
        TransformMatrix<float> scene;
        //! Scene transformation inverse
        TransformMatrix<float> invscene;

        Quaternion<float> savedRotation;

        /*
         * GLFW callback handlers
         */

        virtual void key_callback (GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            // Exit action
            if (key == GLFW_KEY_X && action == GLFW_PRESS) {
                std::cout << "User requested exit.\n";
                this->readyToFinish = true;
            }

            if (key == GLFW_KEY_T && action == GLFW_PRESS) {
                this->rotateModMode = !this->rotateModMode;
            }

            if (!this->sceneLocked && key == GLFW_KEY_C && action == GLFW_PRESS) {
                this->showCoordArrows = !this->showCoordArrows;
            }

            if (key == GLFW_KEY_H && action == GLFW_PRESS) {
                // Help to stdout:
                std::cout << "h: Output this help to stdout\n";
                std::cout << "x: Request exit\n";
                std::cout << "l: Toggle the scene lock\n";
                std::cout << "t: Toggle mouse rotate mode\n";
                std::cout << "c: Toggle coordinate arrows\n";
                std::cout << "s: Take a snapshot\n";
                std::cout << "a: Reset default view\n";
                std::cout << "o: Reduce field of view\n";
                std::cout << "p: Increase field of view\n";
                std::cout << "z: Show the current scenetrans (x,y,z)\n";
                std::cout << "u: Reduce zNear cutoff plane\n";
                std::cout << "i: Increase zNear cutoff plane\n";
                std::cout << "0-9: Select model index (with shift: toggle hide)\n";
                std::cout << "Left: Decrease opacity of selected model\n";
                std::cout << "Right: Increase opacity of selected model\n";
            }

            if (key == GLFW_KEY_L && action == GLFW_PRESS) {
                this->sceneLocked = this->sceneLocked ? false : true;
                std::cout << "Scene is now " << (this->sceneLocked ? "" : "un-") << "locked\n";
            }

            if (key == GLFW_KEY_S && action == GLFW_PRESS) {
                this->saveImage ("./picture.png");
                std::cout << "Took a snap\n";
            }

            if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
                std::cout << "Scenetrans is: " << this->scenetrans << std::endl;
            }

            // Set selected model
            if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
                this->selectedVisualModel = 0;
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
                if (this->vm.size() > 1) { this->selectedVisualModel = 1; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
                if (this->vm.size() > 2) { this->selectedVisualModel = 2; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
                if (this->vm.size() > 3) { this->selectedVisualModel = 3; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
                if (this->vm.size() > 4) { this->selectedVisualModel = 4; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
                if (this->vm.size() > 5) { this->selectedVisualModel = 5; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
                if (this->vm.size() > 6) { this->selectedVisualModel = 6; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (key == GLFW_KEY_7 && action == GLFW_PRESS) {
                if (this->vm.size() > 7) { this->selectedVisualModel = 7; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (key == GLFW_KEY_8 && action == GLFW_PRESS) {
                if (this->vm.size() > 8) { this->selectedVisualModel = 8; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (key == GLFW_KEY_9 && action == GLFW_PRESS) {
                if (this->vm.size() > 9) { this->selectedVisualModel = 9; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            }

            // Toggle hide model if the shift key is down
            if ((key == GLFW_KEY_0 || key == GLFW_KEY_1 || key == GLFW_KEY_2 || key == GLFW_KEY_3
                 || key == GLFW_KEY_4 || key == GLFW_KEY_5 || key == GLFW_KEY_6
                 || key == GLFW_KEY_7 || key == GLFW_KEY_8 || key == GLFW_KEY_9)
                && action == GLFW_PRESS && (mods & GLFW_MOD_SHIFT)) {
                this->vm[this->selectedVisualModel]->toggleHide();
            }

            // Increment/decrement alpha for selected model
            if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                if (!this->vm.empty()) { this->vm[this->selectedVisualModel]->decAlpha(); }
            }
            if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                if (!this->vm.empty()) { this->vm[this->selectedVisualModel]->incAlpha(); }
            }

            // Reset view to default
            if (!this->sceneLocked && key == GLFW_KEY_A && action == GLFW_PRESS) {
                std::cout << "Reset to default view\n";
                // Reset translation
                this->scenetrans = this->scenetrans_default;
                // Reset rotation
                Quaternion<float> rt;
                this->rotation = rt;

                this->render();
            }

            if (!this->sceneLocked && key == GLFW_KEY_O && action == GLFW_PRESS) {
                this->fov -= 2;
                if (this->fov < 1.0) {
                    this->fov = 2.0;
                }
                std::cout << "FOV reduced to " << this->fov << std::endl;
            }
            if (!this->sceneLocked && key == GLFW_KEY_P && action == GLFW_PRESS) {
                this->fov += 2;
                if (this->fov > 179.0) {
                    this->fov = 178.0;
                }
                std::cout << "FOV increased to " << this->fov << std::endl;
            }
            if (!this->sceneLocked && key == GLFW_KEY_U && action == GLFW_PRESS) {
                this->zNear /= 2;
                std::cout << "zNear reduced to " << this->zNear << std::endl;
            }
            if (!this->sceneLocked && key == GLFW_KEY_I && action == GLFW_PRESS) {
                this->zNear *= 2;
                std::cout << "zNear increased to " << this->zNear << std::endl;
            }

            this->key_callback_extra (window, key, scancode, action, mods);
        }

        virtual void cursor_position_callback (GLFWwindow* window, double x, double y)
        {
            this->cursorpos[0] = static_cast<float>(x);
            this->cursorpos[1] = static_cast<float>(y);

            Vector<float> mouseMoveWorld = { 0.0f, 0.0f, 0.0f };

            // This is "rotate the scene" model. Will need "rotate one visual" mode.
            if (this->rotateMode) {
                // Convert mousepress/cursor positions (in pixels) to the range -1 -> 1:
                Vector<float, 2> p0_coord = this->mousePressPosition;
                p0_coord[0] -= this->window_w/2.0;
                p0_coord[0] /= this->window_w/2.0;
                p0_coord[1] -= this->window_h/2.0;
                p0_coord[1] /= this->window_h/2.0;
                Vector<float, 2> p1_coord = this->cursorpos;
                p1_coord[0] -= this->window_w/2.0;
                p1_coord[0] /= this->window_w/2.0;
                p1_coord[1] -= this->window_h/2.0;
                p1_coord[1] /= this->window_h/2.0;

                // DON'T update mousePressPosition until user releases button.
                // this->mousePressPosition = this->cursorpos;

                // Add the depth at which the object lies.  Use forward projection to determine
                // the correct z coordinate for the inverse projection. This assumes only one
                // object.
                Vector<float, 4> point =  { 0.0f, 0.0f, this->scenetrans.z(), 1.0f };
                Vector<float, 4> pp = this->projection * point;
                float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.

                // Construct two points for the start and end of the mouse movement
                Vector<float, 4> p0 = { p0_coord[0], p0_coord[1], coord_z, 1.0f };
                Vector<float, 4> p1 = { p1_coord[0], p1_coord[1], coord_z, 1.0f };

                // Apply the inverse projection to get two points in the world frame of
                // reference for the mouse movement
                Vector<float, 4> v0 = this->invproj * p0;
                Vector<float, 4> v1 = this->invproj * p1;

                // This computes the difference betwen v0 and v1, the 2 mouse positions in the
                // world space. Note the swap between x and y
                if (this->rotateModMode) {
                    // Sort of "rotate the page" mode.
                    mouseMoveWorld[2] = -((v1[1]/v1[3]) - (v0[1]/v0[3])) + ((v1[0]/v1[3]) - (v0[0]/v0[3]));
                } else {
                    mouseMoveWorld[1] = -((v1[0]/v1[3]) - (v0[0]/v0[3]));
                    mouseMoveWorld[0] = -((v1[1]/v1[3]) - (v0[1]/v0[3]));
                }

                // Rotation axis is perpendicular to the mouse position difference vector
                // BUT we have to project into the model frame to determine how to rotate the model!
                float rotamount = mouseMoveWorld.length() * 40.0;
                // Calculate new rotation axis as weighted sum
                this->rotationAxis = (mouseMoveWorld * rotamount);
                this->rotationAxis.renormalize();

                // Now inverse apply the rotation of the scene to the rotation axis
                // (Vector<float,3>), so that we rotate the model the right way.
                Vector<float, 4> tmp_4D = this->invscene * this->rotationAxis;
                this->rotationAxis.set_from (tmp_4D); // Set rotationAxis from 4D result

                // Update rotation from the saved position.
                this->rotation = this->savedRotation;
                Quaternion<float> rotationQuaternion;
                rotationQuaternion.initFromAxisAngle (this->rotationAxis, rotamount);
                this->rotation.premultiply (rotationQuaternion); // combines rotations
                this->render(); // updates viewproj; uses this->rotation

            } else if (this->translateMode) { // allow only rotate OR translate for a single mouse movement

                // Convert mousepress/cursor positions (in pixels) to the range -1 -> 1:
                Vector<float, 2> p0_coord = this->mousePressPosition;
                p0_coord[0] -= this->window_w/2.0;
                p0_coord[0] /= this->window_w/2.0;
                p0_coord[1] -= this->window_h/2.0;
                p0_coord[1] /= this->window_h/2.0;
                Vector<float, 2> p1_coord = this->cursorpos;
                p1_coord[0] -= this->window_w/2.0;
                p1_coord[0] /= this->window_w/2.0;
                p1_coord[1] -= this->window_h/2.0;
                p1_coord[1] /= this->window_h/2.0;

                this->mousePressPosition = this->cursorpos;

                // Add the depth at which the object lies.  Use forward projection to determine
                // the correct z coordinate for the inverse projection. This assumes only one
                // object.
                Vector<float, 4> point =  { 0.0f, 0.0f, this->scenetrans.z(), 1.0f };
                Vector<float, 4> pp = this->projection * point;
                float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.

                // Construct two points for the start and end of the mouse movement
                Vector<float, 4> p0 = { p0_coord[0], p0_coord[1], coord_z, 1.0f };
                Vector<float, 4> p1 = { p1_coord[0], p1_coord[1], coord_z, 1.0f };
                // Apply the inverse projection to get two points in the world frame of reference:
                Vector<float, 4> v0 = this->invproj * p0;
                Vector<float, 4> v1 = this->invproj * p1;
                // This computes the difference betwen v0 and v1, the 2 mouse positions in the world
                mouseMoveWorld[0] = (v1[0]/v1[3]) - (v0[0]/v0[3]);
                mouseMoveWorld[1] = (v1[1]/v1[3]) - (v0[1]/v0[3]);
                //mouseMoveWorld.z = (v1[2]/v1[3]) - (v0[2]/v0[3]);// unmodified

                // This is "translate the scene" mode. Could also have a "translate one
                // HexGridVisual" mode, to adjust relative positions.
                this->scenetrans[0] += mouseMoveWorld[0];
                this->scenetrans[1] -= mouseMoveWorld[1];
                this->render(); // updates viewproj; uses this->scenetrans
            }
        }

        virtual void mouse_button_callback (GLFWwindow* window, int button, int action, int mods)
        {
            // If the scene is locked, then ignore the mouse movements
            if (this->sceneLocked) { return; }

            // button is the button number, action is either key press (1) or key release (0)
            // std::cout << "button: " << button << " action: " << (action==1?("press"):("release")) << std::endl;

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

            this->mouse_button_callback_extra (window, button, action, mods);
        }

        virtual void window_size_callback (GLFWwindow* window, int width, int height)
        {
            this->window_w = width;
            this->window_h = height;
            this->render();
        }

        virtual void scroll_callback (GLFWwindow* window, double xoffset, double yoffset)
        {
            if (this->sceneLocked) { return; }
            // x and y can be +/- 1
            this->scenetrans[0] -= xoffset * this->scenetrans_stepsize;
            if (this->translateMode) {
                this->scenetrans[1]/*z really*/ += yoffset * this->scenetrans_stepsize;
                std::cout << "scenetrans.y = " << this->scenetrans[1] << std::endl;
            } else {
                this->scenetrans[2] += yoffset * this->scenetrans_stepsize;
            }
            this->render();
        }

        //! Extra key callback handling, making it easy for client programs to implement their own actions
        virtual void key_callback_extra (GLFWwindow* window, int key, int scancode, int action, int mods) {}
        //! Extra mousebutton callback handling, making it easy for client programs to implement their own actions
        virtual void mouse_button_callback_extra (GLFWwindow* window, int button, int action, int mods) {}
    };

} // namespace morph
