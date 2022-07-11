/*!
 * \file
 *
 * Awesome graphics code for high performance graphing and visualisation. Uses modern
 * OpenGL and the library GLFW for window management.
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

#include <morph/VisualModel.h>
#include <morph/VisualTextModel.h>
#include <morph/VisualCommon.h>
// Include glfw3 AFTER VisualModel
#include <GLFW/glfw3.h>
// For GLuint and GLenum (though redundant, as already included in VisualModel
#ifndef USE_GLEW
#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#endif

#include <morph/VisualResources.h>
#include <morph/nlohmann/json.hpp>
#include <morph/CoordArrows.h>
#include <morph/Quaternion.h>
#include <morph/TransformMatrix.h>
#include <morph/Vector.h>
#include <morph/ColourMap.h>

#include <string>
#include <array>
#include <vector>

#include <morph/VisualDefaultShaders.h>

// Use Lode Vandevenne's PNG encoder
#define LODEPNG_NO_COMPILE_DECODER 1
#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS 1
#include <morph/lodepng.h>

//! The default z=0 position for VisualModels
#define Z_DEFAULT -5

#ifdef PROFILE_RENDER
#include <chrono>
using namespace std::chrono;
using std::chrono::steady_clock;
#endif

namespace morph {

    /*!
     * Data structure for shader info.
     *
     * LoadShaders() takes an array of ShaderFile structures, each of which contains the
     * type of the shader, and a pointer a C-style character string (i.e., a
     * NULL-terminated array of characters) containing the filename of a GLSL file to
     * use, and another pointer to the compiled-in version of the shader.
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
        const char* compiledIn;
        GLuint shader;
    } ShaderInfo;

    // To enable debugging, set true.
    const bool debug_shaders = false;

    //! Whether to render with perspective or orthographic
    enum class perspective_type
    {
        perspective,
        orthographic
    };

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
    class Visual
    {
    public:
        /*!
         * Construct a new visualiser. The rule is 1 window to one Visual object. So,
         * this creates a new window and a new OpenGL context.
         */
        Visual (int width, int height, const std::string& _title)
            : window_w(width)
            , window_h(height)
            , title(_title)
        {
            this->init();
        }

        /*!
         * Construct with specified coordinate arrows offset (caOffset) lengths
         * (caLength), thickness scaling factor (caThickness) and coordinate arrow 'm'
         * size, caEm.
         */
        Visual (int width, int height, const std::string& _title,
                const Vector<float, 2> caOffset, const Vector<float> caLength, const float caThickness, const float caEm)
            : window_w(width)
            , window_h(height)
            , title(_title)
            , coordArrowsOffset(caOffset)
            , coordArrowsLength(caLength)
            , coordArrowsThickness(caThickness)
            , coordArrowsEm(caEm)
        {
            this->init();
        }

        //! Deconstructor deallocates CoordArrows and destroys GLFW windows
        virtual ~Visual()
        {
            delete this->coordArrows;
            for (unsigned int i = 0; i < this->vm.size(); ++i) {
                delete this->vm[i];
            }
            if (this->textModel != nullptr) {
                delete this->textModel;
            }
            for (auto t : this->texts) { delete t; }
            this->texts.clear();
            glfwDestroyWindow (this->window);
            morph::VisualResources::deregister();
        }

        //! Take a screenshot of the window
        void saveImage (const std::string& img_filename)
        {
            glfwMakeContextCurrent (this->window);
            GLubyte* bits; // RGB bits
            GLubyte* rbits;
            GLint viewport[4]; // current viewport
            glGetIntegerv (GL_VIEWPORT, viewport);
            int w = viewport[2];
            int h = viewport[3];
            bits = new GLubyte[w*h*4];
            rbits = new GLubyte[w*h*4];
            glFinish(); // finish all commands of OpenGL
            glPixelStorei (GL_PACK_ALIGNMENT, 1);
            glPixelStorei (GL_PACK_ROW_LENGTH, 0);
            glPixelStorei (GL_PACK_SKIP_ROWS, 0);
            glPixelStorei (GL_PACK_SKIP_PIXELS, 0);
            glReadPixels (0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, bits);
            for (int i = 0; i < h; ++i) {
                int rev_line = (h-i-1)*4*w;
                int for_line = i*4*w;
                for (int j = 0; j < 4*w; ++j) {
                    rbits[rev_line+j] = bits[for_line+j];
                }
            }
            unsigned int error = lodepng::encode (img_filename, rbits, w, h);
            if (error) {
                std::cout << "encoder error " << error << ": " << lodepng_error_text (error) << std::endl;
            }
            delete[] rbits;
            delete[] bits;
        }

        //! Make this Visual the current one, so that when creating/adding a visual
        //! model, the vao ids relate to the correct OpenGL context.
        void setCurrent() { glfwMakeContextCurrent (this->window); }

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

        //! Add a text label to the scene at a given location. Return the width and
        //! height of the text in a TextGeometry object.
        morph::TextGeometry addLabel (const std::string& _text,
                                      const morph::Vector<float, 3>& _toffset,
                                      const std::array<float, 3>& _tcolour = morph::colour::black,
                                      const morph::VisualFont _font = morph::VisualFont::DVSans,
                                      const float _fontsize = 0.01,
                                      const int _fontres = 24)
        {
            morph::VisualTextModel* tm = nullptr;
            return this->addLabel (_text, _toffset, tm, _tcolour, _font, _fontsize, _fontres);
        }

        //! Add label, using the passed-in pointer. Allows client code to update the text model
        morph::TextGeometry addLabel (const std::string& _text,
                                      const morph::Vector<float, 3>& _toffset,
                                      morph::VisualTextModel*& tm,
                                      const std::array<float, 3>& _tcolour = morph::colour::black,
                                      const morph::VisualFont _font = morph::VisualFont::DVSans,
                                      const float _fontsize = 0.01,
                                      const int _fontres = 24)
        {
            if (this->tshaderprog == 0) { throw std::runtime_error ("No text shader prog."); }
            tm = new morph::VisualTextModel (this->tshaderprog, _font, _fontsize, _fontres);
            tm->setupText (_text, _toffset, _tcolour);
            this->texts.push_back (tm);
            return tm->getTextGeometry();
        }

        /*!
         * Keep on rendering until readToFinish is set true. Used to keep a window open,
         * and responsive, while displaying the result of a simulation. FIXME: This
         * won't work for two or more windows because it will block.
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
            glfwMakeContextCurrent (this->window);

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

            // Set the perspective
            if (this->ptype == perspective_type::orthographic) {
                this->setOrthographic();
            } else {
                this->setPerspective();
            }

            // Calculate model view transformation - transforming from "model space" to "worldspace".
            TransformMatrix<float> sceneview;
            // This line translates from model space to world space.
            sceneview.translate (this->scenetrans); // send backwards into distance
            // And this rotation completes the transition from model to world
            sceneview.rotate (this->rotation);

            // Clear color buffer and **also depth buffer**
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Set the background colour:
            glClearBufferfv (GL_COLOR, 0, bgcolour.data());

            // Lighting shader variables
            //
            // Ambient light colour
            GLint loc_lightcol = glGetUniformLocation (this->shaderprog, static_cast<const GLchar*>("light_colour"));
            if (loc_lightcol != -1) {
                glUniform3fv (loc_lightcol, 1, this->light_colour.data());
            }
            // Ambient light intensity
            GLint loc_ai = glGetUniformLocation (this->shaderprog, static_cast<const GLchar*>("ambient_intensity"));
            if (loc_ai != -1) {
                glUniform1f (loc_ai, this->ambient_intensity);
            }
            // Diffuse light position
            GLint loc_dp = glGetUniformLocation (this->shaderprog, static_cast<const GLchar*>("diffuse_position"));
            if (loc_dp != -1) {
                glUniform3fv (loc_dp, 1, this->diffuse_position.data());
            }
            // Diffuse light intensity
            GLint loc_di = glGetUniformLocation (this->shaderprog, static_cast<const GLchar*>("diffuse_intensity"));
            if (loc_di != -1) {
                glUniform1f (loc_di, this->diffuse_intensity);
            }

#if 0
            // A quick-n-dirty attempt to keep the light position fixed in camera space.
            Vector<float, 2> l_p0_coord = this->coordArrowsOffset;
            Vector<float, 4> l_point =  { 0.0, 0.0, -13.0, 1.0 };
            Vector<float, 4> l_pp = this->projection * l_point;
            float l_coord_z = l_pp[2]/l_pp[3]; // divide by pp[3] is divide by/normalise by 'w'.
            Vector<float, 4> l_p0 = { l_p0_coord.x(), l_p0_coord.y(), l_coord_z, 1.0 };
            Vector<float, 3> l_v0;
            l_v0.set_from (this->invproj * l_p0);
            TransformMatrix<float> lv_matrix;
            lv_matrix.translate (l_v0);
            lv_matrix.rotate (this->rotation);
            GLint loc_lv = glGetUniformLocation (this->shaderprog, static_cast<const GLchar*>("lv_matrix"));
            if (loc_lv != -1) { glUniformMatrix4fv (loc_lv, 1, GL_FALSE, lv_matrix.mat.data()); }
            std::cout << "lv_matrix:\n" << lv_matrix.str() << std::endl;
            std::cout << "p_matrix:\n" << this->projection.str() << std::endl;
            morph::gl::Util::checkError (__FILE__, __LINE__);
#endif
            // Switch to text shader program and set the projection matrix
            glUseProgram (this->tshaderprog);
            GLint loc_p = glGetUniformLocation (this->tshaderprog, static_cast<const GLchar*>("p_matrix"));
            if (loc_p != -1) { glUniformMatrix4fv (loc_p, 1, GL_FALSE, this->projection.mat.data()); }

            // Switch back to the regular shader prog and render the VisualModels.
            glUseProgram (this->shaderprog);

            // Set the projection matrix just once
            loc_p = glGetUniformLocation (this->shaderprog, static_cast<const GLchar*>("p_matrix"));
            if (loc_p != -1) { glUniformMatrix4fv (loc_p, 1, GL_FALSE, this->projection.mat.data()); }

            if (this->showCoordArrows == true) {
                // Ensure coordarrows centre sphere will be visible on BG:
                this->coordArrows->setColourForBackground (this->bgcolour);

                if (this->coordArrowsInScene == true) {
                    this->coordArrows->setSceneMatrix (sceneview);
                } else {
                    this->positionCoordArrows();
                }
                this->coordArrows->render();
            }

            TransformMatrix<float> scenetransonly;
            scenetransonly.translate (this->scenetrans);

            typename std::vector<VisualModel*>::iterator vmi = this->vm.begin();
            while (vmi != this->vm.end()) {
                if ((*vmi)->twodimensional == true) {
                    // It's a two-d thing. Now what?
                    (*vmi)->setSceneMatrix (scenetransonly);
                } else {
                    (*vmi)->setSceneMatrix (sceneview);
                }
                (*vmi)->render();
                ++vmi;
            }

            morph::gl::Util::checkError (__FILE__, __LINE__);

            Vector<float, 3> v0 = this->textPosition ({-0.8f, 0.8f});
            if (this->showTitle == true) {
                // Render the title text
                glUseProgram (this->tshaderprog);
                this->textModel->setSceneTranslation (v0);
                this->textModel->setVisibleOn (this->bgcolour);
                this->textModel->render();
            }

            for (auto t : this->texts) {
                glUseProgram (this->tshaderprog);
                t->setSceneTranslation (v0);
                t->setVisibleOn (this->bgcolour);
                t->render();
            }

            glfwSwapBuffers (this->window);

#ifdef PROFILE_RENDER
            steady_clock::time_point renderend = steady_clock::now();
            steady_clock::duration time_span = renderend - renderstart;
            std::cout << "Render took " << duration_cast<microseconds>(time_span).count() << " us\n";
#endif
        }

        //! Compute a translation vector for text position, using Visual::text_z.
        Vector<float, 3> textPosition (Vector<float, 2> p0_coord)
        {
            // For the depth at which a text object lies, use this->text_z.  Use forward
            // projection to determine the correct z coordinate for the inverse
            // projection.
            Vector<float, 4> point =  { 0.0, 0.0, this->text_z, 1.0 };
            Vector<float, 4> pp = this->projection * point;
            float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.
            // Construct the point for the location of the text
            Vector<float, 4> p0 = { p0_coord.x(), p0_coord.y(), coord_z, 1.0 };
            // Inverse project the point
            Vector<float, 3> v0;
            v0.set_from (this->invproj * p0);
            //tm->setSceneTranslation (v0);
            return v0;
        }

        //! The OpenGL shader program for graphical objects
        GLuint shaderprog;
        //! The text shader program, which uses textures to draw text on quads.
        GLuint tshaderprog;

        //! The colour of ambient and diffuse light sources
        Vector<float> light_colour = {1,1,1};
        //! Strength of the ambient light
        float ambient_intensity = 1.0f;
        //! Position of a diffuse light source
        Vector<float> diffuse_position = {5,5,15};
        //! Strength of the diffuse light source
        float diffuse_intensity = 0.0f;

        //! Compute position and rotation of coordinate arrows in the bottom left of the screen
        void positionCoordArrows()
        {
            // Find out the location of the bottom left of the screen and make the coord
            // arrows stay put there.

            // Add the depth at which the object lies.  Use forward projection to determine
            // the correct z coordinate for the inverse projection. This assumes only one
            // object.
            Vector<float, 4> point =  { 0.0, 0.0, this->scenetrans.z(), 1.0 };
            Vector<float, 4> pp = this->projection * point;
            float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.

            // Construct the point for the location of the coord arrows
            Vector<float, 4> p0 = { this->coordArrowsOffset.x(), this->coordArrowsOffset.y(), coord_z, 1.0 };
            // Inverse project
            Vector<float, 3> v0;
            v0.set_from ((this->invproj * p0));
            // Translate the scene for the CoordArrows such that they sit in a single position on the screen
            this->coordArrows->setSceneTranslation (v0);
            // Apply rotation to the coordArrows model
            this->coordArrows->setViewRotation (this->rotation);
        }

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

        //! Set perspective for ortho projection
        void setOrthographic()
        {
            this->projection.setToIdentity();
            this->projection.orthographic (this->ortho_bl, this->ortho_tr, this->zNear, this->zFar);
            this->invproj = this->projection.invert();
        }

        //! Set to true when the program should end
        bool readyToFinish = false;

        //! Set true to disable the 'X' button on the Window from exiting the program
        bool preventWindowCloseWithButton = false;

        /*
         * User-settable projection values for the near clipping distance, the far
         * clipping distance and the field of view of the camera.
         */

        float zNear = 0.001;
        float zFar = 300.0;
        float fov = 30.0;

        //! Set to true to show the coordinate arrows
        bool showCoordArrows = false;

        //! If true, then place the coordinate arrows at the origin of the scene, rather than offset.
        bool coordArrowsInScene = false;

        //! Set to true to show the title text within the scene
        bool showTitle = false;

        //! How big should the steps in scene translation be when scrolling?
        float scenetrans_stepsize = 0.1;

        //! If you set this to true, then the mouse movements won't change scenetrans or rotation.
        bool sceneLocked = false;

        //! Can change this to orthographic
        perspective_type ptype = perspective_type::perspective;

        //! Orthographic screen bottom left coordinate (you can change these to encapsulate your models)
        morph::Vector<float, 2> ortho_bl = { -1.0f, -1.0f };
        //! Orthographic screen top right coordinate
        morph::Vector<float, 2> ortho_tr = { 1.0f, 1.0f };

        //! The background colour; white by default.
        std::array<float, 4> bgcolour = { 1.0f, 1.0f, 1.0f, 0.5f };

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
        }

        //! Set the scene's x and y values at the same time.
        void setSceneTransXY (float _x, float _y)
        {
            this->scenetrans[0] = _x;
            this->scenetrans[1] = _y;
            this->scenetrans_default[0] = _x;
            this->scenetrans_default[1] = _y;
        }
        //! Set the scene's y value. Use this to shift your scene objects left or right
        void setSceneTransX (float _x) { this->scenetrans[0] = _x; this->scenetrans_default[0] = _x; }
        //! Set the scene's y value. Use this to shift your scene objects up and down
        void setSceneTransY (float _y) { this->scenetrans[1] = _y; this->scenetrans_default[1] = _y; }
        //! Set the scene's z value. Use this to bring the 'camera' closer to your scene
        //! objects (that is, your morph::VisualModel objects).
        void setSceneTransZ (float _z)
        {
            if (_z>0.0f) {
                std::cout << "WARNING setSceneTransZ(): Normally, the default z value is negative.\n";
            }
            this->setZDefault (_z);
            this->scenetrans[2] = _z;
            this->scenetrans_default[2] = _z;
        }
        void setSceneTrans (float _x, float _y, float _z)
        {
            if (_z>0.0f) {
                std::cout << "WARNING setSceneTrans(): Normally, the default z value is negative.\n";
            }
            this->scenetrans[0] = _x;
            this->scenetrans_default[0] = _x;
            this->scenetrans[1] = _y;
            this->scenetrans_default[1] = _y;
            this->setZDefault (_z);
            this->scenetrans[2] = _z;
            this->scenetrans_default[2] = _z;
        }
        void setSceneTrans (const morph::Vector<float, 3>& _xyz)
        {
            if (_xyz[2] > 0.0f) {
                std::cout << "WARNING setSceneTrans(Vector<>&): Normally, the default z value is negative.\n";
            }
            this->setZDefault (_xyz[2]);
            this->scenetrans = _xyz;
            this->scenetrans_default = _xyz;
        }

        void lightingEffects (bool effects_on = true)
        {
            ambient_intensity = effects_on ? 0.4f : 1.0f;
            diffuse_intensity = effects_on ? 0.6f : 0.0f;
        }

        //! Save all the VisualModels in this Visual out to a GLTF format file
        void savegltf (const std::string& gltf_file)
        {
            std::ofstream fout;
            fout.open (gltf_file, std::ios::out|std::ios::trunc);
            if (!fout.is_open()) { throw std::runtime_error ("Visual::savegltf(): Failed to open file for writing"); }
            fout << "{\n  \"scenes\" : [ { \"nodes\" : [ ";
            for (size_t vmi = 0; vmi < this->vm.size(); ++vmi) {
                fout << vmi << (vmi < this->vm.size()-1 ? ", " : "");
            }
            fout << " ] } ],\n";

            fout << "  \"nodes\" : [\n";
            // for loop over VisualModels "mesh" : 0, etc
            for (size_t vmi = 0; vmi < this->vm.size(); ++vmi) {
                fout << "    { \"mesh\" : " << vmi
                     << ", \"translation\" : " << this->vm[vmi]->translation_str()
                     << (vmi < this->vm.size()-1 ? " },\n" : " }\n");
            }
            fout << "  ],\n";

            fout << "  \"meshes\" : [\n";
            // for each VisualModel:
            for (size_t vmi = 0; vmi < this->vm.size(); ++vmi) {
                fout << "    { \"primitives\" : [ { \"attributes\" : { \"POSITION\" : " << 1+vmi*4
                     << ", \"COLOR_0\" : " << 2+vmi*4
                     << ", \"NORMAL\" : " << 3+vmi*4 << " }, \"indices\" : " << vmi*4 << ", \"material\": 0 } ] }"
                     << (vmi < this->vm.size()-1 ? ",\n" : "\n");
            }
            fout << "  ],\n";

            fout << "  \"buffers\" : [\n";
            for (size_t vmi = 0; vmi < this->vm.size(); ++vmi) {
                // indices
                fout << "    {\"uri\" : \"data:application/octet-stream;base64," << this->vm[vmi]->indices_base64() << "\", "
                     << "\"byteLength\" : " << this->vm[vmi]->indices_bytes() << "},\n";
                // pos
                fout << "    {\"uri\" : \"data:application/octet-stream;base64," << this->vm[vmi]->vpos_base64() << "\", "
                     << "\"byteLength\" : " << this->vm[vmi]->vpos_bytes() << "},\n";
                // col
                fout << "    {\"uri\" : \"data:application/octet-stream;base64," << this->vm[vmi]->vcol_base64() << "\", "
                     << "\"byteLength\" : " << this->vm[vmi]->vcol_bytes() << "},\n";
                // norm
                fout << "    {\"uri\" : \"data:application/octet-stream;base64," << this->vm[vmi]->vnorm_base64() << "\", "
                     << "\"byteLength\" : " << this->vm[vmi]->vnorm_bytes() << "}";
                fout << (vmi < this->vm.size()-1 ? ",\n" : "\n");
            }
            fout << "  ],\n";

            fout << "  \"bufferViews\" : [\n";
            for (size_t vmi = 0; vmi < this->vm.size(); ++vmi) {
                // indices
                fout << "    { ";
                fout << "\"buffer\" : " << vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"byteLength\" : " << this->vm[vmi]->indices_bytes() << ", ";
                fout << "\"target\" : 34963 ";
                fout << " },\n";
                // vpos
                fout << "    { ";
                fout << "\"buffer\" : " << 1+vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"byteLength\" : " << this->vm[vmi]->vpos_bytes() << ", ";
                fout << "\"target\" : 34962 ";
                fout << " },\n";
                // vcol
                fout << "    { ";
                fout << "\"buffer\" : " << 2+vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"byteLength\" : " << this->vm[vmi]->vcol_bytes() << ", ";
                fout << "\"target\" : 34962 ";
                fout << " },\n";
                // vnorm
                fout << "    { ";
                fout << "\"buffer\" : " << 3+vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"byteLength\" : " << this->vm[vmi]->vnorm_bytes() << ", ";
                fout << "\"target\" : 34962 ";
                fout << " }";
                fout << (vmi < this->vm.size()-1 ? ",\n" : "\n");
            }
            fout << "  ],\n";

            fout << "  \"accessors\" : [\n";
            for (size_t vmi = 0; vmi < this->vm.size(); ++vmi) {
                this->vm[vmi]->computeVertexMaxMins();
                // indices
                fout << "    { ";
                fout << "\"bufferView\" : " << vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"componentType\" : 5125, "; // 5123 unsigned short, 5121 unsigned byte, 5125 unsigned int, 5126 float
                fout << "\"type\" : \"SCALAR\", ";
                fout << "\"count\" : " << this->vm[vmi]->indices_size();
                fout << "},\n";
                // vpos
                fout << "    { ";
                fout << "\"bufferView\" : " << 1+vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"componentType\" : 5126, ";
                fout << "\"type\" : \"VEC3\", ";
                fout << "\"count\" : " << this->vm[vmi]->vpos_size()/3;
                // vertex position requires max/min to be specified in the gltf format
                fout << ", \"max\" : " << this->vm[vmi]->vpos_max() << ", ";
                fout << "\"min\" : " << this->vm[vmi]->vpos_min();
                fout << " },\n";
                // vcol
                fout << "    { ";
                fout << "\"bufferView\" : " << 2+vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"componentType\" : 5126, ";
                fout << "\"type\" : \"VEC3\", ";
                fout << "\"count\" : " << this->vm[vmi]->vcol_size()/3;
                fout << "},\n";
                // vnorm
                fout << "    { ";
                fout << "\"bufferView\" : " << 3+vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"componentType\" : 5126, ";
                fout << "\"type\" : \"VEC3\", ";
                fout << "\"count\" : " << this->vm[vmi]->vnorm_size()/3;
                fout << "}";
                fout << (vmi < this->vm.size()-1 ? ",\n" : "\n");
            }
            fout << "  ],\n";

            // Default material is single sided, so make it double sided
            fout << "  \"materials\" : [ { \"doubleSided\" : true } ],\n";

            fout << "  \"asset\" : {\n"
                 << "    \"generator\" : \"https://github.com/ABRG-Models/morphologica: morph::Visual::savegltf()\",\n"
                 << "    \"version\" : \"2.0\"\n"
                 << "  }\n";
            fout << "}\n";
            fout.close();
        }

    protected:
        //! A vector of pointers to all the morph::VisualModels (HexGridVisual,
        //! ScatterVisual, etc) which are going to be rendered in the scene.
        std::vector<VisualModel*> vm;

    private:
        //! Private initialization, used by constructors.
        void init()
        {
            // VisualResources provides font management and GLFW management.
            this->resources = morph::VisualResources::i();
            morph::VisualResources::register_visual();

            this->window = glfwCreateWindow (this->window_w, this->window_h, this->title.c_str(), NULL, NULL);
            if (!this->window) {
                // Window or OpenGL context creation failed
                throw std::runtime_error("GLFW window creation failed!");
            }
            // now associate "this" object with mWindow object
            glfwSetWindowUserPointer (this->window, this);

            // Set up callbacks
            glfwSetKeyCallback (this->window, key_callback_dispatch);
            glfwSetMouseButtonCallback (this->window, mouse_button_callback_dispatch);
            glfwSetCursorPosCallback (this->window, cursor_position_callback_dispatch);
            glfwSetWindowSizeCallback (this->window, window_size_callback_dispatch);
            glfwSetWindowCloseCallback (this->window, window_close_callback_dispatch);
            glfwSetScrollCallback (this->window, scroll_callback_dispatch);

            glfwMakeContextCurrent (this->window);

            // Now make sure that Freetype is set up
            this->resources->freetype_init (this->window);

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
                {GL_VERTEX_SHADER, "Visual.vert.glsl", morph::defaultVtxShader },
                {GL_FRAGMENT_SHADER, "Visual.frag.glsl", morph::defaultFragShader },
                {GL_NONE, NULL, NULL },
            };

            this->shaderprog = this->LoadShaders (shaders);

            // An additional shader is used for text
            ShaderInfo tshaders[] = {
                {GL_VERTEX_SHADER, "VisText.vert.glsl", morph::defaultTextVtxShader },
                {GL_FRAGMENT_SHADER, "VisText.frag.glsl" , morph::defaultTextFragShader },
                {GL_NONE, NULL, NULL }
            };
            this->tshaderprog = this->LoadShaders (tshaders);

            // Now client code can set up HexGridVisuals.
            glEnable (GL_DEPTH_TEST);

            // Make it possible to specify alpha. This is correct for text texture rendering too.
            glEnable (GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable (GL_CULL_FACE); // text example has glEnable(GL_CULL_FACE)
            // Possibly redundant call (because it's enabled by default in most drivers) to enable multisampling (for anti-aliasing)
            glEnable (GL_MULTISAMPLE);

            morph::gl::Util::checkError (__FILE__, __LINE__);

            // If possible, read in scenetrans and rotation state from a special config file
            try {
                nlohmann::json vconf;
                std::ifstream fi;
                fi.open ("/tmp/Visual.json", std::ios::in);
                fi >> vconf;
                this->scenetrans[0] = vconf.contains("scenetrans_x") ? vconf["scenetrans_x"].get<float>() : this->scenetrans[0];
                this->scenetrans[1] = vconf.contains("scenetrans_y") ? vconf["scenetrans_y"].get<float>() : this->scenetrans[1];
                this->scenetrans[2] = vconf.contains("scenetrans_z") ? vconf["scenetrans_z"].get<float>() : this->scenetrans[2];
                // Place the same numbers into scenetrans_default, too.
                this->scenetrans_default[0] = this->scenetrans[0];
                this->scenetrans_default[1] = this->scenetrans[1];
                this->scenetrans_default[2] = this->scenetrans[2];
                this->rotation.w = vconf.contains("scenerotn_w") ? vconf["scenerotn_w"].get<float>() : this->rotation.w;
                this->rotation.x = vconf.contains("scenerotn_x") ? vconf["scenerotn_x"].get<float>() : this->rotation.x;
                this->rotation.y = vconf.contains("scenerotn_y") ? vconf["scenerotn_y"].get<float>() : this->rotation.y;
                this->rotation.z = vconf.contains("scenerotn_z") ? vconf["scenerotn_z"].get<float>() : this->rotation.z;
            } catch (...) {
                // No problem if we couldn't read /tmp/Visual.json
            }

            // Use coordArrowsOffset to set the location of the CoordArrows *scene*
            this->coordArrows = new CoordArrows();
            this->coordArrows->init (this->shaderprog,
                                     this->tshaderprog,
                                     this->coordArrowsLength,
                                     this->coordArrowsThickness,
                                     this->coordArrowsEm);
            morph::gl::Util::checkError (__FILE__, __LINE__);

            // Set up the title, which may or may not be rendered
            this->textModel = new VisualTextModel (this->tshaderprog,
                                                   morph::VisualFont::DVSans,
                                                   0.035f, 64, {0.0f, 0.0f, 0.0f},
                                                   this->title);
        }

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
         * Read a default shader, stored as a const char*. ReadDefaultShader reads a
         * file: allocates some memory, copies the text into the new memory and then
         * returns a GLchar* pointer to the memory.
         */
        const GLchar* ReadDefaultShader (const char* shadercontent)
        {
            int len = strlen (shadercontent);
            GLchar* source = new GLchar[len+1];
            memcpy (static_cast<void*>(source), static_cast<const void*>(shadercontent), len);
            source[len] = 0;
            return const_cast<const GLchar*>(source);
        }

        //! Shader loading code.
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
                        source = morph::Visual::ReadDefaultShader (entry->compiledIn);
                    } else if (entry->type == GL_FRAGMENT_SHADER) {
                        if constexpr (debug_shaders == true) {
                            std::cout << "Using compiled-in fragment shader\n";
                        }
                        source = morph::Visual::ReadDefaultShader (entry->compiledIn);
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

    private:
        //! The window (and OpenGL context) for this Visual
        GLFWwindow* window;

        //! Pointer to the singleton GLFW and Freetype resources object
        morph::VisualResources* resources = nullptr;

        //! Current window width
        int window_w;
        //! Current window height
        int window_h;

        //! The title for the Visual. Used in window title and if saving out 3D model or png image.
        std::string title = "morph::Visual";

        //! The user's 'selected visual model'. For model specific changes to alpha and possibly colour
        unsigned int selectedVisualModel = 0;

    protected:
        //! A little model of the coordinate axes.
        CoordArrows* coordArrows;

        //! Position coordinate arrows on screen. Configurable at morph::Visual construction.
        Vector<float, 2> coordArrowsOffset = {-0.8f, -0.8f};
        //! Length of coordinate arrows. Configurable at morph::Visual construction.
        Vector<float> coordArrowsLength = {0.1f, 0.1f, 0.1f};
        //! A factor used to slim (<1) or thicken (>1) the thickness of the axes of the CoordArrows.
        float coordArrowsThickness = 1.0f;
        //! Text size for x,y,z.
        float coordArrowsEm = 0.01f;

    private:
        //! A VisualTextModel for a title text.
        VisualTextModel* textModel = nullptr;
        //! Text models for labels
        std::vector<morph::VisualTextModel*> texts;

        /*
         * Variables to manage projection and rotation of the object
         */

        //! Current cursor position
        Vector<float,2> cursorpos = {0.0f, 0.0f};

        //! Holds the translation coordinates for the current location of the entire scene
        Vector<float> scenetrans = {0.0, 0.0, Z_DEFAULT};

        //! Default for scenetrans. This is a scene position that can be reverted to, to 'reset the view'.
        Vector<float> scenetrans_default = {0.0, 0.0, Z_DEFAULT};

        //! The world depth at which text objects should be rendered
        float text_z = -1.0f;

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
         * GLFW callback dispatch functions
         */

        static void key_callback_dispatch (GLFWwindow* _window, int key, int scancode, int action, int mods)
        {
            Visual* self = static_cast<Visual*>(glfwGetWindowUserPointer (_window));
            self->key_callback (_window, key, scancode, action, mods);
        }
        static void mouse_button_callback_dispatch (GLFWwindow* _window, int button, int action, int mods)
        {
            Visual* self = static_cast<Visual*>(glfwGetWindowUserPointer (_window));
            self->mouse_button_callback (_window, button, action, mods);
        }
        static void cursor_position_callback_dispatch (GLFWwindow* _window, double x, double y)
        {
            Visual* self = static_cast<Visual*>(glfwGetWindowUserPointer (_window));
            self->cursor_position_callback (_window, x, y);
        }
        static void window_size_callback_dispatch (GLFWwindow* _window, int width, int height)
        {
            Visual* self = static_cast<Visual*>(glfwGetWindowUserPointer (_window));
            self->window_size_callback (_window, width, height);
        }
        static void window_close_callback_dispatch (GLFWwindow* _window)
        {
            Visual* self = static_cast<Visual*>(glfwGetWindowUserPointer (_window));
            self->window_close_callback (_window);
        }
        static void scroll_callback_dispatch (GLFWwindow* _window, double xoffset, double yoffset)
        {
            Visual* self = static_cast<Visual*>(glfwGetWindowUserPointer (_window));
            self->scroll_callback (_window, xoffset, yoffset);
        }

        /*
         * GLFW callback handlers
         */

        virtual void key_callback (GLFWwindow* _window, int key, int scancode, int action, int mods)
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
                std::cout << "m: Save 3D models in .gltf format (open in e.g. blender)\n";
                std::cout << "a: Reset default view\n";
                std::cout << "o: Reduce field of view\n";
                std::cout << "p: Increase field of view\n";
                std::cout << "z: Show the current scenetrans/rotation and save to /tmp/Visual.json\n";
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
                std::string fname (this->title);
                morph::Tools::stripFileSuffix (fname);
                fname += ".png";
                // Make fname 'filename safe'
                morph::Tools::conditionAsFilename (fname);
                this->saveImage (fname);
                std::cout << "Saved image to '" << fname << "'\n";
            }

            // Save gltf 3D file
            if (key == GLFW_KEY_M && action == GLFW_PRESS) {
                std::string gltffile = this->title;
                morph::Tools::stripFileSuffix (gltffile);
                gltffile += ".gltf";
                morph::Tools::conditionAsFilename (gltffile);
                this->savegltf (gltffile);
                std::cout << "Saved 3D file '" << gltffile << "'\n";
            }

            if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
                std::cout << "Scenetrans is: " << this->scenetrans << ", scene rotation is " << this->rotation << std::endl;
                std::cout << "Writing scene trans/rotation into /tmp/Visual.json... ";
                std::ofstream fout;
                fout.open ("/tmp/Visual.json", std::ios::out|std::ios::trunc);
                if (fout.is_open()) {
                    fout << "{\"scenetrans_x\":" << this->scenetrans.x()
                         << ", \"scenetrans_y\":" << this->scenetrans.y()
                         << ", \"scenetrans_z\":" << this->scenetrans.z()
                         << ",\n \"scenerotn_w\":" << this->rotation.w
                         << ", \"scenerotn_x\":" <<  this->rotation.x
                         << ", \"scenerotn_y\":" <<  this->rotation.y
                         << ", \"scenerotn_z\":" <<  this->rotation.z << "}\n";
                    fout.close();
                    std::cout << "Success.\n";
                } else {
                    std::cout << "Failed.\n";
                }
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

            this->key_callback_extra (_window, key, scancode, action, mods);
        }

        virtual void cursor_position_callback (GLFWwindow* _window, double x, double y)
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

        virtual void mouse_button_callback (GLFWwindow* _window, int button, int action, int mods)
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

            this->mouse_button_callback_extra (_window, button, action, mods);
        }

        virtual void window_size_callback (GLFWwindow* _window, int width, int height)
        {
            this->window_w = width;
            this->window_h = height;
            this->render();
        }

        virtual void window_close_callback (GLFWwindow* _window)
        {
            if (this->preventWindowCloseWithButton == false) {
                std::cout << "User requested exit\n";
                this->readyToFinish = true;
            } else {
                std::cout << "Ignoring user request to exit (Visual::preventWindowCloseWithButton)\n";
            }
        }

        virtual void scroll_callback (GLFWwindow* _window, double xoffset, double yoffset)
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
        virtual void key_callback_extra (GLFWwindow* _window, int key, int scancode, int action, int mods) {}
        //! Extra mousebutton callback handling, making it easy for client programs to implement their own actions
        virtual void mouse_button_callback_extra (GLFWwindow* _window, int button, int action, int mods) {}
    };

} // namespace morph
