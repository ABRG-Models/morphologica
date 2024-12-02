/*!
 * \file
 *
 * Awesome graphics code for high performance graphing and visualisation. Uses modern OpenGL and the
 * library GLFW for window management (or can be owned by a widget such as a QOpenGLWidget).
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

// Normally, a morph::Visual is the *owner* of a GLFW window in which it does its rendering.
//
// "OWNED_MODE" means that the morph::Visual is itself owned by a windowing system of some sort. At
// present, that can be a QOpenGLWidget or a wxWidget. It could in principle be any other window
// drawing system that's able to provide an OpenGL context for morph::Visual to render into.
//
// Otherwise (and by default), if OWNED_MODE is NOT defined, we include glfw3 headers and
// morph::Visual is the owner of a Window provided by GLFW.

#ifdef _glfw3_h_ // glfw3 has already been externally included
# ifdef OWNED_MODE
#  error "glfw3 has been #included but OWNED_MODE is defined"
# endif
#else // glfw3 not yet included
# ifndef OWNED_MODE
#  define GLFW_INCLUDE_NONE // Here, we tell GLFW that we will explicitly include GL3/gl3.h and GL/glext.h
#  include <GLFW/glfw3.h>
#  ifndef VISUAL_MANAGES_GLFW
#   define VISUAL_MANAGES_GLFW 1 // Used in VisualResources.h
#  endif
# endif
#endif // _glfw3_h_

#if defined __gl3_h_ || defined __gl_h_
// GL headers appear to have been externally included.
#else
// Include the correct GL headers before VisualCommon.h (VisualModel.h will bring these in, too)
# ifndef USE_GLEW
#  ifdef __OSX__
#   include <OpenGL/gl3.h>
#  else
#   include <GL3/gl3.h>
#   include <GL/glext.h>
#  endif
# endif
#endif // GL headers

#include <morph/gl/version.h>
#include <morph/VisualModel.h>
#include <morph/TextFeatures.h>
#include <morph/TextGeometry.h>
#include <morph/VisualTextModel.h> // includes VisualResources.h
#include <morph/VisualCommon.h>
#include <morph/gl/shaders.h> // for ShaderInfo/LoadShaders
#include <morph/keys.h>
#include <morph/version.h>

#include <morph/VisualResources.h>
#include <nlohmann/json.hpp>
#include <morph/CoordArrows.h>
#include <morph/Quaternion.h>
#include <morph/TransformMatrix.h>
#include <morph/vec.h>
#include <morph/tools.h>

#include <string>
#include <array>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <cstddef>

#include <morph/VisualDefaultShaders.h>
#ifndef OWNED_MODE
#include <mutex>
#endif

// Use Lode Vandevenne's PNG encoder
#define LODEPNG_NO_COMPILE_DECODER 1
#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS 1
#include <morph/lodepng.h>

namespace morph {

    //! Whether to render with perspective or orthographic (or even a cylindrical projection)
    enum class perspective_type
    {
        perspective,
        orthographic,
        cylindrical
    };

#ifndef OWNED_MODE
    // The default is to use a GLFW window which is owned by morph::Visual.
    using win_t = GLFWwindow;
    // else: The window is a widget within a wider Qt or wx system. We are owned by the
    // widget. win_t should be defined (with a using win_t = something line) externally.
#endif

    /*!
     * Visual 'scene' class
     *
     * A class for visualising computational models on an OpenGL screen.
     *
     * Each Visual will have its own GLFW window and is essentially a "scene" containing a number of
     * objects. One object might be the visualisation of some data expressed over a HexGrid. Another
     * could be a GraphVisual object. The class handles mouse events to allow the user to rotate and
     * translate the scene, as well as use keys to generate particular effects/views.
     *
     * It's possible to set the background colour of the scene (Visual::bgcolour), the location of
     * the objects in the scene (Visual::setSceneTransZ and friends) and the position and field of
     * view of the 'camera' (Visual::zNear, Visual::zFar and Visual::fov).
     *
     * \tparam glver The OpenGL version, encoded as a single int (see morph::gl::version)
     */
    template <int glver = morph::gl::version_4_1>
    class Visual
    {
        using sc = std::chrono::steady_clock;

    public:
        /*!
         * Default constructor is used when incorporating Visual inside a QWidget.  We have to wait
         * on calling init functions until an OpenGL environment is guaranteed to exist.
         */
        Visual() { }

        /*!
         * Construct a new visualiser. The rule is 1 window to one Visual object. So, this creates a
         * new window and a new OpenGL context.
         */
        Visual (const int width, const int height, const std::string& _title, const bool _version_stdout = true)
            : window_w(width)
            , window_h(height)
            , title(_title)
            , version_stdout(_version_stdout)
        {
            this->init_resources();
            this->init_gl();
        }

        /*!
         * Construct with specified coordinate arrows offset (caOffset) lengths (caLength),
         * thickness scaling factor (caThickness) and coordinate arrow 'm' size, caEm.
         */
        Visual (const int width, const int height, const std::string& _title,
                const morph::vec<float, 2> caOffset, const morph::vec<float, 3> caLength,
                const float caThickness, const float caEm, const bool _version_stdout = true)
            : window_w(width)
            , window_h(height)
            , title(_title)
            , version_stdout(_version_stdout)
            , coordArrowsOffset(caOffset)
            , coordArrowsLength(caLength)
            , coordArrowsThickness(caThickness)
            , coordArrowsEm(caEm)
        {
            this->init_resources();
            this->init_gl();
        }

        //! Deconstructor destroys GLFW/Qt window and deregisters access to VisualResources
        virtual ~Visual()
        {
#ifndef OWNED_MODE
            this->setContext();
            glfwDestroyWindow (this->window);
#endif
            if (this->shaders.gprog) {
                glDeleteProgram (this->shaders.gprog);
                this->shaders.gprog = 0;
                this->active_gprog = morph::visgl::graphics_shader_type::none;
            }
            if (this->shaders.tprog) {
                glDeleteProgram (this->shaders.tprog);
                this->shaders.tprog = 0;
            }

            // Free up the Fonts associated with this morph::Visual
            morph::VisualResources<glver>::i().freetype_deinit (this);
        }

        // Public init that is given a context (window or widget) and then sets up the
        // VisualResource, shaders and so on.
        void init (morph::win_t* ctx)
        {
            this->window = ctx;
            this->init_resources();
            this->init_gl();
        }

        // Do one-time init of the Visual's resources. This gets/creates the VisualResources,
        // registers this visual with resources, calls init_window for any glfw stuff that needs to
        // happen, and lastly initializes the freetype code.
        void init_resources()
        {
            // VisualResources provides font management and GLFW management. Ensure it exists in memory.
            morph::VisualResources<glver>::i().create();

            // Set up the window that will present the OpenGL graphics. No-op in Qt-managed Visual,
            // but this has to happen BEFORE the call to VisualResources::freetype_init()
            this->init_window();

            // Now make sure that Freetype is set up
            morph::VisualResources<glver>::i().freetype_init (this);
        }

        //! Take a screenshot of the window. Return vec containing width * height or {-1, -1} on
        //! failure.
        morph::vec<int, 2> saveImage (const std::string& img_filename)
        {
#ifndef OWNED_MODE
            this->setContext();
#endif
            GLint viewport[4]; // current viewport
            glGetIntegerv (GL_VIEWPORT, viewport);
            morph::vec<int, 2> dims;
            dims[0] = viewport[2];
            dims[1] = viewport[3];
            auto bits = std::make_unique<GLubyte[]>(dims.product() * 4);
            auto rbits = std::make_unique<GLubyte[]>(dims.product() * 4);
            glFinish(); // finish all commands of OpenGL
            glPixelStorei (GL_PACK_ALIGNMENT, 1);
            glPixelStorei (GL_PACK_ROW_LENGTH, 0);
            glPixelStorei (GL_PACK_SKIP_ROWS, 0);
            glPixelStorei (GL_PACK_SKIP_PIXELS, 0);
            glReadPixels (0, 0, dims[0], dims[1], GL_RGBA, GL_UNSIGNED_BYTE, bits.get());
            for (int i = 0; i < dims[1]; ++i) {
                int rev_line = (dims[1] - i - 1) * 4 * dims[0];
                int for_line = i * 4 * dims[0];
                for (int j = 0; j < 4 * dims[0]; ++j) {
                    rbits[rev_line+j] = bits[for_line+j];
                }
            }
            unsigned int error = lodepng::encode (img_filename, rbits.get(), dims[0], dims[1]);
            if (error) {
                std::cerr << "encoder error " << error << ": " << lodepng_error_text (error) << std::endl;
                dims.set_from (-1);
                return dims;
            }
            return dims;
        }

#ifndef OWNED_MODE
        //! Make this Visual the current one, so that when creating/adding a visual model, the vao
        //! ids relate to the correct OpenGL context.
        void setContext() { glfwMakeContextCurrent (this->window); }
        // A callback friendly wrapper
        static void set_context (morph::Visual<glver>* _v) { _v->setContext(); };

        //! Lock the context  to prevent accessing the OpenGL context from multiple threads
        //! then obtain the context.
        void lockContext()
        {
            this->context_.lock();
            this->setContext();
        }

        //! Attempt to lock the context . If the lock is obtained, set the OpenGL context
        //! and return true. If the lock is not obtained, return false.
        bool tryLockContext()
        {
            if (this->context_.try_lock()) {
                this->setContext();
                return true;
            } else {
                return false;
            }
        }

        //! Release the OpenGL context and unlock the context .
        void unlockContext()
        {
            this->releaseContext();
            this->context_mutex.unlock();
        }

        //! Release the OpenGL context
        void releaseContext() { glfwMakeContextCurrent (nullptr); }
#endif

        /*!
         * Set up the passed-in VisualModel with functions that need access to Visual attributes.
         */
        template <typename T>
        void bindmodel (std::unique_ptr<T>& model)
        {
            model->set_parent (this);
            model->get_shaderprogs = &morph::Visual<glver>::get_shaderprogs;
            model->get_gprog = &morph::Visual<glver>::get_gprog;
            model->get_tprog = &morph::Visual<glver>::get_tprog;
#ifndef OWNED_MODE
            model->setContext = &morph::Visual<glver>::set_context;
#endif
        }

        /*!
         * Add a VisualModel to the scene as a unique_ptr. The Visual object takes ownership of the
         * unique_ptr. The index into Visual::vm is returned.
         */
        template <typename T>
        unsigned int addVisualModelId (std::unique_ptr<T>& model)
        {
            std::unique_ptr<morph::VisualModel<glver>> vmp = std::move(model);
            this->vm.push_back (std::move(vmp));
            unsigned int rtn = (this->vm.size()-1);
            return rtn;
        }
        /*!
         * Add a VisualModel to the scene as a unique_ptr. The Visual object takes ownership of the
         * unique_ptr. A non-owning pointer to the model is returned.
         */
        template <typename T>
        T* addVisualModel (std::unique_ptr<T>& model)
        {
            std::unique_ptr<morph::VisualModel<glver>> vmp = std::move(model);
            this->vm.push_back (std::move(vmp));
            return static_cast<T*>(this->vm.back().get());
        }

        /*!
         * VisualModel Getter
         *
         * For the given \a modelId, return a (non-owning) pointer to the visual model.
         *
         * \return VisualModel pointer
         */
        morph::VisualModel<glver>* getVisualModel (unsigned int modelId) { return (this->vm[modelId].get()); }

        //! Remove the VisualModel with ID \a modelId from the scene.
        void removeVisualModel (unsigned int modelId) { this->vm.erase (this->vm.begin() + modelId); }

        //! Remove the VisualModel whose pointer matches the VisualModel* modelPtr
        void removeVisualModel (morph::VisualModel<glver>* modelPtr)
        {
            unsigned int modelId = 0;
            bool found_model = false;
            for (modelId = 0; modelId < this->vm.size(); ++modelId) {
                if (this->vm[modelId].get() == modelPtr) {
                    found_model = true;
                    break;
                }
            }
            if (found_model == true) { this->vm.erase (this->vm.begin() + modelId); }
        }

        //! Add a label _text to the scene at position _toffset. Font features are
        //! defined by the tfeatures. Return geometry of the text.
        morph::TextGeometry addLabel (const std::string& _text,
                                      const morph::vec<float, 3>& _toffset,
                                      const morph::TextFeatures& tfeatures = morph::TextFeatures(0.01f))
        {
            if (this->shaders.tprog == 0) { throw std::runtime_error ("No text shader prog."); }
            auto tmup = std::make_unique<morph::VisualTextModel<glver>> (this, this->shaders.tprog, tfeatures);
            if (tfeatures.centre_horz == true) {
                morph::TextGeometry tg = tmup->getTextGeometry(_text);
                morph::vec<float, 3> centred_locn = _toffset;
                centred_locn[0] = -tg.half_width();
                tmup->setupText (_text, centred_locn, tfeatures.colour);
            } else {
                tmup->setupText (_text, _toffset, tfeatures.colour);
            }
            morph::VisualTextModel<glver>* tm = tmup.get();
            this->texts.push_back (std::move(tmup));
            return tm->getTextGeometry();
        }

        //! Add a label _text to the scene at position _toffset. Font features are
        //! defined by the tfeatures. Return geometry of the text. The pointer tm is a
        //! return value that allows client code to change the text after the label has been added.
        morph::TextGeometry addLabel (const std::string& _text,
                                      const morph::vec<float, 3>& _toffset,
                                      morph::VisualTextModel<glver>*& tm,
                                      const morph::TextFeatures& tfeatures = morph::TextFeatures(0.01f))
        {
            if (this->shaders.tprog == 0) { throw std::runtime_error ("No text shader prog."); }
            auto tmup = std::make_unique<morph::VisualTextModel<glver>> (this, this->shaders.tprog, tfeatures);
            if (tfeatures.centre_horz == true) {
                morph::TextGeometry tg = tmup->getTextGeometry(_text);
                morph::vec<float, 3> centred_locn = _toffset;
                centred_locn[0] = -tg.half_width();
                tmup->setupText (_text, centred_locn, tfeatures.colour);
            } else {
                tmup->setupText (_text, _toffset, tfeatures.colour);
            }
            tm = tmup.get();
            this->texts.push_back (std::move(tmup));
            return tm->getTextGeometry();
        }

#ifndef OWNED_MODE
        /*
         * A note on setContext() in keepOpen/poll/waitevents/wait:
         *
         * I considered automatically calling setContext in these functions. However, the event
         * queue is not necessarily bound to the context (it depends on the platform), so I will
         * leave these as they are. The call to render() inside keepOpen() WILL correctly induce a
         * setContext() call.
         */

        /*!
         * Keep on rendering until readToFinish is set true. Used to keep a window open, and
         * responsive, while displaying the result of a simulation. FIXME: This won't work for two
         * or more windows because it will block.
         */
        void keepOpen()
        {
            while (this->readyToFinish == false) {
                glfwWaitEventsTimeout (0.01667); // 16.67 ms ~ 60 Hz
                this->render();
            }
        }

        //! Wrapper around the glfw polling function
        void poll() { glfwPollEvents(); }
        //! A wait-for-events with a timeout wrapper
        void waitevents (const double& timeout) { glfwWaitEventsTimeout (timeout); }
        //! Collect events for timeout, returning after *all* the time elapsed
        void wait (const double& timeout)
        {
            sc::time_point t0 = sc::now();
            sc::time_point t1 = sc::now();
            int timeout_us = timeout * 1000000;
            while (std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() < timeout_us) {
                glfwWaitEventsTimeout (timeout/10.0);
                t1 = sc::now();
            }
        }
#endif

        void set_cursorpos (double _x, double _y) { this->cursorpos = {static_cast<float>(_x), static_cast<float>(_y)}; }

        //! A callback function
        static void callback_render (morph::Visual<glver>* _v) { _v->render(); };

        //! Render the scene
        void render()
        {
#ifndef OWNED_MODE
            this->setContext();
#endif

#ifdef __OSX__
            // https://stackoverflow.com/questions/35715579/opengl-created-window-size-twice-as-large
            const double retinaScale = 2; // deals with quadrant issue on osx
#else
            const double retinaScale = 1; // Qt has devicePixelRatio() to get retinaScale.
#endif
            if (this->ptype == perspective_type::orthographic || this->ptype == perspective_type::perspective) {
                if (this->active_gprog != morph::visgl::graphics_shader_type::projection2d) {
                    if (this->shaders.gprog) { glDeleteProgram (this->shaders.gprog); }
                    this->shaders.gprog = morph::gl::LoadShaders (this->proj2d_shader_progs);
                    this->active_gprog = morph::visgl::graphics_shader_type::projection2d;
                }
            } else if (this->ptype == perspective_type::cylindrical) {
                if (this->active_gprog != morph::visgl::graphics_shader_type::cylindrical) {
                    if (this->shaders.gprog) { glDeleteProgram (this->shaders.gprog); }
                    this->shaders.gprog = morph::gl::LoadShaders (this->cyl_shader_progs);
                    this->active_gprog = morph::visgl::graphics_shader_type::cylindrical;
                }
            }

            glUseProgram (this->shaders.gprog);

            // Can't do this in a new thread:
            glViewport (0, 0, this->window_w * retinaScale, this->window_h * retinaScale);

            // Set the perspective
            if (this->ptype == perspective_type::orthographic) {
                this->setOrthographic();
            } else if (this->ptype == perspective_type::perspective) {
                this->setPerspective();
            } else if (this->ptype == perspective_type::cylindrical) {
                // Set cylindrical-specific uniforms
                GLint loc_campos = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("cyl_cam_pos"));
                if (loc_campos != -1) { glUniform4fv (loc_campos, 1, this->cyl_cam_pos.data()); }
                GLint loc_cyl_radius = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("cyl_radius"));
                if (loc_cyl_radius != -1) { glUniform1f (loc_cyl_radius, this->cyl_radius); }
                GLint loc_cyl_height = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("cyl_height"));
                if (loc_cyl_height != -1) { glUniform1f (loc_cyl_height, this->cyl_height); }
            } else {
                throw std::runtime_error ("Unknown projection");
            }

            // Calculate model view transformation - transforming from "model space" to "worldspace".
            morph::TransformMatrix<float> sceneview;
            if (this->ptype == perspective_type::orthographic || this->ptype == perspective_type::perspective) {
                // This line translates from model space to world space. Avoid in cyl?
                sceneview.translate (this->scenetrans); // send backwards into distance
            }
            // And this rotation completes the transition from model to world
            sceneview.rotate (this->rotation);

            // Clear color buffer and **also depth buffer**
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Set the background colour:
            glClearBufferfv (GL_COLOR, 0, bgcolour.data());

            // Lighting shader variables
            //
            // Ambient light colour
            GLint loc_lightcol = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("light_colour"));
            if (loc_lightcol != -1) { glUniform3fv (loc_lightcol, 1, this->light_colour.data()); }
            // Ambient light intensity
            GLint loc_ai = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("ambient_intensity"));
            if (loc_ai != -1) { glUniform1f (loc_ai, this->ambient_intensity); }
            // Diffuse light position
            GLint loc_dp = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("diffuse_position"));
            if (loc_dp != -1) { glUniform3fv (loc_dp, 1, this->diffuse_position.data()); }
            // Diffuse light intensity
            GLint loc_di = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("diffuse_intensity"));
            if (loc_di != -1) { glUniform1f (loc_di, this->diffuse_intensity); }

            // Switch to text shader program and set the projection matrix
            glUseProgram (this->shaders.tprog);
            GLint loc_p = glGetUniformLocation (this->shaders.tprog, static_cast<const GLchar*>("p_matrix"));
            if (loc_p != -1) { glUniformMatrix4fv (loc_p, 1, GL_FALSE, this->projection.mat.data()); }

            // Switch back to the regular shader prog and render the VisualModels.
            glUseProgram (this->shaders.gprog);

            // Set the projection matrix just once
            loc_p = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("p_matrix"));
            if (loc_p != -1) { glUniformMatrix4fv (loc_p, 1, GL_FALSE, this->projection.mat.data()); }

            if ((this->ptype == perspective_type::orthographic || this->ptype == perspective_type::perspective)
                && this->showCoordArrows == true) {
                // Ensure coordarrows centre sphere will be visible on BG:
                this->coordArrows->setColourForBackground (this->bgcolour);

                if (this->coordArrowsInScene == true) {
                    this->coordArrows->setSceneMatrix (sceneview);
                } else {
                    this->positionCoordArrows();
                }
                this->coordArrows->render();
            }

            morph::TransformMatrix<float> scenetransonly;
            scenetransonly.translate (this->scenetrans);

            auto vmi = this->vm.begin();
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

            morph::vec<float, 3> v0 = this->textPosition ({-0.8f, 0.8f});
            if (this->showTitle == true) {
                // Render the title text
                this->textModel->setSceneTranslation (v0);
                this->textModel->setVisibleOn (this->bgcolour);
                this->textModel->render();
            }

            auto ti = this->texts.begin();
            while (ti != this->texts.end()) {
                (*ti)->setSceneTranslation (v0);
                (*ti)->setVisibleOn (this->bgcolour);
                (*ti)->render();
                ++ti;
            }

#ifndef OWNED_MODE
            glfwSwapBuffers (this->window);
#endif
        }

        //! Compute a translation vector for text position, using Visual::text_z.
        morph::vec<float, 3> textPosition (const morph::vec<float, 2> p0_coord)
        {
            // For the depth at which a text object lies, use this->text_z.  Use forward
            // projection to determine the correct z coordinate for the inverse
            // projection.
            morph::vec<float, 4> point =  { 0.0f, 0.0f, this->text_z, 1.0f };
            morph::vec<float, 4> pp = this->projection * point;
            float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.
            // Construct the point for the location of the text
            morph::vec<float, 4> p0 = { p0_coord.x(), p0_coord.y(), coord_z, 1.0f };
            // Inverse project the point
            morph::vec<float, 3> v0;
            v0.set_from (this->invproj * p0);
            return v0;
        }

        //! The OpenGL shader programs have an integer ID and are stored in a simple struct. There's
        //! one for graphical objects and a text shader program, which uses textures to draw text on
        //! quads.
        morph::visgl::visual_shaderprogs shaders;
        //! Which shader is active for graphics shading?
        morph::visgl::graphics_shader_type active_gprog = morph::visgl::graphics_shader_type::none;
        //! Stores the info required to load the 2D projection shader
        std::vector<morph::gl::ShaderInfo> proj2d_shader_progs;
        //! Stores the info required to load the text shader
        std::vector<morph::gl::ShaderInfo> text_shader_progs;

        //! Stores the info required to load the cylindrical projection shader
        std::vector<morph::gl::ShaderInfo> cyl_shader_progs;
        //! Passed to the cyl_shader_progs as a uniform to define the location of the cylindrical
        //! projection camera
        morph::vec<float, 4> cyl_cam_pos = { 0.0f, 0.0f, 0.0f, 1.0f };
        //! Default cylindrical camera position
        morph::vec<float, 4> cyl_cam_pos_default = { 0.0f, 0.0f, 0.0f, 1.0f };
        //! The radius of the 'cylindrical projection screen' around the camera position
        float cyl_radius = 0.005f;
        //! The height of the 'cylindrical projection screen'
        float cyl_height = 0.01f;

        // These static functions will be set as callbacks in each VisualModel object.
        static morph::visgl::visual_shaderprogs get_shaderprogs (morph::Visual<glver>* _v) { return _v->shaders; };
        static GLuint get_gprog (morph::Visual<glver>* _v) { return _v->shaders.gprog; };
        static GLuint get_tprog (morph::Visual<glver>* _v) { return _v->shaders.tprog; };

        //! The colour of ambient and diffuse light sources
        morph::vec<float, 3> light_colour = { 1.0f, 1.0f, 1.0f };
        //! Strength of the ambient light
        float ambient_intensity = 1.0f;
        //! Position of a diffuse light source
        morph::vec<float, 3> diffuse_position = { 5.0f, 5.0f, 15.0f };
        //! Strength of the diffuse light source
        float diffuse_intensity = 0.0f;

        //! Compute position and rotation of coordinate arrows in the bottom left of the screen
        void positionCoordArrows()
        {
            // Find out the location of the bottom left of the screen and make the coord
            // arrows stay put there.

            // Add the depth at which the object lies.  Use forward projection to determine the
            // correct z coordinate for the inverse projection. This assumes only one object.
            morph::vec<float, 4> point =  { 0.0f, 0.0f, this->scenetrans.z(), 1.0f };
            morph::vec<float, 4> pp = this->projection * point;
            float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.

            // Construct the point for the location of the coord arrows
            morph::vec<float, 4> p0 = { this->coordArrowsOffset.x(), this->coordArrowsOffset.y(), coord_z, 1.0f };
            // Inverse project
            morph::vec<float, 3> v0;
            v0.set_from ((this->invproj * p0));
            // Translate the scene for the CoordArrows such that they sit in a single position on
            // the screen
            this->coordArrows->setSceneTranslation (v0);
            // Apply rotation to the coordArrows model
            this->coordArrows->setViewRotation (this->rotation);
        }

        //! Set up a perspective projection based on window width and height
        void setPerspective()
        {
            // Calculate aspect ratio
            float aspect = static_cast<float>(this->window_w) / static_cast<float>(this->window_h ? this->window_h : 1);
            // Reset projection
            this->projection.setToIdentity();
            // Set perspective projection
            this->projection.perspective (this->fov, aspect, this->zNear, this->zFar);
            // Compute the inverse projection matrix
            this->invproj = this->projection.invert();
        }

        //! Set an orthographic projection
        void setOrthographic()
        {
            this->projection.setToIdentity();
            this->projection.orthographic (this->ortho_lb, this->ortho_rt, this->zNear, this->zFar);
            this->invproj = this->projection.invert();
        }

        //! Set to true when the program should end
        bool readyToFinish = false;

        //! Set true to disable the 'X' button on the Window from exiting the program
        bool preventWindowCloseWithButton = false;

        /*
         * User-settable projection values for the near clipping distance, the far clipping distance
         * and the field of view of the camera.
         */

        float zNear = 0.001f;
        float zFar = 300.0f;
        float fov = 30.0f;

        //! Set to true to show the coordinate arrows
        bool showCoordArrows = false;

        //! If true, then place the coordinate arrows at the origin of the scene, rather than offset.
        bool coordArrowsInScene = false;

        //! Set to true to show the title text within the scene
        bool showTitle = false;

        //! If true, output some user information to stdout (e.g. user requested quit)
        bool user_info_stdout = true;

        //! How big should the steps in scene translation be when scrolling?
        float scenetrans_stepsize = 0.1f;

        //! If you set this to true, then the mouse movements won't change scenetrans or rotation.
        bool sceneLocked = false;

        //! Can change this to orthographic
        perspective_type ptype = perspective_type::perspective;

        //! Orthographic screen left-bottom coordinate (you can change these to encapsulate your models)
        morph::vec<float, 2> ortho_lb = { -1.3f, -1.0f };
        //! Orthographic screen right-top coordinate
        morph::vec<float, 2> ortho_rt = { 1.3f, 1.0f };

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

        //! Set the scene's x and y values at the same time.
        void setSceneTransXY (const float _x, const float _y)
        {
            this->scenetrans[0] = _x;
            this->scenetrans[1] = _y;
            this->scenetrans_default[0] = _x;
            this->scenetrans_default[1] = _y;
        }
        //! Set the scene's y value. Use this to shift your scene objects left or right
        void setSceneTransX (const float _x) { this->scenetrans[0] = _x; this->scenetrans_default[0] = _x; }
        //! Set the scene's y value. Use this to shift your scene objects up and down
        void setSceneTransY (const float _y) { this->scenetrans[1] = _y; this->scenetrans_default[1] = _y; }
        //! Set the scene's z value. Use this to bring the 'camera' closer to your scene
        //! objects (that is, your morph::VisualModel objects).
        void setSceneTransZ (const float _z)
        {
            if (_z > 0.0f) {
                std::cerr << "WARNING setSceneTransZ(): Normally, the default z value is negative.\n";
            }
            this->scenetrans[2] = _z;
            this->scenetrans_default[2] = _z;
        }
        void setSceneTrans (float _x, float _y, float _z)
        {
            if (_z > 0.0f) {
                std::cerr << "WARNING setSceneTrans(): Normally, the default z value is negative.\n";
            }
            this->scenetrans[0] = _x;
            this->scenetrans_default[0] = _x;
            this->scenetrans[1] = _y;
            this->scenetrans_default[1] = _y;
            this->scenetrans[2] = _z;
            this->scenetrans_default[2] = _z;
        }
        void setSceneTrans (const morph::vec<float, 3>& _xyz)
        {
            if (_xyz[2] > 0.0f) {
                std::cerr << "WARNING setSceneTrans(vec<>&): Normally, the default z value is negative.\n";
            }
            this->scenetrans = _xyz;
            this->scenetrans_default = _xyz;
        }

        void setSceneRotation (const morph::Quaternion<float>& _rotn)
        {
            this->rotation = _rotn;
            this->rotation_default = _rotn;
        }

        void lightingEffects (const bool effects_on = true)
        {
            this->ambient_intensity = effects_on ? 0.4f : 1.0f;
            this->diffuse_intensity = effects_on ? 0.6f : 0.0f;
        }

        //! Save all the VisualModels in this Visual out to a GLTF format file
        virtual void savegltf (const std::string& gltf_file)
        {
            std::ofstream fout;
            fout.open (gltf_file, std::ios::out|std::ios::trunc);
            if (!fout.is_open()) { throw std::runtime_error ("Visual::savegltf(): Failed to open file for writing"); }
            fout << "{\n  \"scenes\" : [ { \"nodes\" : [ ";
            for (std::size_t vmi = 0u; vmi < this->vm.size(); ++vmi) {
                fout << vmi << (vmi < this->vm.size()-1 ? ", " : "");
            }
            fout << " ] } ],\n";

            fout << "  \"nodes\" : [\n";
            // for loop over VisualModels "mesh" : 0, etc
            for (std::size_t vmi = 0u; vmi < this->vm.size(); ++vmi) {
                fout << "    { \"mesh\" : " << vmi
                     << ", \"translation\" : " << this->vm[vmi]->translation_str()
                     << (vmi < this->vm.size()-1 ? " },\n" : " }\n");
            }
            fout << "  ],\n";

            fout << "  \"meshes\" : [\n";
            // for each VisualModel:
            for (std::size_t vmi = 0u; vmi < this->vm.size(); ++vmi) {
                fout << "    { \"primitives\" : [ { \"attributes\" : { \"POSITION\" : " << 1+vmi*4
                     << ", \"COLOR_0\" : " << 2+vmi*4
                     << ", \"NORMAL\" : " << 3+vmi*4 << " }, \"indices\" : " << vmi*4 << ", \"material\": 0 } ] }"
                     << (vmi < this->vm.size()-1 ? ",\n" : "\n");
            }
            fout << "  ],\n";

            fout << "  \"buffers\" : [\n";
            for (std::size_t vmi = 0u; vmi < this->vm.size(); ++vmi) {
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
            for (std::size_t vmi = 0u; vmi < this->vm.size(); ++vmi) {
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
            for (std::size_t vmi = 0u; vmi < this->vm.size(); ++vmi) {
                this->vm[vmi]->computeVertexMaxMins();
                // indices
                fout << "    { ";
                fout << "\"bufferView\" : " << vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                // 5123 unsigned short, 5121 unsigned byte, 5125 unsigned int, 5126 float:
                fout << "\"componentType\" : 5125, ";
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
                 << "    \"generator\" : \"https://github.com/ABRG-Models/morphologica: morph::Visual::savegltf() (ver "
                 << morph::version_string() << ")\",\n"
                 << "    \"version\" : \"2.0\"\n" // This version is the *glTF* version.
                 << "  }\n";
            fout << "}\n";
            fout.close();
        }

        void set_winsize (int _w, int _h) { this->window_w = _w; this->window_h = _h; }

    protected:
        //! A vector of pointers to all the morph::VisualModels (HexGridVisual,
        //! ScatterVisual, etc) which are going to be rendered in the scene.
        std::vector<std::unique_ptr<morph::VisualModel<glver>>> vm;

    private:

        void init_window()
        {
#ifndef OWNED_MODE
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
#endif
        }

        // Initialize OpenGL shaders, set some flags (Alpha, Anti-aliasing), read in any external
        // state from json, and set up the coordinate arrows and any VisualTextModels that will be
        // required to render the Visual.
        void init_gl()
        {
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
            if (this->version_stdout == true) {
                unsigned char* glv = (unsigned char*)glGetString(GL_VERSION);
                std::cout << "This is version " << morph::version_string()
                          << " of morph::Visual<glver=" << morph::gl::version::vstring (glver)
                          << "> running on OpenGL Version " << glv << std::endl;
            }

#ifndef OWNED_MODE
            // Swap as fast as possible (fixes lag of scene with mouse movements)
            glfwSwapInterval (0);
#endif
            // Load up the shaders
            this->proj2d_shader_progs = {
                {GL_VERTEX_SHADER, "Visual.vert.glsl", morph::getDefaultVtxShader(glver), 0 },
                {GL_FRAGMENT_SHADER, "Visual.frag.glsl", morph::getDefaultFragShader(glver), 0 }
            };
            this->shaders.gprog = morph::gl::LoadShaders (this->proj2d_shader_progs);
            this->active_gprog = morph::visgl::graphics_shader_type::projection2d;

            // Alternative cylindrical shader for possible later use. (NB: not loaded immediately)
            this->cyl_shader_progs = {
                {GL_VERTEX_SHADER, "VisCyl.vert.glsl", morph::getDefaultCylVtxShader(glver), 0 },
                {GL_FRAGMENT_SHADER, "Visual.frag.glsl", morph::getDefaultFragShader(glver), 0 }
            };

            // A specific text shader is loaded for text rendering
            this->text_shader_progs = {
                {GL_VERTEX_SHADER, "VisText.vert.glsl", morph::getDefaultTextVtxShader(glver), 0 },
                {GL_FRAGMENT_SHADER, "VisText.frag.glsl" , morph::getDefaultTextFragShader(glver), 0 }
            };
            this->shaders.tprog = morph::gl::LoadShaders (this->text_shader_progs);

            // OpenGL options
            glEnable (GL_DEPTH_TEST);
            glEnable (GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable (GL_CULL_FACE);

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
            this->coordArrows = std::make_unique<morph::CoordArrows<glver>>();
            // For CoordArrows, because we don't add via Visual::addVisualModel(), we
            // have to set the get_shaderprogs function here:
            this->bindmodel (this->coordArrows);
            // And NOW we can proceed to init:
            this->coordArrows->init (this->coordArrowsLength, this->coordArrowsThickness, this->coordArrowsEm);
            morph::gl::Util::checkError (__FILE__, __LINE__);

            // Set up the title, which may or may not be rendered
            this->textModel = std::make_unique<morph::VisualTextModel<glver>> (this, this->shaders.tprog,
                                                                               morph::VisualFont::DVSans,
                                                                               0.035f, 64, morph::vec<float, 3>({0.0f, 0.0f, 0.0f}),
                                                                               this->title);
        }

    private:
        //! The window (and OpenGL context) for this Visual
        morph::win_t* window = nullptr;

#ifndef OWNED_MODE
        //! Context mutex to prevent contexts being acquired in a non-threadsafe manner.
        std::mutex context_mutex;
#endif

        //! Current window width
        int window_w = 640;
        //! Current window height
        int window_h = 480;

        //! The title for the Visual. Used in window title and if saving out 3D model or png image.
        std::string title = "morph::Visual";

        //! If true, output some version information (morphologica version, OpenGL version) to
        //! stdout. Private as this has no effect after init()
        bool version_stdout = true;

        //! The user's 'selected visual model'. For model specific changes to alpha and possibly colour
        unsigned int selectedVisualModel = 0u;

    protected:
        //! A little model of the coordinate axes.
        std::unique_ptr<morph::CoordArrows<glver>> coordArrows;

        //! Position coordinate arrows on screen. Configurable at morph::Visual construction.
        morph::vec<float, 2> coordArrowsOffset = { -0.8f, -0.8f };
        //! Length of coordinate arrows. Configurable at morph::Visual construction.
        morph::vec<float, 3> coordArrowsLength = { 0.1f, 0.1f, 0.1f };
        //! A factor used to slim (<1) or thicken (>1) the thickness of the axes of the CoordArrows.
        float coordArrowsThickness = 1.0f;
        //! Text size for x,y,z.
        float coordArrowsEm = 0.01f;

    private:
        //! A VisualTextModel for a title text.
        std::unique_ptr<morph::VisualTextModel<glver>> textModel = nullptr;
        //! Text models for labels
        std::vector<std::unique_ptr<morph::VisualTextModel<glver>>> texts;

        /*
         * Variables to manage projection and rotation of the scene
         */

        //! Current cursor position
        morph::vec<float,2> cursorpos = { 0.0f, 0.0f };

        //! The default z position for VisualModels should be 'away from the screen' (negative) so we can see them!
        constexpr static float zDefault = -5.0f;

        //! Holds the translation coordinates for the current location of the entire scene
        morph::vec<float, 3> scenetrans = {0.0f, 0.0f, zDefault};

        //! Default for scenetrans. This is a scene position that can be reverted to, to
        //! 'reset the view'. This is copied into scenetrans when user presses Ctrl-a.
        morph::vec<float, 3> scenetrans_default = { 0.0f, 0.0f, zDefault };

        //! The world depth at which text objects should be rendered
        float text_z = -1.0f;

        //! When true, cursor movements induce rotation of scene
        bool rotateMode = false;

        //! When true, rotations about the third axis are possible.
        bool rotateModMode = false;

        //! When true, cursor movements induce translation of scene
        bool translateMode = false;

        //! Screen coordinates of the position of the last mouse press
        morph::vec<float,2> mousePressPosition = { 0.0f, 0.0f };

        //! The current rotation axis. World frame?
        morph::vec<float, 3> rotationAxis = { 0.0f, 0.0f, 0.0f };

        //! A rotation quaternion. You could have guessed that, right?
        morph::Quaternion<float> rotation;

        //! The default rotation of the scene
        morph::Quaternion<float> rotation_default;

        //! A rotation that is saved between mouse button callbacks
        morph::Quaternion<float> savedRotation;

        //! The projection matrix is a member of this class
        morph::TransformMatrix<float> projection;

        //! The inverse of the projection
        morph::TransformMatrix<float> invproj;

        //! A scene transformation
        morph::TransformMatrix<float> scene;
        //! Scene transformation inverse
        morph::TransformMatrix<float> invscene;

#ifndef OWNED_MODE
        /*
         * GLFW callback dispatch functions
         */
    private:
        static void key_callback_dispatch (GLFWwindow* _window, int key, int scancode, int action, int mods)
        {
            Visual<glver>* self = static_cast<Visual<glver>*>(glfwGetWindowUserPointer (_window));
            if (self->key_callback (key, scancode, action, mods)) {
                self->render();
            }
        }
        static void mouse_button_callback_dispatch (GLFWwindow* _window, int button, int action, int mods)
        {
            Visual<glver>* self = static_cast<Visual<glver>*>(glfwGetWindowUserPointer (_window));
            self->mouse_button_callback (button, action, mods);
        }
        static void cursor_position_callback_dispatch (GLFWwindow* _window, double x, double y)
        {
            Visual<glver>* self = static_cast<Visual<glver>*>(glfwGetWindowUserPointer (_window));
            if (self->cursor_position_callback (x, y)) {
                self->render();
            }
        }
        static void window_size_callback_dispatch (GLFWwindow* _window, int width, int height)
        {
            Visual<glver>* self = static_cast<Visual<glver>*>(glfwGetWindowUserPointer (_window));
            if (self->window_size_callback (width, height)) {
                self->render();
            }
        }
        static void window_close_callback_dispatch (GLFWwindow* _window)
        {
            Visual<glver>* self = static_cast<Visual<glver>*>(glfwGetWindowUserPointer (_window));
            self->window_close_callback();
        }
        static void scroll_callback_dispatch (GLFWwindow* _window, double xoffset, double yoffset)
        {
            Visual<glver>* self = static_cast<Visual<glver>*>(glfwGetWindowUserPointer (_window));
            if (self->scroll_callback (xoffset, yoffset)) {
                self->render();
            }
        }

#endif // GLFW-specific callback dispatch functions

    public:

        /*
         * Generic callback handlers
         */

        using keyaction = morph::keyaction;
        using keymod = morph::keymod;
        using key = morph::key;
        // The key_callback handler uses GLFW codes, but they're in a morph header (keys.h)
        virtual bool key_callback (int _key, int scancode, int action, int mods)
        {
            bool needs_render = false;

#ifndef OWNED_MODE // If Visual is 'owned' then the owning system deals with program exit
            // Exit action
            if (_key == key::q && (mods & keymod::control) && action == keyaction::press) {
                this->signal_to_quit();
            }
#endif
            if (!this->sceneLocked && _key == key::c  && (mods & keymod::control) && action == keyaction::press) {
                this->showCoordArrows = !this->showCoordArrows;
                needs_render = true;
            }

            if (_key == key::h && (mods & keymod::control) && action == keyaction::press) {
                // Help to stdout:
                std::cout << "Ctrl-h: Output this help to stdout\n";
                std::cout << "Mouse-primary: rotate mode (use Ctrl to change axis)\n";
                std::cout << "Mouse-secondary: translate mode\n";
#ifndef OWNED_MODE // If Visual is 'owned' then the owning system deals with program exit
                std::cout << "Ctrl-q: Request exit\n";
#endif
                std::cout << "Ctrl-l: Toggle the scene lock\n";
                std::cout << "Ctrl-c: Toggle coordinate arrows\n";
                std::cout << "Ctrl-s: Take a snapshot\n";
                std::cout << "Ctrl-m: Save 3D models in .gltf format (open in e.g. blender)\n";
                std::cout << "Ctrl-a: Reset default view\n";
                std::cout << "Ctrl-o: Reduce field of view\n";
                std::cout << "Ctrl-p: Increase field of view\n";
                std::cout << "Ctrl-y: Cycle perspective\n";
                std::cout << "Ctrl-z: Show the current scenetrans/rotation and save to /tmp/Visual.json\n";
                std::cout << "Ctrl-u: Reduce zNear cutoff plane\n";
                std::cout << "Ctrl-i: Increase zNear cutoff plane\n";
                std::cout << "F1-F10: Select model index (with shift: toggle hide)\n";
                std::cout << "Shift-Left: Decrease opacity of selected model\n";
                std::cout << "Shift-Right: Increase opacity of selected model\n";
                std::cout << "Shift-Up: Double cyl proj radius\n";
                std::cout << "Shift-Down: Halve cyl proj radius\n";
                std::cout << "Ctrl-Up: Double cyl proj height\n";
                std::cout << "Ctrl-Down: Halve cyl proj height\n";
                std::cout << std::flush;
            }

            if (_key == key::l && (mods & keymod::control) && action == keyaction::press) {
                this->sceneLocked = this->sceneLocked ? false : true;
                std::cout << "Scene is now " << (this->sceneLocked ? "" : "un-") << "locked\n";
            }

            if (_key == key::s && (mods & keymod::control) && action == keyaction::press) {
                std::string fname (this->title);
                morph::Tools::stripFileSuffix (fname);
                fname += ".png";
                // Make fname 'filename safe'
                morph::Tools::conditionAsFilename (fname);
                this->saveImage (fname);
                std::cout << "Saved image to '" << fname << "'\n";
            }

            // Save gltf 3D file
            if (_key == key::m && (mods & keymod::control) && action == keyaction::press) {
                std::string gltffile = this->title;
                morph::Tools::stripFileSuffix (gltffile);
                gltffile += ".gltf";
                morph::Tools::conditionAsFilename (gltffile);
                this->savegltf (gltffile);
                std::cout << "Saved 3D file '" << gltffile << "'\n";
            }

            if (_key == key::z && (mods & keymod::control) && action == keyaction::press) {
                std::cout << "Scenetrans setup code:\n    v.setSceneTrans (morph::vec<float,3>{ float{"
                          << this->scenetrans.x() << "}, float{"
                          << this->scenetrans.y() << "}, float{"
                          << this->scenetrans.z()
                          << "} });"
                          <<  "\n    v.setSceneRotation (morph::Quaternion<float>{ float{"
                          << this->rotation.w << "}, float{" << this->rotation.x << "}, float{"
                          << this->rotation.y << "}, float{" << this->rotation.z << "} });\n";
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
            if (_key == key::f1 && action == keyaction::press) {
                this->selectedVisualModel = 0;
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (_key == key::f2 && action == keyaction::press) {
                if (this->vm.size() > 1) { this->selectedVisualModel = 1; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (_key == key::f3 && action == keyaction::press) {
                if (this->vm.size() > 2) { this->selectedVisualModel = 2; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (_key == key::f4 && action == keyaction::press) {
                if (this->vm.size() > 3) { this->selectedVisualModel = 3; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (_key == key::f5 && action == keyaction::press) {
                if (this->vm.size() > 4) { this->selectedVisualModel = 4; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (_key == key::f6 && action == keyaction::press) {
                if (this->vm.size() > 5) { this->selectedVisualModel = 5; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (_key == key::f7 && action == keyaction::press) {
                if (this->vm.size() > 6) { this->selectedVisualModel = 6; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (_key == key::f8 && action == keyaction::press) {
                if (this->vm.size() > 7) { this->selectedVisualModel = 7; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (_key == key::f9 && action == keyaction::press) {
                if (this->vm.size() > 8) { this->selectedVisualModel = 8; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            } else if (_key == key::f10 && action == keyaction::press) {
                if (this->vm.size() > 9) { this->selectedVisualModel = 9; }
                std::cout << "Selected visual model index " << this->selectedVisualModel << std::endl;
            }

            // Toggle hide model if the shift key is down
            if ((_key == key::f10 || _key == key::f1 || _key == key::f2 || _key == key::f3
                 || _key == key::f4 || _key == key::f5 || _key == key::f6
                 || _key == key::f7 || _key == key::f8 || _key == key::f9)
                && action == keyaction::press && (mods & keymod::shift)) {
                this->vm[this->selectedVisualModel]->toggleHide();
            }

            // Increment/decrement alpha for selected model
            if (_key == key::left && (action == keyaction::press || action == keyaction::repeat) && (mods & keymod::shift)) {
                if (!this->vm.empty()) { this->vm[this->selectedVisualModel]->decAlpha(); }
            }
            if (_key == key::right && (action == keyaction::press || action == keyaction::repeat) && (mods & keymod::shift)) {
                if (!this->vm.empty()) { this->vm[this->selectedVisualModel]->incAlpha(); }
            }

            // Cyl (and possibly spherical) projection radius
            if (_key == key::up && (action == keyaction::press || action == keyaction::repeat) && (mods & keymod::shift)) {
                this->cyl_radius *= 2.0f;
                std::cout << "cyl_radius is now " << this->cyl_radius << std::endl;
            }
            if (_key == key::down && (action == keyaction::press || action == keyaction::repeat) && (mods & keymod::shift)) {
                this->cyl_radius *= 0.5f;
                std::cout << "cyl_radius is now " << this->cyl_radius << std::endl;
            }

            // Cyl projection view height
            if (_key == key::up && (action == keyaction::press || action == keyaction::repeat) && (mods & keymod::control)) {
                this->cyl_height *= 2.0f;
                std::cout << "cyl_height is now " << this->cyl_height << std::endl;
            }
            if (_key == key::down && (action == keyaction::press || action == keyaction::repeat) && (mods & keymod::control)) {
                this->cyl_height *= 0.5f;
                std::cout << "cyl_height is now " << this->cyl_height << std::endl;
            }

            // Reset view to default
            if (!this->sceneLocked && _key == key::a && (mods & keymod::control) && action == keyaction::press) {
                std::cout << "Reset to default view\n";
                // Reset translation
                this->scenetrans = this->scenetrans_default;
                this->cyl_cam_pos = this->cyl_cam_pos_default;
                // Reset rotation
                this->rotation = this->rotation_default;

                needs_render = true;
            }

            if (!this->sceneLocked && _key == key::o && (mods & keymod::control) && action == keyaction::press) {
                this->fov -= 2;
                if (this->fov < 1.0) {
                    this->fov = 2.0;
                }
                std::cout << "FOV reduced to " << this->fov << std::endl;
            }
            if (!this->sceneLocked && _key == key::p && (mods & keymod::control) && action == keyaction::press) {
                this->fov += 2;
                if (this->fov > 179.0) {
                    this->fov = 178.0;
                }
                std::cout << "FOV increased to " << this->fov << std::endl;
            }
            if (!this->sceneLocked && _key == key::u && (mods & keymod::control) && action == keyaction::press) {
                this->zNear /= 2;
                std::cout << "zNear reduced to " << this->zNear << std::endl;
            }
            if (!this->sceneLocked && _key == key::i && (mods & keymod::control) && action == keyaction::press) {
                this->zNear *= 2;
                std::cout << "zNear increased to " << this->zNear << std::endl;
            }

            if (_key == key::y && (mods & keymod::control) && action == keyaction::press) {
                if (this->ptype == morph::perspective_type::perspective) {
                    this->ptype = morph::perspective_type::orthographic;
                } else if (this->ptype == morph::perspective_type::orthographic) {
                    this->ptype = morph::perspective_type::cylindrical;
                } else {
                    this->ptype = morph::perspective_type::perspective;
                }
                needs_render = true;
            }

            this->key_callback_extra (_key, scancode, action, mods);

            return needs_render;
        }

        virtual bool cursor_position_callback (double x, double y)
        {
            this->cursorpos[0] = static_cast<float>(x);
            this->cursorpos[1] = static_cast<float>(y);

            morph::vec<float, 3> mouseMoveWorld = { 0.0f, 0.0f, 0.0f };

            bool needs_render = false;

            // This is "rotate the scene" model. Will need "rotate one visual" mode.
            if (this->rotateMode) {
                // Convert mousepress/cursor positions (in pixels) to the range -1 -> 1:
                morph::vec<float, 2> p0_coord = this->mousePressPosition;
                p0_coord -= this->window_w * 0.5f;
                p0_coord /= this->window_w * 0.5f;
                morph::vec<float, 2> p1_coord = this->cursorpos;
                p1_coord -= this->window_w * 0.5f;
                p1_coord /= this->window_w * 0.5f;
                // Note: don't update this->mousePressPosition until user releases button.

                // Add the depth at which the object lies.  Use forward projection to determine the
                // correct z coordinate for the inverse projection. This assumes only one object.
                morph::vec<float, 4> point =  { 0.0f, 0.0f, this->scenetrans.z(), 1.0f };
                morph::vec<float, 4> pp = this->projection * point;
                float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.

                // Construct two points for the start and end of the mouse movement
                morph::vec<float, 4> p0 = { p0_coord[0], p0_coord[1], coord_z, 1.0f };
                morph::vec<float, 4> p1 = { p1_coord[0], p1_coord[1], coord_z, 1.0f };

                // Apply the inverse projection to get two points in the world frame of reference
                // for the mouse movement
                morph::vec<float, 4> v0 = this->invproj * p0;
                morph::vec<float, 4> v1 = this->invproj * p1;

                // This computes the difference betwen v0 and v1, the 2 mouse positions in the world
                // space. Note the swap between x and y
                if (this->rotateModMode) {
                    // Sort of "rotate the page" mode.
                    mouseMoveWorld[2] = -((v1[1]/v1[3]) - (v0[1]/v0[3])) + ((v1[0]/v1[3]) - (v0[0]/v0[3]));
                } else {
                    mouseMoveWorld[1] = -((v1[0]/v1[3]) - (v0[0]/v0[3]));
                    mouseMoveWorld[0] = -((v1[1]/v1[3]) - (v0[1]/v0[3]));
                }

                // Rotation axis is perpendicular to the mouse position difference vector BUT we
                // have to project into the model frame to determine how to rotate the model!
                float rotamount = mouseMoveWorld.length() * 40.0f; // chosen in degrees
                // Calculate new rotation axis as weighted sum
                this->rotationAxis = (mouseMoveWorld * rotamount);
                this->rotationAxis.renormalize();

                // Now inverse apply the rotation of the scene to the rotation axis (vec<float,3>),
                // so that we rotate the model the right way.
                morph::vec<float, 4> tmp_4D = this->invscene * this->rotationAxis;
                this->rotationAxis.set_from (tmp_4D); // Set rotationAxis from 4D result

                // Update rotation from the saved position.
                this->rotation = this->savedRotation;
                morph::Quaternion<float> rotationQuaternion(this->rotationAxis,
                                                            -rotamount * morph::mathconst<float>::deg2rad);
                this->rotation.postmultiply (rotationQuaternion); // combines rotations
                needs_render = true;

            } else if (this->translateMode) { // allow only rotate OR translate for a single mouse movement

                // Convert mousepress/cursor positions (in pixels) to the range -1 -> 1:
                morph::vec<float, 2> p0_coord = this->mousePressPosition;
                p0_coord -= this->window_w * 0.5f;
                p0_coord /= this->window_w * 0.5f;
                morph::vec<float, 2> p1_coord = this->cursorpos;
                p1_coord -= this->window_w * 0.5f;
                p1_coord /= this->window_w * 0.5f;

                this->mousePressPosition = this->cursorpos;

                // Add the depth at which the object lies.  Use forward projection to determine the
                // correct z coordinate for the inverse projection. This assumes only one object.
                morph::vec<float, 4> point =  { 0.0f, 0.0f, this->scenetrans.z(), 1.0f };
                morph::vec<float, 4> pp = this->projection * point;
                float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.

                // Construct two points for the start and end of the mouse movement
                morph::vec<float, 4> p0 = { p0_coord[0], p0_coord[1], coord_z, 1.0f };
                morph::vec<float, 4> p1 = { p1_coord[0], p1_coord[1], coord_z, 1.0f };
                // Apply the inverse projection to get two points in the world frame of reference:
                morph::vec<float, 4> v0 = this->invproj * p0;
                morph::vec<float, 4> v1 = this->invproj * p1;
                // This computes the difference betwen v0 and v1, the 2 mouse positions in the world
                mouseMoveWorld[0] = (v1[0]/v1[3]) - (v0[0]/v0[3]);
                mouseMoveWorld[1] = (v1[1]/v1[3]) - (v0[1]/v0[3]);
                // Note: mouseMoveWorld[2] is unmodified

                // We "translate the whole scene" - used by 2D projection shaders (ignored by cyl shader)
                this->scenetrans[0] += mouseMoveWorld[0];
                this->scenetrans[1] -= mouseMoveWorld[1];

                // Also translate our cylindrical camera position (used in cyl shader, ignored in proj. shader)
                this->cyl_cam_pos[0] -= mouseMoveWorld[0];
                this->cyl_cam_pos[2] += mouseMoveWorld[1];

                needs_render = true; // updates viewproj; uses this->scenetrans
            }

            return needs_render;
        }

        virtual void mouse_button_callback (int button, int action, int mods = 0)
        {
            // If the scene is locked, then ignore the mouse movements
            if (this->sceneLocked) { return; }

            // Record the position at which the button was pressed
            if (action == keyaction::press) { // Button down
                this->mousePressPosition = this->cursorpos;
                // Save the rotation at the start of the mouse movement
                this->savedRotation = this->rotation;
                // Get the scene's rotation at the start of the mouse movement:
                this->scene.setToIdentity();
                this->scene.rotate (this->savedRotation);
                this->invscene = this->scene.invert();
            }

            if (button == morph::mousebutton::left) { // Primary button means rotate
                this->rotateModMode = (mods & keymod::control) ? true : false;
                this->rotateMode = (action == keyaction::press);
                this->translateMode = false;
            } else if (button == morph::mousebutton::right) { // Secondary button means translate
                this->rotateMode = false;
                this->translateMode = (action == keyaction::press);
            }

            this->mouse_button_callback_extra (button, action, mods);
        }

        virtual bool window_size_callback (int width, int height)
        {
            this->window_w = width;
            this->window_h = height;
            return true; // needs_render
        }

        virtual void window_close_callback()
        {
            if (this->preventWindowCloseWithButton == false) {
                this->signal_to_quit();
            } else {
                std::cerr << "Ignoring user request to exit (Visual::preventWindowCloseWithButton)\n";
            }
        }

        //! When user scrolls, we translate the scene (applies to orthographic/projection) and the
        //! cyl_cam_pos (applies to cylindrical projection).
        virtual bool scroll_callback (double xoffset, double yoffset)
        {
            // yoffset non-zero indicates that the most common scroll wheel is changing. If there's
            // a second scroll wheel, xoffset will be passed non-zero. They'll be 0 or +/- 1.

            if (this->sceneLocked) { return false; }

            if (this->ptype == perspective_type::orthographic) {
                // In orthographic, the wheel should scale ortho_lb and ortho_rt
                morph::vec<float, 2> _lb = this->ortho_lb + (yoffset * this->scenetrans_stepsize);
                morph::vec<float, 2> _rt = this->ortho_rt - (yoffset * this->scenetrans_stepsize);
                if (_lb < 0.0f && _rt > 0.0f) {
                    this->ortho_lb = _lb;
                    this->ortho_rt = _rt;
                }

            } else { // perspective_type::perspective or perspective_type::cylindrical

                // xoffset does what mouse drag left/right in rotateModMode does (L/R scene trans)
                this->scenetrans[0] -= xoffset * this->scenetrans_stepsize;
                this->cyl_cam_pos[0] += xoffset * this->scenetrans_stepsize;

                // yoffset does the 'in-out zooming'
                morph::vec<float, 4> scroll_move_y = { 0.0f, static_cast<float>(yoffset) * this->scenetrans_stepsize, 0.0f, 1.0f };
                this->scenetrans[2] += scroll_move_y[1];
                // Translate scroll_move_y then add it to cyl_cam_pos here
                morph::TransformMatrix<float> sceneview_rotn;
                sceneview_rotn.rotate (this->rotation);
                this->cyl_cam_pos += sceneview_rotn * scroll_move_y;
            }
            return true; // needs_render
        }

        //! Extra key callback handling, making it easy for client programs to implement their own actions
        virtual void key_callback_extra ([[maybe_unused]] int key, [[maybe_unused]] int scancode,
                                         [[maybe_unused]] int action, [[maybe_unused]] int mods) {}

        //! Extra mousebutton callback handling, making it easy for client programs to implement their own actions
        virtual void mouse_button_callback_extra ([[maybe_unused]] int button, [[maybe_unused]] int action,
                                                  [[maybe_unused]] int mods) {}

        //! A callback that client code can set so that it knows when user has signalled to
        //! morph::Visual that it's quit time.
        std::function<void()> external_quit_callback;

    protected:
        //! This internal quit function sets a 'readyToFinish' flag that your code can respond to,
        //! and calls an external callback function that you may have set up.
        void signal_to_quit()
        {
            if (this->user_info_stdout == true) { std::cout << "User requested exit.\n"; }
            // 1. Set our 'readyToFinish' flag to true
            this->readyToFinish = true;
            // 2. Call any external callback that's been set by client code
            if (this->external_quit_callback) { this->external_quit_callback(); }
        }
    };

} // namespace morph
