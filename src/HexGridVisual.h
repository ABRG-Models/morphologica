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

        //! Set up a vertex buffer object
        void setupVBO (GLuint& buf, vector<float>& dat, unsigned int bufferAttribPosition);
    };

} // namespace morph

#endif // _HEXGRIDVISUAL_H_
