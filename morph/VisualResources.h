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

#include <iostream>
#include <utility>
#include <stdexcept>
#include <morph/VisualCommon.h>
#include <morph/VisualFace.h>
// FreeType for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

namespace morph {

    //! Singleton resource class for morph::Visual scenes.
    class VisualResources
    {
    private:
        VisualResources() {}
        ~VisualResources()
        {
            // Clean up the faces
            for (auto f : this->faces) {
                delete &f;
            }
            // We're done with freetype
            FT_Done_FreeType (this->freetype);
            // Delete self
            delete VisualResources::pInstance;
        }

        void init()
        {
            // Use of gl calls here may make it neat to set up GL/GLFW here in VisualResources.
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
            morph::gl::Util::checkError (__FILE__, __LINE__);
            if (FT_Init_FreeType (&this->freetype)) {
                std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            }
        }

        //! A pointer returned to the single instance of this class
        static VisualResources* pInstance;

        //! The collection of VisualFaces generated for this instance of the
        //! application. Create one VisualFace for each unique combination of VisualFont
        //! and fontpixels (the texture resolution)
        std::map<std::pair<morph::VisualFont, unsigned int>, morph::gl::VisualFace*> faces;

    public:
        //! FreeType library object, public for access by client code
        FT_Library freetype;

        //! The instance public function. Uses the very short name 'i' to keep code tidy.
        static VisualResources* i()
        {
            if (VisualResources::pInstance == 0) {
                VisualResources::pInstance = new VisualResources;
                VisualResources::i()->init();
            }
            return VisualResources::pInstance;
        }

        //! Return a pointer to a VisualFace for the given \a font at the given texture resolution, \a fontpixels.
        morph::gl::VisualFace* getVisualFace (morph::VisualFont font, unsigned int fontpixels)
        {
            morph::gl::VisualFace* rtn = (morph::gl::VisualFace*)0;
            std::pair<morph::VisualFont, unsigned int> key = std::make_pair(font, fontpixels);
            try {
                rtn = this->faces.at (key);
            } catch (const std::out_of_range& e) {
                this->faces[key] = new morph::gl::VisualFace (font, fontpixels, this->freetype);
                rtn = this->faces.at (key);
            }
            return rtn;
        }
    };

    //! Globally initialise instance pointer to NULL
    VisualResources* VisualResources::pInstance = 0;

} // namespace morph
