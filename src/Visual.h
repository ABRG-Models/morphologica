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
#include "HexGridVisual.h"
#include "Quaternion.h"
#include "TransformMatrix.h"
#include "Vector2.h"
#include "Vector3.h"

// A base class with static event handling dispatchers
#include "VisualBase.h"

#include "GL3/gl3.h"

#include <string>
using std::string;
#include <array>
using std::array;
#include <vector>
using std::vector;

namespace morph {

    /*
     * LoadShaders() takes an array of ShaderFile structures, each of
     * which contains the type of the shader, and a pointer a C-style
     * character string (i.e., a NULL-terminated array of characters)
     * containing the entire shader source.
     *
     * The array of structures is terminated by a final Shader with
     * the "type" field set to GL_NONE.
     *
     * LoadShaders() returns the shader program value (as returned by
     * glCreateProgram()) on success, or zero on failure.
     */

    typedef struct
    {
        GLenum type;
        const char* filename;
        GLuint shader;
    } ShaderInfo;

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
    class Visual : VisualBase
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

        /*!
         * Render the scene
         */
        void render();

        /*!
         * The OpenGL shader program
         */
        GLuint shaderprog;

        //! trigger on mouse press event
        void mousePressEvent (void);

        //! trigger on mouse release event
        void mouseReleaseEvent (void);

        //! trigger on timer event
        void timerEvent (void);

        //! Set perspective based on window width and height
        void setPerspective (void);

        //! Set to true when the program should end
        bool readyToFinish = false;

    private:

        /*!
         * Read a shader
         */
        const GLchar* ReadShader (const char* filename);

        /*!
         * Shader loading code
         */
        GLuint LoadShaders (ShaderInfo* si);

        /*!
         * The window (and OpenGL context) for this Visual
         */
        GLFWwindow* window;

        /*!
         * This Visual is going to render some HexGridVisuals for us. 1 or more.
         */
        vector<HexGridVisual*> hexGridVis;

        /*!
         * Variables to manage projection and rotation of the object
         */
        //@{
        Vector2<float> mousePressPosition;
        Vector3<float> rotationAxis;
        float angularSpeed;
        Quaternion<float> rotation;
        TransformMatrix<float> projection;
        //@}

        virtual void key_callback (GLFWwindow* window, int key, int scancode, int action, int mods);
    };

} // namespace morph

#endif // _VISUAL_H_
