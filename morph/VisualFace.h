/*!
 * \file
 *
 * Declares a VisualFace class to hold the information about a (Freetype-managed) font
 * face and the GL-textures that will reproduce it.
 *
 * \author Seb James
 * \date November 2020
 */

#pragma once

#include <map>
#include <iostream>
#include <utility>

#include <morph/VisualCommon.h>
#include <morph/VisualResources.h>

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include "GL3/gl3.h"
#endif

// FreeType for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

// Here, add code to incorporate Vera.ttf and friends into the binary

namespace morph {

    //! The fonts supported (i.e. compiled in) to morph::Visual
    enum class VisualFont {
        Vera,               // fonts/ttf-bitstream-vera/Vera.ttf
        VeraItalic,         // fonts/ttf-bitstream-vera/VeraIt.ttf
        VeraBold,           // fonts/ttf-bitstream-vera/VeraBd.ttf
        VeraBoldItalic,     // fonts/ttf-bitstream-vera/VeraBI.ttf
        VeraMono,           // fonts/ttf-bitstream-vera/VeraMono.ttf
        VeraMonoItalic,     // fonts/ttf-bitstream-vera/VeraMoIt.ttf
        VeraMonoBold,       // fonts/ttf-bitstream-vera/VeraMoBD.ttf
        VeraMonoBoldItalic, // fonts/ttf-bitstream-vera/VeraMoBI.ttf
        VeraSerif,          // fonts/ttf-bitstream-vera/VeraSe.ttf
        VeraSerifBold       // fonts/ttf-bitstream-vera/VeraSeBd.ttf
    };

    namespace gl {

        class VisualFace
        {
        public:
            /*!
             * Construct with a morph::VisualFont \a _font, which specifies a supported
             * font (one which we can legally include in the source code without,
             * e.g. Bitstream Vera) and \a fontpixels, which is the texture size,
             * e.g. 192. This is the width, in pixels, of the texture that would be
             * applied to the letter 'm'. A larger value is required for fonts that will
             * take up a large part of the screen, but will be detrimental to the
             * appearance of a font which is rendered 'small on the screen'.
             *
             * VisualResources could hold a map of VisualFace instances, to avoid many
             * copies of font textures for separate VisualTextModel instances which have
             * the same pixel size specified for them.
             */
            VisualFace (const morph::VisualFont _font, unsigned int fontpixels)
            {
                std::string fontpath = "";
                switch (_font) {
                case VisualFont::Vera:
                {
                    // Use the vera compiled in font, save it to /tmp/Vera.ttf, and then use that path
                    fontpath = "fonts/ttf-bitstream-vera/Vera.ttf";
                    break;
                }
                default:
                {
                    std::cout << "ERROR::Unsupported morph font\n";
                    break;
                }
                }

                // Keep the face as a morph::Visual owned resource, shared by VisTextModels?
                if (FT_New_Face (VisualResources::i()->freetype, fontpath.c_str(), 0, &this->face)) {
                    std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
                }

                FT_Set_Pixel_Sizes (this->face, 0, fontpixels);

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
                    morph::gl::CharInfo glchar = {
                        texture,
                        {static_cast<int>(this->face->glyph->bitmap.width), static_cast<int>(this->face->glyph->bitmap.rows)}, // size
                        {this->face->glyph->bitmap_left, this->face->glyph->bitmap_top}, // bearing
                        static_cast<unsigned int>(this->face->glyph->advance.x)          // advance
                    };
#if 1
                    std::cout << "Inserting character into this->glchars with info: ID:" << glchar.textureID
                              << ", Size:" << glchar.size << ", Bearing:" << glchar.bearing
                              << ", Advance:" << glchar.advance << std::endl;
#endif
                    this->glchars.insert (std::pair<char, morph::gl::CharInfo>(c, glchar));
                }
                glBindTexture(GL_TEXTURE_2D, 0);
                // At this point could FT_Done_Face() etc, I think. as we no longer do anything Freetypey with it.
                FT_Done_Face (this->face);
            }

            ~VisualFace() { /* GL deconstruction? */ }

            //! The FT_Face that we're managing
            FT_Face face;

            //! The OpenGL character info stuff
            std::map<char, morph::gl::CharInfo> glchars;
        };
    } // namespace gl
} // namespace morph
