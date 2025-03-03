/*!
 * \file
 *
 * Declares a class to hold vertices of the quads that are the backing for a sequence of text
 * characters. This is for use in VisualModel-derived classes. Within the backend, the
 * VisualTextModelImpl classes are used directly.
 *
 * \author Seb James
 * \date Oct 2020 - Mar 2025
 */

#pragma once

#ifdef GLAD_OPTION_GL_MX
# include <morph/VisualTextModelImplMX.h>
#else
# include <morph/VisualTextModelImpl.h>
#endif

namespace morph {
    // glad_type is set in VisualOwnable.h or VisualOwnableMX.h
    template <int glver = morph::gl::version_4_1>
    struct VisualTextModel : public morph::VisualTextModelImpl<morph::gl::multicontext, glver> {
        VisualTextModel(morph::TextFeatures _tf)
            : morph::VisualTextModelImpl<morph::gl::multicontext, glver>::VisualTextModelImpl(_tf) {}
    };
} // namespace morph
