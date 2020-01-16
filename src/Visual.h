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
#include "CoordArrows.h"
#ifdef TRIANGLE_VIS_TESTING
# include "TriangleVisual.h"
#endif
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

//! The default z=0 position for HexGridVisual models
#define Z_DEFAULT 5

namespace morph {

    /*!
     * Data structure for shader info.
     *
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

        void saveImage (const string& s);

        /*!
         * Add the vertices for the data in @dat, defined on the HexGrid @hg to the
         * visual. Offset (spatially) every vertex using @offset. Scale the data
         * linearly using @scale, which defines a y=mx+c type of scaling, with
         * scale[0]='m' and scale[1]='c'. (plus a colour scaling defined in scale[2]
         * and scale[3] in the same way).
         */
        //@{
        void updateHexGridVisual (const unsigned int gridId,
                                  const vector<float>& data,
                                  const array<float, 4> scale);
        void updateHexGridVisual (const unsigned int gridId,
                                  const vector<double>& data,
                                  const array<double, 4> scale);
        //@}

        /*!
         * Add the vertices for the data in @dat, defined on the HexGrid @hg to the
         * visual. Spatially offset every vertex using @offset. A scaling must be
         * applied to the data; otherwise, the scalar values of data should be of the
         * same order as the positions of the hexes. Only the client code can
         * determine this, so the scaling has to be supplied, and could be modified
         * during visualization. Scale the data linearly using @scale, which defines a
         * y=mx+c type of scaling, with scale[0]='m' and scale[1]='c'.
         *
         * Returns a numeric ID which identifies where the HexGridVisual object lives
         * (hgv_float or hgv_double and which index therein).
         */
        //@{
        unsigned int addHexGridVisual (const HexGrid* hg,
                                       const array<float, 3> offset,
                                       const vector<float>& data,
                                       const array<float, 4> scale);
        unsigned int addHexGridVisual (const HexGrid* hg,
                                       const array<float, 3> offset,
                                       const vector<double>& data,
                                       const array<double, 4> scale);
        //@}

        /*!
         * Keep on rendering until readToFinish is set true. Used to keep a window
         * open, and responsive, while displaying the result of a simulation.
         */
        void keepOpen (void);

        //! Render the scene
        void render (void);

        //! The OpenGL shader program
        GLuint shaderprog;

        //! Set perspective based on window width and height
        void setPerspective (void);

        //! Set to true when the program should end
        bool readyToFinish = false;

        /*!
         * User-settable projection values for the near clipping distance, the far
         * clipping distance and the field of view of the camera.
         */
        //@{
        float zNear = 1.0;
        float zFar = 15.0;
        float fov = 30.0;
        //@}

        //! How big should the steps in scene translation be when scrolling?
        float scenetrans_stepsize = 0.1;

        //! Setter for zDefault
        void setZDefault (float f) {
            this->zDefault = f;
            this->scenetrans.z = -f;
        }

    private:

        //! The default z=0 position for HexGridVisual models
        float zDefault = Z_DEFAULT;

        /*!
         * Read a shader from a file.
         */
        const GLchar* ReadShader (const char* filename);

        /*!
         * Read a default shader, stored as a const char* like ReadShader reads a
         * file: allocate some memory, copy the text into the new memory and then
         * return a (GLchar*) pointer to the memory.
         */
        const GLchar* ReadDefaultShader (const char* shadercontent);

        /*!
         * Shader loading code
         */
        GLuint LoadShaders (ShaderInfo* si);

        /*!
         * The window (and OpenGL context) for this Visual
         */
        GLFWwindow* window;

        /*!
         * Current window width and height
         */
        //@{
        int window_w;
        int window_h;
        //@}

        /*!
         * This Visual is going to render some HexGridVisuals for us. 1 or
         * more. Various data types are possible.
         */
        vector<HexGridVisual<float>*> hgv_float;
        vector<HexGridVisual<double>*> hgv_double;
        // Plus int/unsigned int.

        //! A little model of the coordinate axes.
        CoordArrows* coordArrows;

        /*!
         * Variables to manage projection and rotation of the object
         */
        //@{

        //! Current cursor position
        Vector2<float> cursorpos;

        //! Holds the translation coordinates for the current location of the entire scene
        Vector3<float> scenetrans = {0.0, 0.0, -Z_DEFAULT};

        //! Default for scenetrans
        const Vector3<float> scenetrans_default = {0.0, 0.0, -Z_DEFAULT};

        //! When true, cursor movements induce rotation of scene
        bool rotateMode = false;

        //! When true, rotations about the third axis are possible.
        bool rotateModMode = false;

        //! When true, cursor movements induce translation of scene
        bool translateMode = false;

        //! Screen coordinates of the position of the last mouse press
        Vector2<float> mousePressPosition;

        //! The current rotation axis. World frame?
        Vector3<float> rotationAxis;

        //! A rotation quaternion. You could have guessed that, right?
        Quaternion<float> rotation;

        //! The projection matrix is a member of this class
        TransformMatrix<float> projection;

        //! The inverse of the projection
        TransformMatrix<float> invproj;

        //! A scene transformation and associated inverse
        //@{
        TransformMatrix<float> scene;
        TransformMatrix<float> invscene;
        //@}
        Quaternion<float> savedRotation;

        //@}

        /*!
         * GLFW callback handlers
         */
        //@{
        virtual void key_callback (GLFWwindow* window, int key, int scancode, int action, int mods);
        virtual void cursor_position_callback (GLFWwindow* window, double x, double y);
        virtual void mouse_button_callback (GLFWwindow* window, int button, int action, int mods);
        virtual void window_size_callback (GLFWwindow* window, int width, int height);
        virtual void scroll_callback (GLFWwindow* window, double xoffset, double yoffset);
        //@}
    };

} // namespace morph

#endif // _VISUAL_H_
