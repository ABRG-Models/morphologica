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
#include <morph/VisualCommon.h>
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
            FT_Done_FreeType (this->freetype);
            delete VisualResources::pInstance;
        }

        void init()
        {
            // Use of gl calls here may make it imperative to set up GL/GLFW here in VisualResources.
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
            morph::gl::Util::checkError (__FILE__, __LINE__);
            if (FT_Init_FreeType (&this->freetype)) {
                std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            }
        }

        //! A pointer returned to the single instance of this class
        static VisualResources* pInstance;

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
    };

    //! Globally initialise instance pointer to NULL
    VisualResources* VisualResources::pInstance = 0;

} // namespace morph
