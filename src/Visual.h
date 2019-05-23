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

#include <string>
using std::string;
#include <array>
using std::array;
#include <vector>
using std::vector;

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
        Visual (int width, int height, const string& title);
        ~Visual();

        static void errorCallback (int error, const char* description);

        //void setTitle(const string& s);
        //void saveImage (const string& s);

        //void addHex (array<float, 3> coords, float radius, array<float, 3> colour);

        // Maybe:
        //void updateHexGrid (HexGrid* hg, const vector<float>& dat);

        // We redraw, updating colour and 3D position based on current rotation etc.
        //void redraw();


    private:
        /*!
         * The window (and OpenGL context) for this Visual
         */
        GLFWwindow* window;
    };

} // namespace morph

#endif // _VISUAL_H_
