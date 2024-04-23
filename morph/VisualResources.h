/*!
 * \file
 *
 * Declares a VisualResource class to hold the information about Freetype and other
 * one-per-program resources.
 *
 * \author Seb James
 * \date November 2020
 */

#pragma once

#ifndef OWNED_MODE
# define GLFW_INCLUDE_NONE
# include <GLFW/glfw3.h>
#endif
#include <iostream>
#include <tuple>
#include <set>
#include <stdexcept>
#include <memory>
#include <morph/gl/version.h>
#include <morph/gl/util.h>
#include <morph/VisualFace.h>
// FreeType for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

namespace morph {

    // Pointers to morph::Visual are used to index font faces
    template<int>
    class Visual;

    //! Singleton resource class for morph::Visual scenes.
    template <int glver>
    class VisualResources
    {
    private:
        VisualResources() { this->init(); }
        ~VisualResources()
        {
            // Normally, when each morph::Visual goes out of scope, the faces associated with that
            // Visual get cleaned up (in VisualResources::freetype_deinit). So at this point, faces
            // should be empty, and the following clear() should do nothing.
            this->faces.clear();

            // As with the case for faces, when each morph::Visual goes out of scope, the FreeType
            // instance gets cleaned up. So at this stage freetypes should also be empy and nothing
            // will happen here either.
            for (auto& ft : this->freetypes) { FT_Done_FreeType (ft.second); }

#ifndef OWNED_MODE
            // Shut down GLFW
            glfwTerminate();
#endif
        }

#ifndef OWNED_MODE
        void glfw_init()
        {
            if (!glfwInit()) { std::cerr << "GLFW initialization failed!\n"; }

            // Set up error callback
            glfwSetErrorCallback (morph::VisualResources<glver>::errorCallback);

            // See https://www.glfw.org/docs/latest/monitor_guide.html
            GLFWmonitor* primary = glfwGetPrimaryMonitor();
            float xscale, yscale;
            glfwGetMonitorContentScale(primary, &xscale, &yscale);

            if constexpr (morph::gl::version::gles (glver) == true) {
                glfwWindowHint (GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                glfwWindowHint (GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
            }
            glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, morph::gl::version::major (glver));
            glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, morph::gl::version::minor (glver));
#ifdef __OSX__
            glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
            // Tell glfw that we'd like to do anti-aliasing.
            glfwWindowHint (GLFW_SAMPLES, 4);
        }
#endif
        void init()
        {
#ifndef OWNED_MODE
            // The initial init only does glfw. Have to wait until later for Freetype init
            this->glfw_init();
#endif
        }

        //! The collection of VisualFaces generated for this instance of the
        //! application. Create one VisualFace for each unique combination of VisualFont
        //! and fontpixels (the texture resolution)
        std::map<std::tuple<morph::VisualFont, unsigned int,
                            morph::Visual<glver>*>,
                 std::unique_ptr<morph::gl::VisualFace>> faces;

        //! An error callback function for the GLFW windowing library
        static void errorCallback (int error, const char* description)
        {
            std::cerr << "Error: " << description << " (code "  << error << ")\n";
        }

        //! FreeType library object, public for access by client code?
        std::map<morph::Visual<glver>*, FT_Library> freetypes;

    public:
        VisualResources(const VisualResources<glver>&) = delete;
        VisualResources& operator=(const VisualResources<glver> &) = delete;
        VisualResources(VisualResources<glver> &&) = delete;
        VisualResources & operator=(VisualResources<glver> &&) = delete;

        //! Initialize a freetype library instance and add to this->freetypes. I wanted
        //! to have only a single freetype library instance, but this didn't work, so I
        //! create one FT_Library for each OpenGL context (i.e. one for each morph::Visual
        //! window). Thus, arguably, the FT_Library should be a member of morph::Visual,
        //! but that's a task for the future, as I coded it this way under the false
        //! assumption that I'd only need one FT_Library.
        void freetype_init (morph::Visual<glver>* _vis)
        {
            FT_Library freetype = nullptr;
            try {
                freetype = this->freetypes.at (_vis);
            } catch (const std::out_of_range& e) {
                // Use of gl calls here may make it neat to set up GL/GLFW here in VisualResources.
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
                morph::gl::Util::checkError (__FILE__, __LINE__);
                if (FT_Init_FreeType (&freetype)) {
                    std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
                } else {
                    // Successfully initialized freetype
                    this->freetypes[_vis] = freetype;
                }
            }
        }

        //! When a morph::Visual goes out of scope, its freetype library instance should be
        //! deinitialized.
        void freetype_deinit (morph::Visual<glver>* _vis)
        {
            // First clear the faces associated with Visual<>* _vis
            this->clearVisualFaces (_vis);
            // Second, clean up the FreeType library instance and erase from this->freetypes
            auto freetype = this->freetypes.find (_vis);
            if (freetype != this->freetypes.end()) {
                FT_Done_FreeType (freetype->second);
                this->freetypes.erase (freetype);
            }
        }

        //! The instance public function. Uses the very short name 'i' to keep code tidy.
        //! This relies on C++11 magic statics (N2660).
        static auto& i()
        {
            static VisualResources<glver> instance;
            return instance;
        }

        //! A function to call to simply make sure the singleton instance exists
        void create() {}

        //! Return a pointer to a VisualFace for the given \a font at the given texture
        //! resolution, \a fontpixels and the given window (i.e. OpenGL context) \a _win.
        morph::gl::VisualFace* getVisualFace (morph::VisualFont font, unsigned int fontpixels,
                                              morph::Visual<glver>* _vis)
        {
            morph::gl::VisualFace* rtn = nullptr;
            auto key = std::make_tuple(font, fontpixels, _vis);
            try {
                rtn = this->faces.at(key).get();
            } catch (const std::out_of_range& e) {
                this->faces[key] = std::make_unique<morph::gl::VisualFace> (font, fontpixels, this->freetypes.at(_vis));
                rtn = this->faces.at(key).get();
            }
            return rtn;
        }

        //! Loop through this->faces clearing out those associated with the given morph::Visual
        void clearVisualFaces (morph::Visual<glver>* _vis)
        {
            auto f = this->faces.begin();
            while (f != this->faces.end()) {
                // f->first is a key. If its third, Visual<>* element == _vis, then delete and erase
                if (std::get<morph::Visual<glver>*>(f->first) == _vis) {
                    f = this->faces.erase (f);
                } else { f++; }
            }
        }
    };

} // namespace morph
