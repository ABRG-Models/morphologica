/*!
 * \file
 *
 * Declares a VisualModel class to hold the vertices that make up some individual model object that
 * can be part of an OpenGL scene. This is the user-facing VisualModel.
 *
 * \author Seb James
 * \date May 2019 - March 2025
 */

#pragma once

#ifdef GLAD_OPTION_GL_MX
# include <morph/VisualModelImplMX.h>
#else
# include <morph/VisualModelImplNoMX.h>
#endif

namespace morph {

    /*!
     * An OpenGL model class
     *
     * This class is the 'OpenGL model' class. It has the common code to create the vertices for
     * some individual OpenGL model which is to be rendered in a 3-D scene.
     *
     * Some OpenGL models are derived directly from VisualModel; see for example morph::CoordArrows.
     *
     * Other models in morphologica are derived via morph::VisualDataModel, which adds a common
     * mechanism for managing the data which is to be visualised by the final 'Visual' object (such
     * as morph::HexGridVisual or morph::ScatterVisual)
     *
     * The base and implementation classes underlying class VisualModel contain some common 'object
     * primitives' code, such as computeSphere and computeCone, which compute the vertices that will
     * make up sphere and cone, respectively. If you need to see the primitives, look at
     * morph/VisualModelBase.h
     *
     * Note on morph::gl::multicontext. This is defined as a static constexpr int with the value 1
     * or 0 in <morph/VisualOwnableNoMX.h> or <morph/VisualOwnableMX.h>, one or other of which must have
     * been #included before you include <morph/VisualModel.h>
     */
    template <int glver = morph::gl::version_4_1>
    struct VisualModel : public morph::VisualModelImpl<glver, morph::gl::multicontext> {
        VisualModel() : morph::VisualModelImpl<glver, morph::gl::multicontext>::VisualModelImpl() {}
        VisualModel(const morph::vec<float, 3>& _mv_offset)
            : morph::VisualModelImpl<glver, morph::gl::multicontext>::VisualModelImpl(_mv_offset) {}
    };

} // namespace morph
