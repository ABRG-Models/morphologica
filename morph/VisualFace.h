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
#include <fstream>

#include <morph/tools.h>
#include <morph/VisualCommon.h>

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif

// FreeType for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

/*
 * The following inline assembly incorporates Vera.ttf and friends *into the binary*. We
 * have different code for Linux and Mac. Both tested only on Intel CPUs.
 */

#ifdef __GLN__

asm("\n.pushsection vera_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/Vera.ttf\"\n.popsection\n");
asm("\n.pushsection verait_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraIt.ttf\"\n.popsection\n");
asm("\n.pushsection verabd_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraBd.ttf\"\n.popsection\n");
asm("\n.pushsection verabi_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraBI.ttf\"\n.popsection\n");
asm("\n.pushsection veramono_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraMono.ttf\"\n.popsection\n");
asm("\n.pushsection veramoit_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraMoIt.ttf\"\n.popsection\n");
asm("\n.pushsection veramobd_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraMoBd.ttf\"\n.popsection\n");
asm("\n.pushsection veramobi_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraMoBI.ttf\"\n.popsection\n");
asm("\n.pushsection verase_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraSe.ttf\"\n.popsection\n");
asm("\n.pushsection verasebd_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraSeBd.ttf\"\n.popsection\n");

#elif defined __OSX__

// On Mac, we need a different incantation to use .incbin
asm("\t.global ___start_vera_ttf\n\t.global ___stop_vera_ttf\n___start_vera_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/Vera.ttf\"\n___stop_vera_ttf:\n");
asm("\t.global ___start_verait_ttf\n\t.global ___stop_verait_ttf\n___start_verait_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraIt.ttf\"\n___stop_verait_ttf:\n");
asm("\t.global ___start_verabd_ttf\n\t.global ___stop_verabd_ttf\n___start_verabd_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraBd.ttf\"\n___stop_verabd_ttf:\n");
asm("\t.global ___start_verabi_ttf\n\t.global ___stop_verabi_ttf\n___start_verabi_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraBI.ttf\"\n___stop_verabi_ttf:\n");
asm("\t.global ___start_veramono_ttf\n\t.global ___stop_veramono_ttf\n___start_veramono_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraMono.ttf\"\n___stop_veramono_ttf:\n");
asm("\t.global ___start_veramoit_ttf\n\t.global ___stop_veramoit_ttf\n___start_veramoit_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraMoIt.ttf\"\n___stop_veramoit_ttf:\n");
asm("\t.global ___start_veramobd_ttf\n\t.global ___stop_veramobd_ttf\n___start_veramobd_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraMoBd.ttf\"\n___stop_veramobd_ttf:\n");
asm("\t.global ___start_veramobi_ttf\n\t.global ___stop_veramobi_ttf\n___start_veramobi_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraMoBI.ttf\"\n___stop_veramobi_ttf:\n");
asm("\t.global ___start_verase_ttf\n\t.global ___stop_verase_ttf\n___start_verase_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraSe.ttf\"\n___stop_verase_ttf:\n");
asm("\t.global ___start_verasebd_ttf\n\t.global ___stop_verasebd_ttf\n___start_verasebd_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraSeBd.ttf\"\n___stop_verasebd_ttf:\n");

#else
# error "Inline assembly code for including truetype fonts in the binary only work on Linux/MacOS (and then, probably only on Intel compatible compilers. Sorry about that!"
#endif

// These external pointers are set up by the inline assembly above
extern const char __start_verabd_ttf[];
extern const char __stop_verabd_ttf[];
extern const char __start_verabi_ttf[];
extern const char __stop_verabi_ttf[];
extern const char __start_verait_ttf[];
extern const char __stop_verait_ttf[];
extern const char __start_veramobd_ttf[];
extern const char __stop_veramobd_ttf[];
extern const char __start_veramobi_ttf[];
extern const char __stop_veramobi_ttf[];
extern const char __start_veramoit_ttf[];
extern const char __stop_veramoit_ttf[];
extern const char __start_veramono_ttf[];
extern const char __stop_veramono_ttf[];
extern const char __start_verasebd_ttf[];
extern const char __stop_verasebd_ttf[];
extern const char __start_verase_ttf[];
extern const char __stop_verase_ttf[];
extern const char __start_vera_ttf[];
extern const char __stop_vera_ttf[];

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
            VisualFace (const morph::VisualFont _font, unsigned int fontpixels, FT_Library& ft_freetype)
            {
                std::string fontpath = "";

                switch (_font) {
                case VisualFont::Vera:
                {
                    fontpath = "/tmp/Vera.ttf";
                    this->makeTempFontFile (fontpath, __start_vera_ttf, __stop_vera_ttf);
                    break;
                }
                case VisualFont::VeraItalic:
                {
                    fontpath = "/tmp/VeraIt.ttf";
                    this->makeTempFontFile (fontpath, __start_verait_ttf, __stop_verait_ttf);
                    break;
                }
                case VisualFont::VeraBold:
                {
                    fontpath = "/tmp/VeraBd.ttf";
                    this->makeTempFontFile (fontpath, __start_verabd_ttf, __stop_verabd_ttf);
                    break;
                }
                case VisualFont::VeraBoldItalic:
                {
                    fontpath = "/tmp/VeraBI.ttf";
                    this->makeTempFontFile (fontpath, __start_verabi_ttf, __stop_verabi_ttf);
                    break;
                }
                case VisualFont::VeraMono:
                {
                    fontpath = "/tmp/VeraMono.ttf";
                    this->makeTempFontFile (fontpath, __start_veramono_ttf, __stop_veramono_ttf);
                    break;
                }
                case VisualFont::VeraMonoBold:
                {
                    fontpath = "/tmp/VeraMoBd.ttf";
                    this->makeTempFontFile (fontpath, __start_veramobd_ttf, __stop_veramobd_ttf);
                    break;
                }
                case VisualFont::VeraMonoItalic:
                {
                    fontpath = "/tmp/VeraMoIt.ttf";
                    this->makeTempFontFile (fontpath, __start_veramoit_ttf, __stop_veramoit_ttf);
                    break;
                }
                case VisualFont::VeraMonoBoldItalic:
                {
                    fontpath = "/tmp/VeraMoBI.ttf";
                    this->makeTempFontFile (fontpath, __start_veramobi_ttf, __stop_veramobi_ttf);
                    break;
                }
                case VisualFont::VeraSerif:
                {
                    fontpath = "/tmp/VeraSe.ttf";
                    this->makeTempFontFile (fontpath, __start_verase_ttf, __stop_verase_ttf);
                    break;
                }
                case VisualFont::VeraSerifBold:
                {
                    fontpath = "/tmp/VeraSeBd.ttf";
                    this->makeTempFontFile (fontpath, __start_verasebd_ttf, __stop_verasebd_ttf);
                    break;
                }
                default:
                {
                    std::cout << "ERROR::Unsupported morph font\n";
                    break;
                }
                }

                // Keep the face as a morph::Visual owned resource, shared by VisTextModels?
                if (FT_New_Face (ft_freetype, fontpath.c_str(), 0, &this->face)) {
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

        private:

            //! Create a temporary font file at fontpath, using the embedded data
            //! starting from filestart and extending to filenend
            void makeTempFontFile (const std::string& fontpath, const char* file_start, const char* file_stop)
            {
                const char* p;
                if (!morph::Tools::fileExists (fontpath)) {
                    std::ofstream fout;
                    fout.open (fontpath.c_str(), std::ios::out | std::ios::trunc);
                    if (fout.is_open()) {
                        for (p = file_start; p < file_stop; p++) { fout << *p; }
                        fout.close();
                    } else {
                        std::cout << "WARNING: Failed to open " << fontpath << "!!\n";
                    }
                } else {
                    std::cout << "INFO: " << fontpath << " already exists, no need to re-create it\n";
                }
            }
        };
    } // namespace gl
} // namespace morph
