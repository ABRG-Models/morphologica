/*!
 * \file
 *
 * Awesome graphics code for high performance graphing and visualisation.
 *
 * This is the main visual scene class in morphologica and derives from
 * morph::VisualOwnable, adding window handling with GLFW3.
 *
 * Created by Seb James on 2019/05/01
 *
 * \author Seb James
 * \date May 2019
 */
#pragma once

#include <morph/VisualMX.h>

namespace morph {

    /*!
     * Visual 'scene' class
     *
     * A class for visualising computational models on an OpenGL screen.
     *
     * Each Visual will have its own GLFW window and is essentially a "scene" containing a number of
     * objects. One object might be the visualisation of some data expressed over a HexGrid. Another
     * could be a GraphVisual object. The class handles mouse events to allow the user to rotate and
     * translate the scene, as well as use keys to generate particular effects/views.
     *
     * morph::Visual is actually provided by the multi-context aware VisualMX class,
     * which loads GLAD headers with the multi context option MX enabled.
     *
     * If you want a morph::Visual which loads a single set of OpenGL function aliases
     * such as glClear, glEnable, glDisable and so on, you can use
     * morph::VisualSgl<>. If you want to be explicit about the fact that you're using
     * the multi-context aware class, use morph::VisualMX<>
     *
     * \tparam glver The OpenGL version, encoded as a single int (see morph::gl::version)
     */
    template <int glver = morph::gl::version_4_1>
    struct Visual : public morph::VisualMX<glver>
    {
        Visual (const int _width, const int _height, const std::string& _title, const bool _version_stdout = true)
            : morph::VisualMX<glver> (_width, _height, _title, _version_stdout) {}
    };

} // namespace morph
