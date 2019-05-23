/*!
 * Visual.h
 *
 * Graphics code. Replacement for display.h. Uses modern OpenGL and
 * the library GLFW for window management.
 *
 * Created by Seb James on 2019/05/01
 */

#ifndef _VISUAL_H_
#define _VISUAL_H_

#include <GLFW/glfw3.h>
#include "HexGrid.h"

#include <string>
using std::string;
#include <array>
using std::array;
#include <vector>
using std::vector;

namespace morph {

    class HexGridVisual
    {
    public:
        /*!
         * The offset of this HexGridVisual
         */
        array<float, 3> offset;
    };

    /*!
     * A class for visualising computational models on an OpenGL
     * screen. Will be specialised for rendering HexGrids to begin
     * with.
     *
     * Each Visual will have its own GLFW window and is essentially a
     * "scene" containing a number of objects. One object might be the
     * visualisation of some data expressed over a HexGrid. It should
     * be possible to translate objects with respect to each other and
     * also to rotate the entire scene, as well as use keys to
     * generate particular effects/views.
     */
    class Visual
    {
    public:
        /*!
         * Construct a new visualiser. The rule is 1 window to one
         * Visual object. So, this creates a new window and a new
         * OpenGL context.
         */
        Visual (int width, int height, const string& title);
        ~Visual();

        static void errorCallback (int error, const char* description);

        //void setTitle(const string& s);
        //void saveImage (const string& s);

        //void addHex (array<float, 3> coords, float radius, array<float, 3> colour);

        /*!
         * Add the vertices for the data in @dat, defined on the
         * HexGrid @hg to the visual. Offset every vertex using
         * @offset.
         */
        void updateHexGridVisual (const unsigned int gridId,
                                  const vector<float>& data);

        /*!
         * Add the vertices for the data in @dat, defined on the
         * HexGrid @hg to the visual. Offset every vertex using
         * @offset.
         */
        unsigned int addHexGridVisual (const HexGrid* hg,
                                       const vector<float>& data,
                                       const array<float, 3> offset);

        // We redraw, updating colour and 3D position based on current rotation etc.
        void redraw();

    private:
        /*!
         * The window (and OpenGL context) for this Visual
         */
        GLFWwindow* window;

        /*!
         * This Visual is going to render some HexGridVisuals for us. 1 or more.
         */
        vector<HexGridVisual*> HexGridVis;
    };

} // namespace morph

#endif // _VISUAL_H_
