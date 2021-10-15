/*
 * A VisualModel for rendering a set of 3D axes, either 3 axes or a kind of framework
 * box. Use along with ScatterVisual or HexGridVisual for plotting 3D graph
 * visualisations.
 */
#pragma once

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif

#include <morph/Scale.h>
#include <morph/Vector.h>

namespace morph {

    class TriaxesVisual : public VisualModel
    {
    public:
        TriaxesVisual (GLuint sp, const Vector<float> _offset)
        {
        }
    };

} // namespace morph
