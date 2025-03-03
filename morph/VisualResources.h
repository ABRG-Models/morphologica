/*!
 * \file
 *
 * Declares a VisualResource class to hold the information about Freetype and any other
 * one-per-program resources.
 *
 * \author Seb James
 * \date November 2020
 */

#pragma once

#include <morph/VisualFace.h>
#include <morph/VisualResourcesBase.h>
#include <morph/gl/util.h>

namespace morph {

    // Pointers to morph::VisualBase are used to index font faces
    template<int>
    class VisualBase;

    //! Singleton resource class for morph::Visual scenes.
    template <int glver>
    class VisualResources : public VisualResourcesBase<glver>
    {
    private:
        VisualResources(){}
        // Normally, when each morph::Visual goes out of scope, the faces associated with that
        // Visual get cleaned up (in VisualResources::freetype_deinit). So at this point, faces
        // should be empty, and the following clear() should do nothing.
        ~VisualResources() { this->faces.clear(); }

        //! The collection of VisualFaces generated for this instance of the
        //! application. Create one VisualFace for each unique combination of VisualFont
        //! and fontpixels (the texture resolution)
        std::map<std::tuple<morph::VisualFont, unsigned int, morph::VisualBase<glver>*>,
                 std::unique_ptr<morph::visgl::VisualFace>> faces;
    public:
        VisualResources(const VisualResources<glver>&) = delete;
        VisualResources& operator=(const VisualResources<glver> &) = delete;
        VisualResources(VisualResources<glver> &&) = delete;
        VisualResources & operator=(VisualResources<glver> &&) = delete;

        //! The instance public function. Uses the very short name 'i' to keep code tidy.
        //! This relies on C++11 magic statics (N2660).
        static auto& i()
        {
            static VisualResources<glver> instance;
            return instance;
        }

        //! A function to call to simply make sure the singleton instance exists
        virtual void create() final {}

        //! Initialize a freetype library instance and add to this->freetypes. I wanted
        //! to have only a single freetype library instance, but this didn't work, so I
        //! create one FT_Library for each OpenGL context (i.e. one for each morph::Visual
        //! window). Thus, arguably, the FT_Library should be a member of morph::Visual,
        //! but that's a task for the future, as I coded it this way under the false
        //! assumption that I'd only need one FT_Library.
        void freetype_init (morph::VisualBase<glver>* _vis)
        {
            FT_Library freetype = nullptr;
            try {
                freetype = this->freetypes.at (_vis);
            } catch (const std::out_of_range&) {
                // Use of gl calls here may make it neat to set up GL here in VisualResources?
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

        //! Return a pointer to a VisualFace for the given \a font at the given texture
        //! resolution, \a fontpixels and the given window (i.e. OpenGL context) \a _win.
        morph::visgl::VisualFace* getVisualFace (morph::VisualFont font, unsigned int fontpixels, morph::VisualBase<glver>* _vis)
        {
            morph::visgl::VisualFace* rtn = nullptr;
            auto key = std::make_tuple(font, fontpixels, _vis);
            try {
                rtn = this->faces.at(key).get();
            } catch (const std::out_of_range&) {
                this->faces[key] = std::make_unique<morph::visgl::VisualFace> (font, fontpixels, this->freetypes.at(_vis));
                rtn = this->faces.at(key).get();
            }
            return rtn;
        }

        morph::visgl::VisualFace* getVisualFace (const morph::TextFeatures& tf, morph::VisualBase<glver>* _vis)
        {
            return this->getVisualFace (tf.font, tf.fontres, _vis);
        }

        //! Loop through this->faces clearing out those associated with the given morph::Visual
        virtual void clearVisualFaces (morph::VisualBase<glver>* _vis) final
        {
            auto f = this->faces.begin();
            while (f != this->faces.end()) {
                // f->first is a key. If its third, Visual<>* element == _vis, then delete and erase
                if (std::get<morph::VisualBase<glver>*>(f->first) == _vis) {
                    f = this->faces.erase (f);
                } else { f++; }
            }
        }
    };

} // namespace morph
