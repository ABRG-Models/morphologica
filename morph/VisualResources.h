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
#include <morph/gl/util.h>
#include <morph/VisualFace.h>
// FreeType for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

namespace morph {

    // Pointers to morph::Visual are used to index font faces
    template<int, int, bool>
    class Visual;

    //! Singleton resource class for morph::Visual scenes.
    template <int glv1, int glv2, bool gles>
    class VisualResources
    {
    private:
        VisualResources() { this->init(); }
        ~VisualResources()
        {
            // Clean up the faces, which is a map:
            for (auto& f : this->faces) { delete f.second; }
            this->faces.clear();

            // We're done with freetype, so clear those up
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
            glfwSetErrorCallback (morph::VisualResources<glv1,glv2,gles>::errorCallback);

            // See https://www.glfw.org/docs/latest/monitor_guide.html
            GLFWmonitor* primary = glfwGetPrimaryMonitor();
            float xscale, yscale;
            glfwGetMonitorContentScale(primary, &xscale, &yscale);

            if constexpr (gles == true) {
                glfwWindowHint (GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                glfwWindowHint (GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
            }
            glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, glv1);
            glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, glv2);
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
                            morph::Visual<glv1,glv2,gles>*>,
                 morph::gl::VisualFace*> faces;

        //! An error callback function for the GLFW windowing library
        static void errorCallback (int error, const char* description)
        {
            std::cerr << "Error: " << description << " (code "  << error << ")\n";
        }

        //! FreeType library object, public for access by client code?
        std::map<morph::Visual<glv1,glv2,gles>*, FT_Library> freetypes;

    public:
        VisualResources(const VisualResources<glv1,glv2,gles>&) = delete;
        VisualResources& operator=(const VisualResources<glv1,glv2,gles> &) = delete;
        VisualResources(VisualResources<glv1,glv2,gles> &&) = delete;
        VisualResources & operator=(VisualResources<glv1,glv2,gles> &&) = delete;

        //! Initialize a freetype library instance and add to this->freetypes. I wanted
        //! to have only a single freetype library instance, but this didn't work, so I
        //! create one FT_Library for each OpenGL context (i.e. one for each morph::Visual
        //! window). Thus, arguably, the FT_Library should be a member of morph::Visual,
        //! but that's a task for the future, as I coded it this way under the false
        //! assumption that I'd only need one FT_Library.
        void freetype_init (morph::Visual<glv1,glv2,gles>* _vis)
        {
            FT_Library freetype = (FT_Library)0;
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

        //! The instance public function. Uses the very short name 'i' to keep code tidy.
        //! This relies on C++11 magic statics (N2660).
        static auto& i()
        {
            static VisualResources<glv1,glv2,gles> instance;
            return instance;
        }

        //! A function to call to simply make sure the singleton instance exists
        void create() {}

        //! Return a pointer to a VisualFace for the given \a font at the given texture
        //! resolution, \a fontpixels and the given window (i.e. OpenGL context) \a _win.
        morph::gl::VisualFace* getVisualFace (morph::VisualFont font, unsigned int fontpixels,
                                              morph::Visual<glv1,glv2,gles>* _vis)
        {
            morph::gl::VisualFace* rtn = nullptr;
            auto key = std::make_tuple(font, fontpixels, _vis);
            try {
                rtn = this->faces.at (key);
            } catch (const std::out_of_range& e) {
                this->faces[key] = new morph::gl::VisualFace (font, fontpixels, this->freetypes.at(_vis));
                rtn = this->faces.at (key);
            }
            return rtn;
        }
    };

} // namespace morph
