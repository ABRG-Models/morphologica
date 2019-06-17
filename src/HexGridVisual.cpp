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

    // Do the computations to initialize the vertices that well
    // represent the HexGrid.
    this->initializeVerticesHexesInterpolated();
    //this->initializeVerticesTris();

    // Create vertex array object
    glCreateVertexArrays (1, &this->vao);
    glBindVertexArray (this->vao);

    // Allocate/create the vertex buffer objects
    this->vbos = new GLuint[numVBO];
    glCreateBuffers (numVBO, this->vbos);

    // Set up the indices buffer
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

    // Here's how to unbind the VAO
    //glBindVertexArray(0);
}

void
morph::HexGridVisual::setupVBO (GLuint& buf,
                                vector<float>& dat,
                                unsigned int bufferAttribPosition)
{
    int sz = dat.size() * sizeof(float);
    glBindBuffer (GL_ARRAY_BUFFER, buf);
    glBufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
    glVertexAttribPointer (bufferAttribPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
    glEnableVertexAttribArray (bufferAttribPosition);
}

morph::HexGridVisual::~HexGridVisual()
{
    // destroy buffers
    glDeleteBuffers (4, vbos);
    delete (this->vbos);
}

void
morph::HexGridVisual::initializeVerticesTris (void)
{
    unsigned int nhex = this->hg->num();
    for (unsigned int hi = 0; hi < nhex; ++hi) {
        this->vertex_push (this->hg->d_x[hi], this->hg->d_y[hi], (*this->data)[hi], this->vertexPositions);
        this->vertex_push (morph::Tools::getJetColorF((double)(*this->data)[hi]+0.5), this->vertexColors);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
    }

    // Build indices based on neighbour relations in the HexGrid
    for (unsigned int hi = 0; hi < nhex; ++hi) {
        if (HAS_NNE(hi) && HAS_NE(hi)) {
            cout << "1st triangle " << hi << "->" << NNE(hi) << "->" << NE(hi) << endl;
            this->indices.push_back (hi);
            this->indices.push_back (NNE(hi));
            this->indices.push_back (NE(hi));
        }

        if (HAS_NW(hi) && HAS_NSW(hi)) {
            cout << "2nd triangle " << hi << "->" << NW(hi) << "->" << NSW(hi) << endl;
            this->indices.push_back (hi);
            this->indices.push_back (NW(hi));
            this->indices.push_back (NSW(hi));
        }
    }
}

void
morph::HexGridVisual::initializeVerticesHexesStepped (void)
{
}

void
morph::HexGridVisual::initializeVerticesHexesInterpolated (void)
{
    float sr = this->hg->getSR();
    float vne = this->hg->getVtoNE();
    float lr = this->hg->getLR();

    unsigned int nhex = this->hg->num();
    unsigned int idx = 0;

    float datum = 0.0f;
    float third = 0.33333333333333f;
    float half = 0.5f;
    for (unsigned int hi = 0; hi < nhex; ++hi) {

        // Use a single colour for each hex, even though hex z positions are interpolated
        array<float, 3> clr = morph::Tools::getJetColorF((double)(*this->data)[hi]+0.5);

        // First push the 7 positions of the triangle vertices, starting with the centre
        this->vertex_push (this->hg->d_x[hi], this->hg->d_y[hi], (*this->data)[hi], this->vertexPositions);

        if (HAS_NNE(hi) && HAS_NE(hi)) {
            // Compute mean of this->data[hi] and NE and E hexes
            datum = third * ((*this->data)[hi] + (*this->data)[NNE(hi)] + (*this->data)[NE(hi)]);
        } else if (HAS_NNE(hi) || HAS_NE(hi)) {
            if (HAS_NNE(hi)) {
                datum = half * ((*this->data)[hi] + (*this->data)[NNE(hi)]);
            } else {
                datum = half * ((*this->data)[hi] + (*this->data)[NE(hi)]);
            }
        } else {
            datum = (*this->data)[hi];
        }
        this->vertex_push (this->hg->d_x[hi]+sr, this->hg->d_y[hi]+vne, datum, this->vertexPositions);

        // SE
        if (HAS_NE(hi) && HAS_NSE(hi)) {
            datum = third * ((*this->data)[hi] + (*this->data)[NE(hi)] + (*this->data)[NSE(hi)]);
        } else if (HAS_NE(hi) || HAS_NSE(hi)) {
            if (HAS_NE(hi)) {
                datum = half * ((*this->data)[hi] + (*this->data)[NE(hi)]);
            } else {
                datum = half * ((*this->data)[hi] + (*this->data)[NSE(hi)]);
            }
        } else {
            datum = (*this->data)[hi];
        }
        this->vertex_push (this->hg->d_x[hi]+sr, this->hg->d_y[hi]-vne, datum, this->vertexPositions);

        // S
        if (HAS_NSE(hi) && HAS_NSW(hi)) {
            datum = third * ((*this->data)[hi] + (*this->data)[NSE(hi)] + (*this->data)[NSW(hi)]);
        } else if (HAS_NSE(hi) || HAS_NSW(hi)) {
            if (HAS_NSE(hi)) {
                datum = half * ((*this->data)[hi] + (*this->data)[NSE(hi)]);
            } else {
                datum = half * ((*this->data)[hi] + (*this->data)[NSW(hi)]);
            }
        } else {
            datum = (*this->data)[hi];
        }
        this->vertex_push (this->hg->d_x[hi], this->hg->d_y[hi]-lr, datum, this->vertexPositions);

        // SW
        if (HAS_NW(hi) && HAS_NSW(hi)) {
            datum = third * ((*this->data)[hi] + (*this->data)[NW(hi)] + (*this->data)[NSW(hi)]);
        } else if (HAS_NW(hi) || HAS_NSW(hi)) {
            if (HAS_NW(hi)) {
                datum = half * ((*this->data)[hi] + (*this->data)[NW(hi)]);
            } else {
                datum = half * ((*this->data)[hi] + (*this->data)[NSW(hi)]);
            }
        } else {
            datum = (*this->data)[hi];
        }
        this->vertex_push (this->hg->d_x[hi]-sr, this->hg->d_y[hi]-vne, datum, this->vertexPositions);

        // NW
        if (HAS_NNW(hi) && HAS_NW(hi)) {
            datum = third * ((*this->data)[hi] + (*this->data)[NNW(hi)] + (*this->data)[NW(hi)]);
        } else if (HAS_NNW(hi) || HAS_NW(hi)) {
            if (HAS_NNW(hi)) {
                datum = half * ((*this->data)[hi] + (*this->data)[NNW(hi)]);
            } else {
                datum = half * ((*this->data)[hi] + (*this->data)[NW(hi)]);
            }
        } else {
            datum = (*this->data)[hi];
        }
        this->vertex_push (this->hg->d_x[hi]-sr, this->hg->d_y[hi]+vne, datum, this->vertexPositions);

        // N
        if (HAS_NNW(hi) && HAS_NNE(hi)) {
            datum = third * ((*this->data)[hi] + (*this->data)[NNW(hi)] + (*this->data)[NNE(hi)]);
        } else if (HAS_NNW(hi) || HAS_NNE(hi)) {
            if (HAS_NNW(hi)) {
                datum = half * ((*this->data)[hi] + (*this->data)[NNW(hi)]);
            } else {
                datum = half * ((*this->data)[hi] + (*this->data)[NNE(hi)]);
            }
        } else {
            datum = (*this->data)[hi];
        }
        this->vertex_push (this->hg->d_x[hi], this->hg->d_y[hi]+lr, datum, this->vertexPositions);

        // All normal point up
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);

        // Seven vertices with the same colour
        this->vertex_push (clr, this->vertexColors);
        this->vertex_push (clr, this->vertexColors);
        this->vertex_push (clr, this->vertexColors);
        this->vertex_push (clr, this->vertexColors);
        this->vertex_push (clr, this->vertexColors);
        this->vertex_push (clr, this->vertexColors);
        this->vertex_push (clr, this->vertexColors);

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

void
morph::HexGridVisual::render (void)
{
    glBindVertexArray (this->vao);
    //cout << "Render " << this->indices.size() << " vertices" << endl;
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
