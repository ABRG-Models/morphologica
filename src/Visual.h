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

//#include <GL/glx.h>
//#include <GL/glu.h>
#include <vgl.h>

using std::vector;
using std::array;

namespace morph {

    /*!
     * A class for visualising computational models on an OpenGL screen
     */
    class Visual
    {
    public:
        /*!
         * Construct a new visualiser. The rule is 1 window to one
         * Visual object. So, this creates a new window and a new
         * OpenGL context.
         */
        Visual();

        void setTitle(const std::string& s);
        void saveImage (const std::string& s);

        void addHex (array<float, 3> coords, float radius, array<float, 3> colour);

        // Maybe:
        void updateHexGrid (HexGrid* hg, const vector<float>& dat);

        // We redraw, updating colour and 3D position based on current rotation etc.
        void redraw();
    };

} // namespace morph

#endif // _VISUAL_H_
