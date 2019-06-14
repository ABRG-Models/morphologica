#ifndef _HEXGRIDVISUAL_H_
#define _HEXGRIDVISUAL_H_

#include "GL3/gl3.h"

#include "HexGrid.h"

#include <vector>
using std::vector;
#include <array>
using std::array;

typedef GLuint VBOint;
#define VBO_ENUM_TYPE GL_UNSIGNED_INT

/*!
 * Macros for testing neighbours. The step along for neighbours on the
 * rows above/below is given by:
 *
 * Dest  | step
 * ----------------------
 * NNE   | +rowlen
 * NNW   | +rowlen - 1
 * NSW   | -rowlen
 * NSE   | -rowlen + 1
 */
//@{
#define NE(hi) (this->hg->d_ne[hi])
#define HAS_NE(hi) (this->hg->d_ne[hi] == -1 ? false : true)

#define NW(hi) (this->hg->d_nw[hi])
#define HAS_NW(hi) (this->hg->d_nw[hi] == -1 ? false : true)

#define NNE(hi) (this->hg->d_nne[hi])
#define HAS_NNE(hi) (this->hg->d_nne[hi] == -1 ? false : true)

#define NNW(hi) (this->hg->d_nnw[hi])
#define HAS_NNW(hi) (this->hg->d_nnw[hi] == -1 ? false : true)

#define NSE(hi) (this->hg->d_nse[hi])
#define HAS_NSE(hi) (this->hg->d_nse[hi] == -1 ? false : true)

#define NSW(hi) (this->hg->d_nsw[hi])
#define HAS_NSW(hi) (this->hg->d_nsw[hi] == -1 ? false : true)
//@}

#define IF_HAS_NE(hi, yesval, noval)  (HAS_NE(hi)  ? yesval : noval)
#define IF_HAS_NNE(hi, yesval, noval) (HAS_NNE(hi) ? yesval : noval)
#define IF_HAS_NNW(hi, yesval, noval) (HAS_NNW(hi) ? yesval : noval)
#define IF_HAS_NW(hi, yesval, noval)  (HAS_NW(hi)  ? yesval : noval)
#define IF_HAS_NSW(hi, yesval, noval) (HAS_NSW(hi) ? yesval : noval)
#define IF_HAS_NSE(hi, yesval, noval) (HAS_NSE(hi) ? yesval : noval)

namespace morph {

    //! Forward declaration of Visual class
    class Visual;

    //! The locations for the position, normal and colour vertex attributes in the GLSL program
    enum AttribLocn { posnLoc = 0, normLoc = 1, colLoc = 2 };

    class HexGridVisual
    {
    public:
        HexGridVisual(GLuint sp,
                      const HexGrid* _hg,
                      const vector<float>* _data,
                      const array<float, 3> _offset);

        ~HexGridVisual();

        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices (void);

        //! Render the HexGridVisual
        void render (void);

        //! The offset of this HexGridVisual
        array<float, 3> offset;

    private:
        //! This enum contains the positions within the vbo array of the different vertex buffer objects
        enum VBOPos { posnVBO, normVBO, colVBO, idxVBO, numVBO };

        //! The parent Visual object - provides access to the shader prog
        const Visual* parent;

        //! The HexGrid to visualize
        const HexGrid* hg;

        //! The data to visualize as z/colour
        const vector<float>* data;

        //! A copy of the reference to the shader program
        GLuint shaderprog;

        // Add a way to control the scaling scheme here.

        /*!
         * Compute positions and colours of vertices for the hexes and
         * store in these:
         */
        //@{
        //! The OpenGL Vertex Array Object
        GLuint vao;

        //! Vertex Buffer Objects stored in an array
        GLuint* vbos;

        //! CPU-side data for indices
        vector<VBOint> indices;
        //! CPU-side data for vertex positions
        vector<float> vertexPositions;
        //! CPU-side data for vertex normals
        vector<float> vertexNormals;
        //! CPU-side data for vertex colours
        vector<float> vertexColors;
        //@}

        //! I guess we'll need a shader program.
        GLuint* shaderProgram;

        //! Push three floats onto the vector of floats @vp
        void vertex_push (const float& x, const float& y, const float& z, vector<float>& vp);
        void vertex_push (const array<float, 3>& arr, vector<float>& vp);

        //! Set up a vertex buffer object
        void setupVBO (GLuint& buf, vector<float>& dat, unsigned int bufferAttribPosition);
    };

} // namespace morph

#endif // _HEXGRIDVISUAL_H_
