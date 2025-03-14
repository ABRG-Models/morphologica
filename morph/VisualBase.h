/*!
 * \file
 *
 * Awesome graphics code for high performance graphing and visualisation. This is the abstract base
 * class for the Visual scene classes (it contains common functionality, but no GL)
 *
 * \author Seb James
 * \date March 2025
 */
#pragma once

#include <morph/gl/version.h>
#include <morph/VisualModel.h>
#include <morph/TextFeatures.h>
#include <morph/TextGeometry.h>
#include <morph/VisualCommon.h>
#include <morph/gl/shaders.h>
#include <morph/keys.h>
#include <morph/version.h>

#include <nlohmann/json.hpp>
#include <morph/CoordArrows.h>
#include <morph/quaternion.h>
#include <morph/mat44.h>
#include <morph/vec.h>
#include <morph/tools.h>

#include <string>
#include <array>
#include <vector>
#include <memory>
#include <functional>
#include <cstddef>

#include <morph/VisualDefaultShaders.h>

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

#ifdef _morph_OSX_
    // https://stackoverflow.com/questions/35715579/opengl-created-window-size-twice-as-large
    static constexpr double retinaScale = 2; // deals with quadrant issue on osx
#else
    static constexpr double retinaScale = 1; // Qt has devicePixelRatio() to get retinaScale.
#endif

    /*!
     * VisualBase, the morph::Visual 'scene' base class
     *
     * A base class for visualising computational models on an OpenGL screen.
     *
     * This contains code that is not OpenGL dependent. OpenGL dependent code is in
     * VisualOwnable or VisualOwnableMX.
     *
     * For morphologica program using GLFW windows, Inheritance chain will either be:
     *
     *   VisualBase -> VisualOwnable -> VisualNoMX            for single context GL, global fn aliases
     *
     *   VisualBase -> VisualOwnableMX -> VisualMX -> Visual  for multi context, GL fn pointers (GLAD only)
     *
     * morphologica based widgets, such as the Qt compatible morph::qt::viswidget have this:
     *
     *   VisualBase -> VisualOwnable -> viswidget             for single context GL, global fn aliases
     *
     *   VisualBase -> VisualOwnableMX -> viswidget_mx        for single context GL, global fn aliases
     *
     * \tparam glver The OpenGL version, encoded as a single int (see morph::gl::version)
     */
    template <int glver = morph::gl::version_4_1>
    class VisualBase
    {
    public:
        /*!
         * Default constructor is used when incorporating Visual inside another object
         * such as a QWidget.  We have to wait on calling init functions until an OpenGL
         * environment is guaranteed to exist.
         */
        VisualBase() { }

        /*!
         * Construct a new visualiser. The rule is 1 window to one Visual object. So, this creates a
         * new window and a new OpenGL context.
         */
        VisualBase (const int _width, const int _height, const std::string& _title, const bool _version_stdout = true)
            : window_w(_width)
            , window_h(_height)
            , title(_title)
            , version_stdout(_version_stdout)
        {
            this->init_gl(); // abstract
        }

        //! Deconstruct gl memory/context
        virtual void deconstructCommon() = 0;

        // We do not manage OpenGL context, but it is simpler to have a no-op set/releaseContext for some of the GL setup functions
        virtual void setContext() {}       // no op here
        virtual void releaseContext() {}   // no op here
        virtual void setSwapInterval() {}  // no op here
        virtual void swapBuffers() {}      // no op here

        // A callback friendly wrapper for setContext
        static void set_context (morph::VisualBase<glver>* _v) { _v->setContext(); };
        // A callback friendly wrapper for releaseContext
        static void release_context (morph::VisualBase<glver>* _v) { _v->releaseContext(); };

        // Public init that is given a context (window or widget) and then sets up the
        // VisualResource, shaders and so on.
        void init (morph::win_t* ctx)
        {
            this->window = ctx;
            this->init_resources();
            this->init_gl();
        }

    protected:
        virtual void freetype_init() = 0; // does this need to be here?

    public:
        // Do one-time init of resources (such as freetypes, windowing system etc)
        virtual void init_resources() = 0;

        //! Take a screenshot of the window. Return vec containing width * height or {-1, -1} on
        //! failure. Set transparent_bg to get a transparent background.
        virtual morph::vec<int, 2> saveImage (const std::string& img_filename, const bool transparent_bg = false) = 0;

        /*!
         * Set up the passed-in VisualModel (or indeed, VisualTextModel) with functions that need access to Visual attributes.
         */
        template <typename T>
        void bindmodel (std::unique_ptr<T>& model)
        {
            model->set_parent (this);
            model->get_shaderprogs = &morph::VisualBase<glver>::get_shaderprogs;
            model->get_gprog = &morph::VisualBase<glver>::get_gprog;
            model->get_tprog = &morph::VisualBase<glver>::get_tprog;
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
         * Test the pointer vmp. Return vmp if it is owned by a unique_ptr in
         * Visual::vm. If it is not present, return nullptr.
         */
        const morph::VisualModel<glver>* validVisualModel (const morph::VisualModel<glver>* vmp) const
        {
            const morph::VisualModel<glver>* rtn = nullptr;
            for (unsigned int modelId = 0; modelId < this->vm.size(); ++modelId) {
                if (this->vm[modelId].get() == vmp) {
                    rtn = vmp;
                    break;
                }
            }
            return rtn;
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

        //! Remove the VisualModel whose pointer matches the VisualModel* vmp
        void removeVisualModel (morph::VisualModel<glver>* vmp)
        {
            unsigned int modelId = 0;
            bool found_model = false;
            for (modelId = 0; modelId < this->vm.size(); ++modelId) {
                if (this->vm[modelId].get() == vmp) {
                    found_model = true;
                    break;
                }
            }
            if (found_model == true) { this->vm.erase (this->vm.begin() + modelId); }
        }

        void set_cursorpos (double _x, double _y) { this->cursorpos = {static_cast<float>(_x), static_cast<float>(_y)}; }

        //! A callback function
        static void callback_render (morph::VisualBase<glver>* _v) { _v->render(); };

        //! Render the scene
        virtual void render() noexcept = 0;

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
        static morph::visgl::visual_shaderprogs get_shaderprogs (morph::VisualBase<glver>* _v) { return _v->shaders; };
        static GLuint get_gprog (morph::VisualBase<glver>* _v) { return _v->shaders.gprog; };
        static GLuint get_tprog (morph::VisualBase<glver>* _v) { return _v->shaders.tprog; };

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

        //! Set to true when the program should end
        bool readyToFinish = false;

        //! paused can be set true so that pauseOpen() can be used to display the window mid-simulation
        bool paused = false;

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

        void setSceneRotation (const morph::quaternion<float>& _rotn)
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

        //! Set up a perspective projection based on window width and height. Not public.
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

        /*!
         * Set an orthographic projection. This is not a public function. To choose orthographic
         * projection for your Visual, write something like:
         *
         * \code
         *   morph::Visual<> v(width, height, title);
         *   v.ptype = morph::perspective_type::orthographic;
         * \endcode
         */
        void setOrthographic()
        {
            this->projection.setToIdentity();
            this->projection.orthographic (this->ortho_lb, this->ortho_rt, this->zNear, this->zFar);
            this->invproj = this->projection.invert();
        }

        //! A vector of pointers to all the morph::VisualModels (HexGridVisual,
        //! ScatterVisual, etc) which are going to be rendered in the scene.
        std::vector<std::unique_ptr<morph::VisualModel<glver>>> vm;

        // Initialize OpenGL shaders, set some flags (Alpha, Anti-aliasing), read in any external
        // state from json, and set up the coordinate arrows and any VisualTextModels that will be
        // required to render the Visual.
        virtual void init_gl() = 0;

        //! The window (and OpenGL context) for this Visual
        morph::win_t* window = nullptr;

        //! Current window width
        int window_w = 640;
        //! Current window height
        int window_h = 480;

        //! The title for the Visual. Used in window title and if saving out 3D model or png image.
        std::string title = "morph::Visual";

        //! If true, output some version information (morphologica version, OpenGL version) to
        //! stdout. Protected as this has no effect after init()
        bool version_stdout = true;

        //! The user's 'selected visual model'. For model specific changes to alpha and possibly colour
        unsigned int selectedVisualModel = 0u;

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
        morph::quaternion<float> rotation;

        //! The default rotation of the scene
        morph::quaternion<float> rotation_default;

        //! A rotation that is saved between mouse button callbacks
        morph::quaternion<float> savedRotation;

        //! The projection matrix is a member of this class
        morph::mat44<float> projection;

        //! The inverse of the projection
        morph::mat44<float> invproj;

        //! A scene transformation
        morph::mat44<float> scene;
        //! Scene transformation inverse
        morph::mat44<float> invscene;

    public:

        /*
         * Generic callback handlers
         */

        using keyaction = morph::keyaction;
        using keymod = morph::keymod;
        using key = morph::key;
        // The key_callback handler uses GLFW codes, but they're in a morph header (keys.h)
        template<bool owned = true>
        bool key_callback (int _key, int scancode, int action, int mods) // can't be virtual.
        {
            bool needs_render = false;

            if constexpr (owned == true) { // If Visual is 'owned' then the owning system deals with program exit
                // Exit action
                if (_key == key::q && (mods & keymod::control) && action == keyaction::press) {
                    this->signal_to_quit();
                }
            }

            if (!this->sceneLocked && _key == key::c  && (mods & keymod::control) && action == keyaction::press) {
                this->showCoordArrows = !this->showCoordArrows;
                needs_render = true;
            }

            if (_key == key::h && (mods & keymod::control) && action == keyaction::press) {
                // Help to stdout:
                std::cout << "Ctrl-h: Output this help to stdout\n"
                          << "Mouse-primary: rotate mode (use Ctrl to change axis)\n"
                          << "Mouse-secondary: translate mode\n";
                if constexpr (owned == true) { // If Visual is 'owned' then the owning system deals with program exit
                    std::cout << "Ctrl-q: Request exit\n";
                }
                std::cout << "Ctrl-v: Un-pause\n"
                          << "Ctrl-l: Toggle the scene lock\n"
                          << "Ctrl-c: Toggle coordinate arrows\n"
                          << "Ctrl-s: Take a snapshot\n"
                          << "Ctrl-m: Save 3D models in .gltf format (open in e.g. blender)\n"
                          << "Ctrl-a: Reset default view\n"
                          << "Ctrl-o: Reduce field of view\n"
                          << "Ctrl-p: Increase field of view\n"
                          << "Ctrl-y: Cycle perspective\n"
                          << "Ctrl-z: Show the current scenetrans/rotation and save to /tmp/Visual.json\n"
                          << "Ctrl-u: Reduce zNear cutoff plane\n"
                          << "Ctrl-i: Increase zNear cutoff plane\n"
                          << "F1-F10: Select model index (with shift: toggle hide)\n"
                          << "Shift-Left: Decrease opacity of selected model\n"
                          << "Shift-Right: Increase opacity of selected model\n"
                          << "Shift-Up: Double cyl proj radius\n"
                          << "Shift-Down: Halve cyl proj radius\n"
                          << "Ctrl-Up: Double cyl proj height\n"
                          << "Ctrl-Down: Halve cyl proj height\n"
                          << std::flush;
            }

            if (_key == key::l && (mods & keymod::control) && action == keyaction::press) {
                this->sceneLocked = this->sceneLocked ? false : true;
                std::cout << "Scene is now " << (this->sceneLocked ? "" : "un-") << "locked\n";
            }

            if (_key == key::v && (mods & keymod::control) && action == keyaction::press) {
                if (this->paused == true) {
                    this->paused = false;
                    std::cout << "Scene un-paused\n";
                } // else no-op
            }

            if (_key == key::s && (mods & keymod::control) && action == keyaction::press) {
                std::string fname (this->title);
                morph::tools::stripFileSuffix (fname);
                fname += ".png";
                // Make fname 'filename safe'
                morph::tools::conditionAsFilename (fname);
                this->saveImage (fname);
                std::cout << "Saved image to '" << fname << "'\n";
            }

            // Save gltf 3D file
            if (_key == key::m && (mods & keymod::control) && action == keyaction::press) {
                std::string gltffile = this->title;
                morph::tools::stripFileSuffix (gltffile);
                gltffile += ".gltf";
                morph::tools::conditionAsFilename (gltffile);
                this->savegltf (gltffile);
                std::cout << "Saved 3D file '" << gltffile << "'\n";
            }

            if (_key == key::z && (mods & keymod::control) && action == keyaction::press) {
                std::cout << "Scenetrans setup code:\n    v.setSceneTrans (morph::vec<float,3>{ float{"
                          << this->scenetrans.x() << "}, float{"
                          << this->scenetrans.y() << "}, float{"
                          << this->scenetrans.z()
                          << "} });"
                          <<  "\n    v.setSceneRotation (morph::quaternion<float>{ float{"
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

        //! Rotate the scene about axis by angle (angle in radians)
        void rotate_scene (const morph::vec<float>& axis, const float angle)
        {
            this->rotationAxis = axis;
            morph::quaternion<float> rotnQuat (this->rotationAxis, -angle);
            this->rotation.postmultiply (rotnQuat);
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
                morph::quaternion<float> rotnQuat (this->rotationAxis, -rotamount * morph::mathconst<float>::deg2rad);
                this->rotation.postmultiply (rotnQuat); // combines rotations
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
                morph::mat44<float> sceneview_rotn;
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

        //! Unpause, allowing pauseOpen() to return
        void unpause() { this->paused = false; }
    };

} // namespace morph
