/*!
 * \file
 *
 * Declares a VisualModel class to hold the vertices that make up some individual
 * model object that can be part of an OpenGL scene.
 *
 * \author Seb James
 * \date May 2019 - Mar 2025
 */

#pragma once

/*
 * Depending on whether we used glad/gl_mx.h or glad/gl.h, we will have GLAD_OPTION_GL_MX defined or
 * undefined. From this set glad_type to select VisualModel.
 */
#ifdef GLAD_OPTION_GL_MX
# include <morph/VisualModelImplMX.h>
namespace morph { static constexpr int glad_type = 1; }
#else
# include <morph/VisualModelImpl.h>
namespace morph { static constexpr int glad_type = 0; }
#endif

namespace morph {

    /*!
     * OpenGL model base class
     *
     * This class is a base 'OpenGL model' class. It has the common code to create the vertices for
     * some individual OpengGL model which is to be rendered in a 3-D scene.
     *
     * Some OpenGL models are derived directly from VisualModel; see for example morph::CoordArrows.
     *
     * Other models in morphologica are derived via morph::VisualDataModel, which adds a common
     * mechanism for managing the data which is to be visualised by the final 'Visual' object (such
     * as morph::HexGridVisual or morph::ScatterVisual)
     *
     * The base and implementation classes underlying class VisualModel contain some common 'object
     * primitives' code, such as computeSphere and computeCone, which compute the vertices that will
     * make up sphere and cone, respectively.
     */
    template <int glver = morph::gl::version_4_1>
    struct VisualModel : public morph::VisualModelImpl<morph::glad_type, glver> {
        VisualModel() : morph::VisualModelImpl<morph::glad_type, glver>::VisualModelImpl() {}
        VisualModel(const morph::vec<float, 3>& _mv_offset)
            : morph::VisualModelImpl<morph::glad_type, glver>::VisualModelImpl(_mv_offset) {}
    };

} // namespace morph
