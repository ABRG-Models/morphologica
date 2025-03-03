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

#include <iostream>
#include <tuple>
#include <set>
#include <stdexcept>
#include <memory>
#include <morph/gl/version.h>
#include <morph/VisualFont.h>
// FreeType for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

namespace morph {

    // Pointers to morph::VisualBase are used to index font faces
    template<int>
    class VisualBase;

    //! Singleton resource class for morph::Visual scenes. (base class, with no GL calls, and no
    //! instance function)
    template <int glver>
    class VisualResourcesBase
    {
    protected:
        VisualResourcesBase() { }
        ~VisualResourcesBase()
        {
            // As with the case for faces, when each morph::Visual goes out of scope, the FreeType
            // instance gets cleaned up. So at this stage freetypes should also be empy and nothing
            // will happen here either.
            for (auto& ft : this->freetypes) { FT_Done_FreeType (ft.second); }
        }

        //! FreeType library object
        std::map<morph::VisualBase<glver>*, FT_Library> freetypes;

    public:
        VisualResourcesBase(const VisualResourcesBase<glver>&) = delete;
        VisualResourcesBase& operator=(const VisualResourcesBase<glver> &) = delete;
        VisualResourcesBase(VisualResourcesBase<glver> &&) = delete;
        VisualResourcesBase & operator=(VisualResourcesBase<glver> &&) = delete;

        //! A function to call to simply make sure the singleton instance exists. In derived class
        //! this could be a no-op.
        virtual void create() = 0;

        // Note: freetype_init function is in derived class

        //! When a morph::Visual goes out of scope, its freetype library instance should be
        //! deinitialized.
        void freetype_deinit (morph::VisualBase<glver>* _vis)
        {
            // First clear the faces associated with VisualBase<>* _vis
            this->clearVisualFaces (_vis);
            // Second, clean up the FreeType library instance and erase from this->freetypes
            auto freetype = this->freetypes.find (_vis);
            if (freetype != this->freetypes.end()) {
                FT_Done_FreeType (freetype->second);
                this->freetypes.erase (freetype);
            }
        }

        // Note: get/clearVisualFace functions are in derived classes
        virtual void clearVisualFaces (morph::VisualBase<glver>* _vis) = 0;
    };

} // namespace morph
