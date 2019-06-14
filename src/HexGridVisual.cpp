#include "HexGridVisual.h"

#include "GL3/gl3.h"
#include "GL/glext.h"

#include "tools.h"

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;
#include <array>
using std::array;

morph::HexGridVisual::HexGridVisual (GLuint sp,
                                     const HexGrid* _hg,
                                     const vector<float>* _data,
                                     const array<float, 3> _offset)
{
    // Set up...
    this->shaderprog = sp;
    this->offset = _offset;
    this->hg = _hg;
    this->data = _data;

    this->initializeVertices();

    // Allocate the vertex buffer object array and init the unsigned
    // int `names' to 1,2,3,4
    this->vbos = new GLuint[numVBO];

    glCreateVertexArrays (1, &this->vao); // OpenGL 4.5+

    glBindVertexArray (this->vao);

    glCreateBuffers (numVBO, this->vbos);

    // Element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[idxVBO]);
    int sz = this->indices.size() * sizeof(VBOint);
    cout << "indices sz = " << sz << " bytes" << endl;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);

    // Binds data from the "C++ world" to the OpenGL shader world for
    // "position", "normalin" and "color"
    this->setupVBO (this->vbos[posnVBO], this->vertexPositions, posnLoc);
    this->setupVBO (this->vbos[normVBO], this->vertexNormals, normLoc);
    this->setupVBO (this->vbos[colVBO], this->vertexColors, colLoc);

    // Possibly release (unbind) the vertex buffers (but not index buffer)
    // Possible glVertexAttribPointer and glEnableVertexAttribArray?
    glUseProgram (shaderprog);

    //glBindVertexArray(0);
}

void
morph::HexGridVisual::setupVBO (GLuint& buf,
                                vector<float>& dat,
                                unsigned int bufferAttribPosition)
{
    // I use the data array to determine the size of each vertex
    // buffer object. Each vbo is 3 times the size of this->data,
    // because each vbo contains 3 coordinates per element in
    // this->data. sz could be a member variable.
    int sz = dat.size() * sizeof(float);
    cout << "data sz = " << sz << " bytes" << endl;

    glBindBuffer (GL_ARRAY_BUFFER, buf);
    glBufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
    glVertexAttribPointer (bufferAttribPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
    cout << "EnableVertexAttribArray at position " << bufferAttribPosition << endl;
    glEnableVertexAttribArray (bufferAttribPosition);
}

morph::HexGridVisual::~HexGridVisual()
{
    // destroy buffers
    glDeleteBuffers (4, vbos);
    delete (this->vbos);
}

#ifdef INITIALISE_TRIANGLE_GRID
//writeme
#else // This is probably compute intensive:
void
morph::HexGridVisual::initializeVertices (void)
{
    // Simplest visualization is: for each vertex in hg, create 6
    // triangles, all with a common colour. This way, I don't have to
    // worry about the order of the Hexes.

    float sr = this->hg->getSR();
    float vne = this->hg->getVtoNE();
    float lr = this->hg->getLR();

    unsigned int nhex = this->hg->num();
    unsigned int idx = 0;

    float datum = 0.0f;
    for (unsigned int hi = 0; hi < nhex; ++hi) {
        cout << "Hex " << hi << endl;
        // First push the 7 positions of the triangle vertices, starting with the centre
        this->vertex_push (this->hg->d_x[hi],
                           this->hg->d_y[hi],
                           (*this->data)[hi], this->vertexPositions);
        cout << "centre vertex " << this->hg->d_x[hi] << "," << this->hg->d_y[hi] << "," << (*this->data)[hi] << endl;

        if (HAS_NNE(hi) && HAS_NE(hi)) {
            datum = (1/3.0f) * ((*this->data)[hi] + (*this->data)[NNE(hi)] + (*this->data)[NE(hi)]);
        } else {
            datum = (*this->data)[hi];
        }
        this->vertex_push (this->hg->d_x[hi]+sr,
                           this->hg->d_y[hi]+vne,
                           datum, // mean of this->data[hi] and NE and E hexes
                           this->vertexPositions);
        this->vertex_push (morph::Tools::getJetColorF((double)datum), this->vertexColors);
        cout << "NE vertex " << this->hg->d_x[hi]+sr << "," << this->hg->d_y[hi]+vne << "," << (*this->data)[hi] << endl;

        // SE
        if (HAS_NE(hi) && HAS_NSE(hi)) {
            datum = (1/3.0f) * ((*this->data)[hi] + (*this->data)[NE(hi)] + (*this->data)[NSE(hi)]);
        } else {
            datum = (*this->data)[hi];
        }
        this->vertex_push (this->hg->d_x[hi]+sr,
                           this->hg->d_y[hi]-vne,
                           datum, this->vertexPositions);
        this->vertex_push (morph::Tools::getJetColorF((double)datum), this->vertexColors);

        // S
        if (HAS_NSE(hi) && HAS_NSW(hi)) {
            datum = (1/3.0f) * ((*this->data)[hi] + (*this->data)[NSE(hi)] + (*this->data)[NSW(hi)]);
        } else {
            datum = (*this->data)[hi];
        }
        this->vertex_push (this->hg->d_x[hi],
                           this->hg->d_y[hi]-lr,
                           datum, this->vertexPositions);
        this->vertex_push (morph::Tools::getJetColorF((double)datum), this->vertexColors);

        // SW
        if (HAS_NW(hi) && HAS_NSW(hi)) {
            datum = (1/3.0f) * ((*this->data)[hi] + (*this->data)[NW(hi)] + (*this->data)[NSW(hi)]);
        } else {
            datum = (*this->data)[hi];
        }
        this->vertex_push (this->hg->d_x[hi]-sr,
                           this->hg->d_y[hi]-vne,
                           datum, this->vertexPositions);
        this->vertex_push (morph::Tools::getJetColorF((double)datum), this->vertexColors);

        // NW
        if (HAS_NNW(hi) && HAS_NW(hi)) {
            datum = (1/3.0f) * ((*this->data)[hi] + (*this->data)[NNW(hi)] + (*this->data)[NW(hi)]);
        } else {
            datum = (*this->data)[hi];
        }
        this->vertex_push (this->hg->d_x[hi]-sr,
                           this->hg->d_y[hi]+vne,
                           datum, this->vertexPositions);
        this->vertex_push (morph::Tools::getJetColorF((double)datum), this->vertexColors);

        // N
        if (HAS_NNW(hi) && HAS_NNE(hi)) {
            datum = (1/3.0f) * ((*this->data)[hi] + (*this->data)[NNW(hi)] + (*this->data)[NNE(hi)]);
        } else {
            datum = (*this->data)[hi];
        }
        this->vertex_push (this->hg->d_x[hi],
                           this->hg->d_y[hi]+lr,
                           datum, this->vertexPositions);
        this->vertex_push (morph::Tools::getJetColorF((double)datum), this->vertexColors);

        // All normal point up
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);

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

        idx += 7; // 7 vertices (each of 3 floats for x/y/z), 18 indices.

#if 0
        cout << "vertexPositions size: " << vertexPositions.size() << endl;
        cout << "vertexNormals size: " << vertexNormals.size() << endl;
        cout << "vertexColors size: " << vertexPositions.size() << endl;
        cout << "indices size: " << indices.size() << " elements" << endl;
#endif
    }
    cout << "vertexPositions size: " << vertexPositions.size() << " elements" << endl;
    cout << "vertexNormals size: " << vertexNormals.size() << " elements" << endl;
    cout << "vertexColors size: " << vertexPositions.size() << " elements" << endl;
    cout << "indices size: " << indices.size() << " elements" << endl;
}
#endif

void
morph::HexGridVisual::render (void)
{
    glBindVertexArray (this->vao);
    cout << "Render " << this->indices.size() << " vertices" << endl;
    glDrawElements (GL_TRIANGLES, this->indices.size(), VBO_ENUM_TYPE, 0);
    glBindVertexArray(0);
}

void
morph::HexGridVisual::vertex_push (const float& x, const float& y, const float& z, vector<float>& vp)
{
    vp.push_back (x);
    vp.push_back (y);
    vp.push_back (z);
}

void
morph::HexGridVisual::vertex_push (const array<float, 3>& arr, vector<float>& vp)
{
    vp.push_back (arr[0]);
    vp.push_back (arr[1]);
    vp.push_back (arr[2]);
}
