/*!
 * \file
 *
 * Graphics code. A replacement for morph::Gdisplay from display.h. Uses modern OpenGL
 * and the library GLFW for window management.
 *
 * Created by Seb James on 2019/05/01
 *
 * \author Seb James
 * \date May 2019
 */
#pragma once

#ifdef USE_GLEW
// Including glew.h and linking with libglew helps older platforms,
// such as Ubuntu 16.04. Not necessary on later platforms.
# include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include "HexGrid.h"
#include "VisualModel.h"
#include "HexGridVisual.h"
#include "QuadsVisual.h"
#include "PointRowsVisual.h"
#include "ScatterVisual.h"
#include "QuiverVisual.h"
#include "CoordArrows.h"
#ifdef TRIANGLE_VIS_TESTING
# include "TriangleVisual.h"
#endif
#include "Quaternion.h"
#include "TransformMatrix.h"
#include "Vector.h"

// A base class with static event handling dispatchers
#include "VisualBase.h"

#include "ColourMap.h"

#include "GL3/gl3.h"

#include <string>
#include <array>
#include <vector>

//! The default z=0 position for VisualModels
#define Z_DEFAULT -5

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
     * Visual 'scene' class
     *
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
        Visual (int width, int height, const std::string& title);
        ~Visual();

        static void errorCallback (int error, const char* description);

        /*!
         * Take a screenshot of the window
         */
        void saveImage (const std::string& s);

        /*!
         * Add a VisualModel to the scene. The VisualModel* should be a pointer to a
         * visual model which has been newly allocated by the client code. Do not add
         * a pointer to the same VisualModel more than once! When this morph::Visual
         * object goes out of scope, its deconstructor will delete each VisualModel
         * that it has a pointer for.
         */
        unsigned int addVisualModel (VisualModel* model);

        /*!
         * VisualModel Getter
         *
         * For the given \a modelId, return a pointer to the visual model.
         *
         * \return VisualModel pointer
         */
        VisualModel* getVisualModel (unsigned int modelId);

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
        ///@{
        float zNear = 1.0;
        float zFar = 15.0;
        float fov = 30.0;
        ///@}

        //! Set to true to show the coordinate arrows
        bool showCoordArrows = false;

        //! How big should the steps in scene translation be when scrolling?
        float scenetrans_stepsize = 0.1;

        //! If you set this to true, then the mouse movements won't change scenetrans
        //! or rotation.
        bool sceneLocked = false;

        //! The background colour; black by default.
        std::array<float, 4> bgcolour = { 0.0f, 0.0f, 0.0f, 0.0f };

        /*!
         * User can directly set bgcolour for any background colour they like, but
         * here are convenience functions:
         */
        ///@{
        void backgroundWhite (void) {
            this->bgcolour = { 1.0f, 1.0f, 1.0f, 0.5f };
        }
        void backgroundBlack (void) {
            this->bgcolour = { 0.0f, 0.0f, 0.0f, 0.0f };
        }
        ///@}

        //! Setter for zDefault
        void setZDefault (float f) {
            if (f>0.0f) {
                std::cout << "WARNING setZDefault(): Normally, the default z value is negative." << std::endl;
            }
            this->zDefault = f;
            this->scenetrans[2] = f;
        }

        //! Setters for x/y
        ///@{
        void setSceneTransXY (float _x, float _y) {
            this->scenetrans[0] = _x;
            this->scenetrans[1] = _y;
        }
        void setSceneTransX (float _x) {
            this->scenetrans[0] = _x;
        }
        void setSceneTransY (float _y) {
            this->scenetrans[1] = _y;
        }
        ///@}

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
        ///@{
        int window_w;
        int window_h;
        ///@}

        //! A vector of pointers to all the morph::VisualModels (HexGridVisual,
        //! ScatterVisual, etc) which are going to be rendered in the scene.
        std::vector<VisualModel*> vm;

        //! A little model of the coordinate axes.
        CoordArrows* coordArrows;

        //! Position and length of coordinate arrows. Need to be configurable at Visual
        //! construction.
        std::array<float, 3> coordArrowsOffset = {0.0/* -1.5 */, 0.0, 0.0};
        std::array<float, 3> coordArrowsLength = {1., 1., 1.};

        /*!
         * Variables to manage projection and rotation of the object
         */
        ///@{

        //! Current cursor position
        Vector<float,2> cursorpos = {0.0f, 0.0f};

        //! Holds the translation coordinates for the current location of the entire scene
        Vector<float,3> scenetrans = {0.0, 0.0, Z_DEFAULT};

        //! Default for scenetrans
        const Vector<float> scenetrans_default = {0.0, 0.0, Z_DEFAULT};

        //! When true, cursor movements induce rotation of scene
        bool rotateMode = false;

        //! When true, rotations about the third axis are possible.
        bool rotateModMode = false;

        //! When true, cursor movements induce translation of scene
        bool translateMode = false;

        //! Screen coordinates of the position of the last mouse press
        Vector<float,2> mousePressPosition = {0.0f, 0.0f};

        //! The current rotation axis. World frame?
        Vector<float> rotationAxis = {0.0f, 0.0f, 0.0f};

        //! A rotation quaternion. You could have guessed that, right?
        Quaternion<float> rotation;

        //! The projection matrix is a member of this class
        TransformMatrix<float> projection;

        //! The inverse of the projection
        TransformMatrix<float> invproj;

        //! A scene transformation and associated inverse
        ///@{
        TransformMatrix<float> scene;
        TransformMatrix<float> invscene;
        ///@}
        Quaternion<float> savedRotation;

        ///@}

        /*!
         * GLFW callback handlers
         */
        ///@{
        virtual void key_callback (GLFWwindow* window, int key, int scancode, int action, int mods);
        virtual void cursor_position_callback (GLFWwindow* window, double x, double y);
        virtual void mouse_button_callback (GLFWwindow* window, int button, int action, int mods);
        virtual void window_size_callback (GLFWwindow* window, int width, int height);
        virtual void scroll_callback (GLFWwindow* window, double xoffset, double yoffset);
        ///@}
    };

} // namespace morph
