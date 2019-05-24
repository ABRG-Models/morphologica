#ifndef _HEXGRIDVISUAL_H_
#define _HEXGRIDVISUAL_H_

#include <OpenGLBuffer> // etc

#include <vector>
using std::vector;

typedef GLuint VBOint;
#define VBO_ENUM_TYPE GL_UNSIGNED_INT

class HexGridVisual
{
public:
    HexGridVisual(const HexGrid* _hg,
                  const vector<float>& _data,
                  const array<float, 3> _offset);

    ~HexGridVisual();

    //! Initialize vertex buffer objects and vertex array object.
    void initialize (void);

    //! Render the HexGridVisual
    void render (void);

    //! The offset of this HexGridVisual
    array<float, 3> offset;

private:
    //! The HexGrid to visualize
    const HexGrid* hg;

    //! The data to visualize as z/colour
    const vector<float>& data;

    // Add a way to control the scaling scheme here.

    /*!
     * Compute positions and colours of vertices for the hexes and
     * store in these:
     */
    //@{
    //! Indices Vertex Buffer Object
    OpenGLBuffer ivbo;
    //! positions Vertex Buffer Object
    OpenGLBuffer pvbo;
    //! normals Vertex Buffer Object
    OpenGLBuffer nvbo;
    //! colors Vertex Buffer Object
    OpenGLBuffer cvbo;

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
    OpenGLShaderProgram* shaderProgram;

    //! The OpenGL Vertex Array Object
    OpenGLVertexArrayObject vao;

    //! Push three floats onto the vector of floats @vp
    void vertex_push (const float& x, const float& y, const float& z, vector<float>& vp);

    //! Set up a vertex buffer object
    void setupVBO (OpenGLBuffer& buf, vector<float>& dat, const char* arrayname);
};

#endif // _HEXGRIDVISUAL_H_
