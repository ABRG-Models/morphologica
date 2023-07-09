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

#ifdef USING_QT
# include <morph/qt/qwindow.h>
#else
# include <GLFW/glfw3.h>
#endif
#include <iostream>
#include <utility>
#include <tuple>
#include <set>
#include <stdexcept>
#include <morph/VisualCommon.h>
#include <morph/VisualFace.h>
// FreeType for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

namespace morph {

    // Pointers to morph::Visual are used to index font faces
    class Visual;

    //! Singleton resource class for morph::Visual scenes.
    class VisualResources
    {
    private:
        VisualResources() {}
        //! The deconstructor is never called for a singleton.
        ~VisualResources() {}

#ifdef USING_QT
        void qt_init()
        {
            // Any per-program qt init could go here.
        }
#else
        void glfw_init()
        {
            if (!glfwInit()) { std::cerr << "GLFW initialization failed!\n"; }

            // Set up error callback
            glfwSetErrorCallback (morph::VisualResources::errorCallback);

            // See https://www.glfw.org/docs/latest/monitor_guide.html
            GLFWmonitor* primary = glfwGetPrimaryMonitor();
            float xscale, yscale;
            glfwGetMonitorContentScale(primary, &xscale, &yscale);

            glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 1);
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
#ifdef USING_QT
            // One of these will be a no-op
            this->qt_init();
#else
            // The initial init only does glfw. Have to wait until later for Freetype init
            this->glfw_init();
#endif
        }

        //! A pointer returned to the single instance of this class
        static VisualResources* pInstance;

        //! How many Visuals are we managing?
        static int numVisuals;

        //! The collection of VisualFaces generated for this instance of the
        //! application. Create one VisualFace for each unique combination of VisualFont
        //! and fontpixels (the texture resolution)
        std::map<std::tuple<morph::VisualFont, unsigned int, morph::Visual*>, morph::gl::VisualFace*> faces;

        //! An error callback function for the GLFW windowing library
        static void errorCallback (int error, const char* description)
        {
            std::cerr << "Error: " << description << " (code "  << error << ")\n";
        }

        //! FreeType library object, public for access by client code?
        std::map<morph::Visual*, FT_Library> freetypes;

    public:

        //! Initialize a freetype library instance and add to this->freetypes. I wanted
        //! to have only a single freetype library instance, but this didn't work, so I
        //! create one FT_Library for each OpenGL context (i.e. one for each morph::Visual
        //! window). Thus, arguably, the FT_Library should be a member of morph::Visual,
        //! but that's a task for the future, as I coded it this way under the false
        //! assumption that I'd only need one FT_Library.
        void freetype_init (morph::Visual* _vis)
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
                    //std::cout << "Initialized freetype which has value: "
                    //          << (unsigned long long int)freetype << std::endl;
                    this->freetypes[_vis] = freetype;
                }
            }
        }

        //! The instance public function. Uses the very short name 'i' to keep code tidy.
        static VisualResources* i()
        {
            if (VisualResources::pInstance == nullptr) {
                VisualResources::pInstance = new VisualResources;
                VisualResources::i()->init();
            }
            return VisualResources::pInstance;
        }

        //! register a morph::Visual as being handled by this VisualResources singleton instance
        static void register_visual() { VisualResources::numVisuals++; }

        //! De-register a morph::Visual. When there are no morph::Visuals left,
        //! deconstruct this VisualResources and delete self.
        static void deregister()
        {
            VisualResources::numVisuals--;
            if (VisualResources::numVisuals <= 0) {
                VisualResources::pInstance->deconstruct();
                // Delete self
                delete VisualResources::pInstance;
                VisualResources::pInstance = nullptr;
            }
        }

        //! Deallocate memory for faces, FT_Librarys and the GLFW system.
        void deconstruct()
        {
            // Clean up the faces, which is a map:
            // std::map<
            //          std::pair<morph::VisualFont, unsigned int>,
            //          morph::gl::VisualFace*
            //         > faces;
            // (want to delete the morph::gl::VisualFace)
            for (auto& f : this->faces) { delete f.second; }
            this->faces.clear();

            // We're done with freetype, so clear those up
            for (auto& ft : this->freetypes) { FT_Done_FreeType (ft.second); }


#ifdef USING_QT
                // Qt cleanup?
#else
                // Shut down GLFW
                glfwTerminate();
#endif

            // Note: static deregister() will delete self
        }

        //! Return a pointer to a VisualFace for the given \a font at the given texture
        //! resolution, \a fontpixels and the given window (i.e. OpenGL context) \a _win.
        morph::gl::VisualFace* getVisualFace (morph::VisualFont font, unsigned int fontpixels, morph::Visual* _vis)
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

    //! Globally initialise instance pointer to nullptr
    VisualResources* VisualResources::pInstance = nullptr;
    //! The number of morph::Visuals to which this singleston class provides resources.
    int VisualResources::numVisuals = 0;
} // namespace morph
