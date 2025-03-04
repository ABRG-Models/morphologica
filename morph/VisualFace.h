/*!
 * \file
 *
 * Declares a VisualFace class to hold the information about a (Freetype-managed) font
 * face and the GL-textures that will reproduce it.
 *
 * This class is derived from VisualFaceBase and adds globally aliased GL function calls.
 *
 * \author Seb James
 * \date November 2020
 */

#pragma once

#include <morph/VisualFaceBase.h>

#if defined __gl3_h_ || defined __gl_h_
// GL headers have been externally included
#else
# error "GL headers should have been included already"
#endif

namespace morph {

    namespace visgl {

        struct VisualFace : public morph::visgl::VisualFaceBase
        {
            /*!
             * Construct with a morph::VisualFont \a _font, which specifies a supported
             * font (one which we can legally include in the source code without paying any licence fees,
             * e.g. Bitstream Vera) and \a fontpixels, which is the texture size,
             * e.g. 192. This is the width, in pixels, of the texture that would be
             * applied to the letter 'm'. A larger value is required for fonts that will
             * take up a large part of the screen, but will be detrimental to the
             * appearance of a font which is rendered 'small on the screen'.
             *
             * VisualResources holds a map of VisualFace instances, to avoid many copies
             * of font textures for separate VisualTextModel instances which might have
             * the same pixel size.
             */
            VisualFace (const morph::VisualFont _font, unsigned int fontpixels, FT_Library& ft_freetype)
            {
                this->init_common (_font, fontpixels, ft_freetype);

                // How far to loop. In principle, up to 21 bits worth - that's 2097151 possible characters!
                for (char32_t c = 0; c < 2097151; c++) {
                    // Check glyph index first, if it's 0 it's a blank so skip.
                    if (FT_Get_Char_Index (this->face, c) == 0) { continue; }

                    // load character glyph
                    if (FT_Load_Char (this->face, c, FT_LOAD_RENDER)) {
                        std::cout << "ERROR::FREETYPE: Failed to load Glyph for Unicode 0x"
                                  << std::hex << static_cast<unsigned int>(c) << std::dec << std::endl;
                        continue;
                    }

                    // generate texture
                    unsigned int texture = 0;

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
                    morph::visgl::CharInfo glchar = {
                        texture,
                        {static_cast<int>(this->face->glyph->bitmap.width), static_cast<int>(this->face->glyph->bitmap.rows)}, // size
                        {this->face->glyph->bitmap_left, this->face->glyph->bitmap_top}, // bearing
                        static_cast<unsigned int>(this->face->glyph->advance.x)          // advance
                    };

                    if constexpr (debug_visualface == true) {
                        std::cout << "Inserting character into this->glchars with info: ID:" << glchar.textureID
                                  << ", Size:" << glchar.size << ", Bearing:" << glchar.bearing
                                  << ", Advance:" << glchar.advance << std::endl;
                    }
                    this->glchars.insert (std::pair<char32_t, morph::visgl::CharInfo>(c, glchar));
                }

                glBindTexture(GL_TEXTURE_2D, 0);

                // At this point could FT_Done_Face() etc, I think. as we no longer do anything Freetypey with it.
                FT_Done_Face (this->face);
            }

            ~VisualFace() {}
        };
    } // namespace gl
} // namespace morph
