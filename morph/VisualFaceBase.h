/*!
 * \file
 *
 * Declares a VisualFace class to hold the information about a (Freetype-managed) font
 * face and the GL-textures that will reproduce it.
 *
 * This is the non-GL base class.
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
#include <morph/VisualCommon.h> // for visgl::CharInfo
#include <morph/VisualFont.h>
#include <morph/TextFeatures.h>

// FreeType for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

/*
 * The following inline assembly incorporates Vera.ttf and friends *into the binary*. We
 * have different code for Linux and Mac. Both tested only on Intel CPUs.
 */

#ifdef __GLN__

# ifdef __AARCH64__

// "a", @progbits isn't liked by pi/arm, but "a", %progbits DOES seem to be necessary
asm("\n.pushsection vera_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/Vera.ttf\"\n.popsection\n");
asm("\n.pushsection verait_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraIt.ttf\"\n.popsection\n");
asm("\n.pushsection verabd_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraBd.ttf\"\n.popsection\n");
asm("\n.pushsection verabi_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraBI.ttf\"\n.popsection\n");
asm("\n.pushsection veramono_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraMono.ttf\"\n.popsection\n");
asm("\n.pushsection veramoit_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraMoIt.ttf\"\n.popsection\n");
asm("\n.pushsection veramobd_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraMoBd.ttf\"\n.popsection\n");
asm("\n.pushsection veramobi_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraMoBI.ttf\"\n.popsection\n");
asm("\n.pushsection verase_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraSe.ttf\"\n.popsection\n");
asm("\n.pushsection verasebd_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/ttf-bitstream-vera/VeraSeBd.ttf\"\n.popsection\n");

// DejaVu Sans allows for Greek symbols and will be the default
asm("\n.pushsection dvsans_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/dejavu/DejaVuSans.ttf\"\n.popsection\n");
asm("\n.pushsection dvsansit_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/dejavu/DejaVuSans-Oblique.ttf\"\n.popsection\n");
asm("\n.pushsection dvsansbd_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/dejavu/DejaVuSans-Bold.ttf\"\n.popsection\n");
asm("\n.pushsection dvsansbi_ttf, \"a\", %progbits\n.incbin \"" MORPH_FONTS_DIR "/dejavu/DejaVuSans-BoldOblique.ttf\"\n.popsection\n");

# else

// "a", @progbits means 'allocatable section containing type data'. It seems not to be strictly necessary.
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

// DejaVu Sans allows for Greek symbols and will be the default
asm("\n.pushsection dvsans_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/dejavu/DejaVuSans.ttf\"\n.popsection\n");
asm("\n.pushsection dvsansit_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/dejavu/DejaVuSans-Oblique.ttf\"\n.popsection\n");
asm("\n.pushsection dvsansbd_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/dejavu/DejaVuSans-Bold.ttf\"\n.popsection\n");
asm("\n.pushsection dvsansbi_ttf, \"a\", @progbits\n.incbin \"" MORPH_FONTS_DIR "/dejavu/DejaVuSans-BoldOblique.ttf\"\n.popsection\n");

#endif

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

asm("\t.global ___start_dvsans_ttf\n\t.global ___stop_dvsans_ttf\n___start_dvsans_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/dejavu/DejaVuSans.ttf\"\n___stop_dvsans_ttf:\n");
asm("\t.global ___start_dvsansit_ttf\n\t.global ___stop_dvsansit_ttf\n___start_dvsansit_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/dejavu/DejaVuSans-Oblique.ttf\"\n___stop_dvsansit_ttf:\n");
asm("\t.global ___start_dvsansbd_ttf\n\t.global ___stop_dvsansbd_ttf\n___start_dvsansbd_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/dejavu/DejaVuSans-Bold.ttf\"\n___stop_dvsansbd_ttf:\n");
asm("\t.global ___start_dvsansbi_ttf\n\t.global ___stop_dvsansbi_ttf\n___start_dvsansbi_ttf:\n\t.incbin \"" MORPH_FONTS_DIR "/dejavu/DejaVuSans-BoldOblique.ttf\"\n___stop_dvsansbi_ttf:\n");

#elif defined __WIN__

# include <morph/fonts/verafonts.h> // Includes vera fonts AND DejaVu fonts.
# include <cstdlib>

#elif defined __WIN__INCBIN // Only for parsing this file with the incbin executable to create verafonts.h

// Visual Studio doesn't allow __asm{} calls in C__ code anymore, so try Dale Weiler's incbin.h
#define INCBIN_PREFIX vf_
#include <morph/fonts/incbin.h>
INCBIN(vera, "./fonts/ttf-bitstream-vera/Vera.ttf");
INCBIN(verait, "./fonts/ttf-bitstream-vera/VeraIt.ttf");
INCBIN(verabd, "./fonts/ttf-bitstream-vera/VeraBd.ttf");
INCBIN(verabi, "./fonts/ttf-bitstream-vera/VeraBI.ttf");
INCBIN(veramono, "./fonts/ttf-bitstream-vera/VeraMono.ttf");
INCBIN(veramoit, "./fonts/ttf-bitstream-vera/VeraMoIt.ttf");
INCBIN(veramobd, "./fonts/ttf-bitstream-vera/VeraMoBd.ttf");
INCBIN(veramobi, "./fonts/ttf-bitstream-vera/VeraMoBI.ttf");
INCBIN(verase, "./fonts/ttf-bitstream-vera/VeraSe.ttf");
INCBIN(verasebd, "./fonts/ttf-bitstream-vera/VeraSeBd.ttf");
// These translation units now have three symbols, eg:
// extern const unsigned char vf_veraData[];
// extern const unsigned char *const vf_veraEnd;
// extern const unsigned int vf_veraSize;

INCBIN(dvsans, "./fonts/dejavu/DejaVuSans.ttf");
INCBIN(dvsansit, "./fonts/dejavu/DejaVuSans-Oblique.ttf");
INCBIN(dvsansbd, "./fonts/dejavu/DejaVuSans-Bold.ttf");
INCBIN(dvsansbi, "./fonts/dejavu/DejaVuSans-BoldOblique.ttf");

#else
# error "Inline assembly code for including truetype fonts in the binary only work on Linux/MacOS (and then, probably only on Intel compatible compilers. Sorry about that!"
#endif

// These external pointers are set up by the inline assembly above
#ifndef __WIN__
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

extern const char __start_dvsans_ttf[];
extern const char __stop_dvsans_ttf[];
extern const char __start_dvsansit_ttf[];
extern const char __stop_dvsansit_ttf[];
extern const char __start_dvsansbd_ttf[];
extern const char __stop_dvsansbd_ttf[];
extern const char __start_dvsansbi_ttf[];
extern const char __stop_dvsansbi_ttf[];
#endif

namespace morph {

    namespace visgl {

        struct VisualFaceBase
        {
            VisualFaceBase () {}
            ~VisualFaceBase () {}

            //! Set true for informational/debug messages
            static constexpr bool debug_visualface = false;

            //! The FT_Face that we're managing
            FT_Face face;

            //! The OpenGL character info stuff
            std::map<char32_t, morph::visgl::CharInfo> glchars;

        protected:

            void init_common (const morph::VisualFont _font, unsigned int fontpixels, FT_Library& ft_freetype)
            {
                std::string fontpath = "";
#ifdef __WIN__
		char* userprofile = getenv ("USERPROFILE");
		std::string uppath("");
		if (userprofile != nullptr) {
		    uppath = std::string (userprofile);
		}

                switch (_font) {
                case VisualFont::DVSans:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\DejaVuSans.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_dvsansData, vf_dvsansEnd);
                    break;
                }
                case VisualFont::DVSansItalic:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\DejaVuSans-Oblique.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_dvsansitData, vf_dvsansitEnd);
                    break;
                }
                case VisualFont::DVSansBold:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\DejaVuSans-Bold.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_dvsansbdData, vf_dvsansbdEnd);
                    break;
                }
                case VisualFont::DVSansBoldItalic:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\DejaVuSans-BoldOblique.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_dvsansbiData, vf_dvsansbiEnd);
                    break;
                }
                case VisualFont::Vera:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\Vera.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_veraData, vf_veraEnd);
                    break;
                }
                case VisualFont::VeraItalic:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\VeraIt.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_veraitData, vf_veraitEnd);
                    break;
                }
                case VisualFont::VeraBold:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\VeraBd.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_verabdData, vf_verabdEnd);
                    break;
                }
                case VisualFont::VeraBoldItalic:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\VeraBI.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_verabiData, vf_verabiEnd);
                    break;
                }
                case VisualFont::VeraMono:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\VeraMono.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_veramonoData, vf_veramonoEnd);
                    break;
                }
                case VisualFont::VeraMonoBold:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\VeraMoBd.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_veramobdData, vf_veramobdEnd);
                    break;
                }
                case VisualFont::VeraMonoItalic:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\VeraMoIt.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_veramoitData, vf_veramoitEnd);
                    break;
                }
                case VisualFont::VeraMonoBoldItalic:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\VeraMoBI.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_veramobiData, vf_veramobiEnd);
                    break;
                }
                case VisualFont::VeraSerif:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\VeraSe.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_veraseData, vf_veraseEnd);
                    break;
                }
                case VisualFont::VeraSerifBold:
                {
                    fontpath = uppath + "\\AppData\\Local\\Temp\\VeraSeBd.ttf";
                    this->makeTempFontFile<const unsigned char> (fontpath, vf_verasebdData, vf_verasebdEnd);
                    break;
                }
                default:
                {
                    std::cout << "ERROR::Unsupported morph font\n";
                    break;
                }
                }
#else	 // Non-windows:
                switch (_font) {
                case VisualFont::DVSans:
                {
                    fontpath = "/tmp/DejaVuSans.ttf";
                    this->makeTempFontFile (fontpath, __start_dvsans_ttf, __stop_dvsans_ttf);
                    break;
                }
                case VisualFont::DVSansItalic:
                {
                    fontpath = "/tmp/DejaVuSans-Oblique.ttf";
                    this->makeTempFontFile (fontpath, __start_dvsansit_ttf, __stop_dvsansit_ttf);
                    break;
                }
                case VisualFont::DVSansBold:
                {
                    fontpath = "/tmp/DejaVuSans-Bold.ttf";
                    this->makeTempFontFile (fontpath, __start_dvsansbd_ttf, __stop_dvsansbd_ttf);
                    break;
                }
                case VisualFont::DVSansBoldItalic:
                {
                    fontpath = "/tmp/DejaVuSans-BoldOblique.ttf";
                    this->makeTempFontFile (fontpath, __start_dvsansbi_ttf, __stop_dvsansbi_ttf);
                    break;
                }
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
#endif // Windows/Non-windows

                // Keep the face as a morph::Visual owned resource, shared by VisTextModels?
                if constexpr (debug_visualface == true) {
                    std::cout << "FT_New_Face (ft_freetype, " << fontpath << ", 0, &this->face);\n";
                }
                if (FT_New_Face (ft_freetype, fontpath.c_str(), 0, &this->face)) {
                    std::cout << "ERROR::FREETYPE: Failed to load font (font file may be invalid)" << std::endl;
                }

                FT_Set_Pixel_Sizes (this->face, 0, fontpixels);

                // Can I check this->face for how many glyphs it has? Yes:
                // std::cout << "This face has " << this->face->num_glyphs << " glyphs.\n";
            }

            //! Create a temporary font file at fontpath, using the embedded data
            //! starting from filestart and extending to filenend
            template <typename T = const char>
            void makeTempFontFile (const std::string& fontpath, T* file_start, T* file_stop)
            {
                T* p;
                if (!morph::tools::fileExists (fontpath)) {
                    std::ofstream fout;
                    fout.open (fontpath.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
                    if (fout.is_open()) {
                        for (p = file_start; p < file_stop; p++) { fout << *p; }
                        fout.close();
                    } else {
                        std::cout << "WARNING: Failed to open " << fontpath << "!!\n";
                    }
                } else {
                    if constexpr (debug_visualface == true) {
                        std::cout << "INFO: " << fontpath << " already exists, no need to re-create it\n";
                    }
                }
            }
        };
    } // namespace gl
} // namespace morph
