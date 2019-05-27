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

    // Because I put vertices in three buffers, all attributes start at position 0
    enum Attrib_IDs { vPosition = 0 };

    class HexGridVisual
    {
    public:
        HexGridVisual(const Visual* _parent,
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
        //! The parent Visual object - provides access to the shader prog
        const Visual* parent;

        //! The HexGrid to visualize
        const HexGrid* hg;

        //! The data to visualize as z/colour
        const vector<float>* data;

        // Add a way to control the scaling scheme here.

        /*!
         * Compute positions and colours of vertices for the hexes and
         * store in these:
         */
        //@{
        //! Indices Vertex Buffer Object
        GLuint* bufobjs;

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

        //! The OpenGL Vertex Array Object
        GLuint vaos[1];

        //! Push three floats onto the vector of floats @vp
        void vertex_push (const float& x, const float& y, const float& z, vector<float>& vp);

        //! Set up a vertex buffer object
        void setupVBO (GLuint& buf, vector<float>& dat, const char* arrayname);
    };

} // namespace morph

#endif // _HEXGRIDVISUAL_H_
