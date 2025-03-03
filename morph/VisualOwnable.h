/*!
 * \file
 *
 * Awesome graphics code for high performance graphing and visualisation. This is the
 * base class that sets up GL, leaving choice of window system (GLFW3/Qt/wx/etc) to a
 * derived class such as morph::Visual or morph::qt::viswidget.
 *
 * Normally, a morph::Visual is the *owner* of a GLFW window in which it does its
 * rendering.
 *
 * This is a base class that is ownable, and can be used in other window drawing system
 * such as Qt and wx.
 *
 * Created by Seb James on 2025/03/01, from morph::Visual.h
 *
 * \author Seb James
 * \date March 2025
 */
#pragma once

#if defined __gl3_h_ || defined __gl_h_ // could get a fuller list from glfw.h
// GL headers appear to have been externally included.
#else
// Include GLAD header
# define GLAD_GL_IMPLEMENTATION
#  include <morph/glad/gl.h>
#endif // GL headers

#include <morph/VisualResources.h>
#include <morph/VisualTextModel.h>
#include <morph/VisualBase.h>

namespace morph {

    /*!
     * VisualOwnable - adds GL calls to the 'scene' base class, VisualBase
     *
     * A class for visualising computational models on an OpenGL screen.
     *
     * Each VisualOwnable provides a "scene" containing a number of objects. One object
     * might be the visualisation of some data expressed over a HexGrid. Another could
     * be a GraphVisual object. The class can pass through mouse events to allow the
     * user to rotate and translate the scene, as well as use keys to generate
     * particular effects/views (though particular implementations will live in derived
     * classes).
     *
     * \tparam glver The OpenGL version, encoded as a single int (see morph::gl::version)
     */
    template <int glver = morph::gl::version_4_1>
    class VisualOwnable : public morph::VisualBase<glver>
    {
    public:
        /*!
         * Default constructor is used when incorporating Visual inside another object
         * such as a QWidget.  We have to wait on calling init functions until an OpenGL
         * environment is guaranteed to exist.
         */
        VisualOwnable() { }

        /*!
         * Construct a new visualiser. The rule is 1 window to one Visual object. So, this creates a
         * new window and a new OpenGL context.
         */
        VisualOwnable (const int _width, const int _height, const std::string& _title, const bool _version_stdout = true)
        {
            this->window_w = _width;
            this->window_h = _height;
            this->title = _title;
            this->version_stdout = _version_stdout;

            this->init_gl();
        }

        //! Deconstruct gl memory/context
        void deconstructCommon()
        {
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

    protected:
        void freetype_init() final
        {
            // Now make sure that Freetype is set up (we assume that caller code has set the correct OpenGL context)
            morph::VisualResources<glver>::i().freetype_init (this);
        }

    public:
        // Do one-time init of the Visual's resources. This gets/creates the VisualResources,
        // registers this visual with resources, calls init_window for any glfw stuff that needs to
        // happen, and lastly initializes the freetype code.
        void init_resources()
        {
            // VisualResources provides font management and GLFW management. Ensure it exists in memory.
            morph::VisualResources<glver>::i().create();
            this->freetype_init();
        }

        //! Stores the OpenGL function context version that was loaded
        int glfn_version = 0;

        //! Take a screenshot of the window. Return vec containing width * height or {-1, -1} on
        //! failure. Set transparent_bg to get a transparent background.
        morph::vec<int, 2> saveImage (const std::string& img_filename, const bool transparent_bg = false)
        {
            this->setContext();

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
                if (transparent_bg) {
                    for (int j = 0; j < 4 * dims[0]; ++j) {
                        rbits[rev_line + j] = bits[for_line + j];
                    }
                } else {
                    for (int j = 0; j < 4 * dims[0]; ++j) {
                        rbits[rev_line + j] = (j % 4 == 3) ? 255 : bits[for_line + j];
                    }
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

        //! Render the scene
        void render() noexcept final
        {
            this->setContext();

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
            glViewport (0, 0, this->window_w * morph::retinaScale, this->window_h * morph::retinaScale);

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
                // unknown projection
                return;
            }

            // Calculate model view transformation - transforming from "model space" to "worldspace".
            morph::mat44<float> sceneview;
            if (this->ptype == perspective_type::orthographic || this->ptype == perspective_type::perspective) {
                // This line translates from model space to world space. Avoid in cyl?
                sceneview.translate (this->scenetrans); // send backwards into distance
            }
            // And this rotation completes the transition from model to world
            sceneview.rotate (this->rotation);

            // Clear color buffer and **also depth buffer**
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Set the background colour:
            glClearBufferfv (GL_COLOR, 0, this->bgcolour.data());

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
                this->coordArrows->setColourForBackground (this->bgcolour); // releases context...
                this->setContext(); // ...so re-acquire if we're managing it

                if (this->coordArrowsInScene == true) {
                    this->coordArrows->setSceneMatrix (sceneview);
                } else {
                    this->positionCoordArrows();
                }
                this->coordArrows->render();
            }

            morph::mat44<float> scenetransonly;
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

            this->swapBuffers();
        }

    public:
#ifdef GLAD_GL // Only define if GL was included with GLAD
        void init_glad (GLADloadfunc procaddressfn) // need basic version of this in case client code does not use glad
        {
            this->glfn_version = gladLoadGL (procaddressfn);
            if (this->glfn_version == 0) {
                throw std::runtime_error ("Failed to initialize GLAD GL context");
            }
        }
#endif

        //! Add a label _text to the scene at position _toffset. Font features are
        //! defined by the tfeatures. Return geometry of the text.
        morph::TextGeometry addLabel (const std::string& _text,
                                      const morph::vec<float, 3>& _toffset,
                                      const morph::TextFeatures& tfeatures = morph::TextFeatures(0.01f))
        {
            this->setContext();
            if (this->shaders.tprog == 0) { throw std::runtime_error ("No text shader prog."); }
            auto tmup = std::make_unique<morph::VisualTextModel<glver>> (tfeatures);
            this->bindmodel (tmup);
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
            this->releaseContext();
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
            this->setContext();
            if (this->shaders.tprog == 0) { throw std::runtime_error ("No text shader prog."); }
            auto tmup = std::make_unique<morph::VisualTextModel<glver>> (tfeatures);
            this->bindmodel (tmup);
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
            this->releaseContext();
            return tm->getTextGeometry();
        }

    protected:
        // Initialize OpenGL shaders, set some flags (Alpha, Anti-aliasing), read in any external
        // state from json, and set up the coordinate arrows and any VisualTextModels that will be
        // required to render the Visual.
        void init_gl()
        {
            this->setContext(); // if managing context

            if (this->version_stdout == true) {
                unsigned char* glv = (unsigned char*)glGetString(GL_VERSION);
                std::cout << "This is version " << morph::version_string()
                          << " of morph::Visual<glver=" << morph::gl::version::vstring (glver)
                          << "> running on OpenGL Version " << glv << std::endl;
            }

            this->setSwapInterval();

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
            this->coordArrows->init (this->coordArrowsLength, this->coordArrowsThickness, this->coordArrowsEm); // sets up text
            this->coordArrows->finalize(); // VisualModel::finalize releases context (normally this is the right thing)...
            this->setContext();            // ...but we've got more work to do, so re-acquire context (if we're managing it)

            morph::gl::Util::checkError (__FILE__, __LINE__);

            // Set up the title, which may or may not be rendered
            morph::TextFeatures title_tf(0.035f, 64);
            this->textModel = std::make_unique<morph::VisualTextModel<glver>> (title_tf);
            this->bindmodel (this->textModel);
            this->textModel->setSceneTranslation ({0.0f, 0.0f, 0.0f});
            this->textModel->setupText (this->title);

            this->releaseContext();
        }

        //! A VisualTextModel for a title text.
        std::unique_ptr<morph::VisualTextModel<glver>> textModel = nullptr;
        //! Text models for labels
        std::vector<std::unique_ptr<morph::VisualTextModel<glver>>> texts;
    };

} // namespace morph
