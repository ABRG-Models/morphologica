#include <iostream>
using std::cout;
using std::endl;

HexGridVisual::HexGridVisual (const HexGrid* _hg,
                              const vector<float>& _data,
                              const array<float, 3> _offset)
    : ivbo(OpenGLBuffer::IndexBuffer)
    , pvbo(OpenGLBuffer::VertexBuffer)
    , nvbo(OpenGLBuffer::VertexBuffer)
    , cvbo(OpenGLBuffer::VertexBuffer)
{
    // Set up...
    this->offset = _offset;
    this->hg = _hg;
    this->data = _data;

    this->initialize();
}

HexGridVisual::~HexGridVisual()
{
    if (this->ivbo.isCreated()) {
        this->ivbo.destroy();
    }
    if (this->pvbo.isCreated()) {
        this->pvbo.destroy();
    }
    if (this->nvbo.isCreated()) {
        this->nvbo.destroy();
    }
    if (this->cvbo.isCreated()) {
        this->cvbo.destroy();
    }
}

void
HexGridVisual::initialize (void)
{
    // Simplest visualization is: for each vertex in hg, create 6
    // triangles, all with a common colour. This way, I don't have to
    // worry about the order of the Hexs.

    float sr = this->hg->getSR();
    float vne = this->hg->getVtoNE();
    float lr = this->hg->getLR();

    unsigned int nhex = this->hg->num();
    unsigned int idx = 0;

    for (unsigned int hi = 0; hi < nhex; ++hi) {
        // First push the 7 positions of the triangle vertices, starting with the centre
        this->vertex_push (this->hg->d_x[hi],
                           this->hg->d_y[hi],
                           this->data[hi], this->vertexPositions);
        // NE
        this->vertex_push (this->hg->d_x[hi]+sr,
                           this->hg->d_y[hi]+vne,
                           this->data[hi], this->vertexPositions);
        // SE
        this->vertex_push (this->hg->d_x[hi]+sr,
                           this->hg->d_y[hi]-vne,
                           this->data[hi], this->vertexPositions);
        // S
        this->vertex_push (this->hg->d_x[hi],
                           this->hg->d_y[hi]-lr,
                           this->data[hi], this->vertexPositions);
        // SW
        this->vertex_push (this->hg->d_x[hi]-sr,
                           this->hg->d_y[hi]-vne,
                           this->data[hi], this->vertexPositions);
        // NW
        this->vertex_push (this->hg->d_x[hi]-sr,
                           this->hg->d_y[hi]+vne,
                           this->data[hi], this->vertexPositions);
        // N
        this->vertex_push (this->hg->d_x[hi],
                           this->hg->d_y[hi]+lr,
                           this->data[hi], this->vertexPositions);

        // All normal point up
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);

        // All vertices black to begin with, but should set from data.
        this->vertex_push (0.0f, 0.0f, 0.0f, this->vertexColors); // Black
        this->vertex_push (0.0f, 0.0f, 0.0f, this->vertexColors);
        this->vertex_push (0.0f, 0.0f, 0.0f, this->vertexColors);
        this->vertex_push (0.0f, 0.0f, 0.0f, this->vertexColors);
        this->vertex_push (0.0f, 0.0f, 0.0f, this->vertexColors);
        this->vertex_push (0.0f, 0.0f, 0.0f, this->vertexColors);

        // Define indices now to produce the 6 triangles in the hex
        this->indices.push_back (idx+1);
        this->indices.push_back (idx);
        this->indices.push_back (idx+2);

        this->indices.push_back (idx+2);
        this->indices.push_back (idx);
        this->indices.push_back (idx+3);

        this->indices.push_back (idx+3);
        this->indices.push_back (idx);
        this->indices.push_back (idx+4);

        this->indices.push_back (idx+4);
        this->indices.push_back (idx);
        this->indices.push_back (idx+5);

        this->indices.push_back (idx+5);
        this->indices.push_back (idx);
        this->indices.push_back (idx+6);

        this->indices.push_back (idx+6);
        this->indices.push_back (idx);
        this->indices.push_back (idx+1);

        idx += 7;
    }
}

void
HexGridVisual::render (void)
{
    // render...
}

void
HexGridVisual::setupVBO (OpenGLBuffer& buf,
                         vector<float>& dat,
                         const char* arrayname)
{
    if (buf.create() == false) {
        cout << "VBO create failed" << endl;
    }
    buf.setUsagePattern (OpenGLBuffer::StaticDraw);
    if (buf.bind() == false) {
        cout << "VBO bind failed" << endl;
    }
    buf.allocate (dat.data(), dat.size() * sizeof(float));
    // Because array attributes are disabled by default in OpenGL 4:
    this->shaderProgram->enableAttributeArray (arrayname);
    this->shaderProgram->setAttributeBuffer (arrayname, GL_FLOAT, 0, 3);
}

void
HexGridVisual::vertex_push (const float& x, const float& y, const float& z, vector<float>& vp)
{
    vp.push_back (x);
    vp.push_back (y);
    vp.push_back (z);
}
