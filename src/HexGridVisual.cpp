#include "HexGridVisual.h"

#include "GL3/gl3.h"
#include "GL/glext.h"

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;
#include <array>
using std::array;

morph::HexGridVisual::HexGridVisual (const Visual* _parent,
                                     const HexGrid* _hg,
                                     const vector<float>* _data,
                                     const array<float, 3> _offset)
{
    // Set up...
    this->parent = _parent;
    this->offset = _offset;
    this->hg = _hg;
    this->data = _data;

    // Allocate the vertex buffer object array and init the unsigned
    // int `names' to 1,2,3,4
    this->bufobjs = new GLuint[4];
    this->vaos = new GLuint[1];

    this->initializeVertices();

    glCreateVertexArrays (1, this->vaos); // OpenGL 4.5+
    cout << "vertex array name/handle: " << this->vaos[0] << endl;

#if 0 // Probably need this:
    // Enable my attributes
    glEnableVertexArrayAttrib (this->vaos[0], loc_attrib);
    glEnableVertexArrayAttrib (this->vaos[0], normal_attrib);
    glEnableVertexArrayAttrib (this->vaos[0], texcoord_attrib);
#endif

    // Check for GL_INVALID_VALUE?
    glBindVertexArray (this->vaos[0]);
    // Check for GL_INVALID_VALUE?

    // OpenGL 4.5 and lower:
    glGenBuffers (4, this->bufobjs);
    // OpenGL 4.5 only:
    //glCreateBuffers (4, this->bufobjs);
    // Verify buffer names:
    for (unsigned int ii = 0; ii < 4; ++ii) {
        cout << "buffer " << ii << ": " << this->bufobjs[ii] << endl;
    }
    // Check for GL_INVALID_VALUE?

    // Set up indices buffer object (bind, then allocate space)
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->bufobjs[0]); // For index buffer
    int sz = this->indices.size() * sizeof(VBOint);
    cout << "After binding indices buff, set buffer data for " << sz << " indices" << endl;
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);

    // Is this needed for indices?
    glVertexAttribPointer (vPosition, 1, VBO_ENUM_TYPE, GL_FALSE, 0, (void*)0); // VBO_ENUM_TYPE is GL_UNSIGNED_INT
    glEnableVertexAttribArray (vPosition);
    GLuint arrayEnabled = GL_FALSE;
    glGetVertexAttribIuiv (vPosition, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &arrayEnabled);
    cout << "arrayEnabled: " << arrayEnabled << endl;

    // Binds data from the "C++ world" to the OpenGL shader world for
    // "position", "normalin" and "color"
    this->setupVBO (this->bufobjs[1], this->vertexPositions, "position");
    this->setupVBO (this->bufobjs[2], this->vertexNormals, "normalin");
    this->setupVBO (this->bufobjs[3], this->vertexColors, "color");

    // Possibly release (unbind) the vertex buffers (but not index buffer)
    // Possible glVertexAttribPointer and glEnableVertexAttribArray?
}

void
morph::HexGridVisual::setupVBO (GLuint& buf,
                                vector<float>& dat,
                                const char* arrayname)
{
    glBindBuffer (GL_ARRAY_BUFFER, buf);
    //if (checkBound == false) {
    //    cout << "VBO bind failed" << endl;
    //}
    int sz = (*this->data).size() * 3 * sizeof(float); // *3 to include position, colour & normal? Or is that ALREADY factored into the size of dat?
    glBufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
    // Something like:
    glVertexAttribPointer (vPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray (vPosition);
}

morph::HexGridVisual::~HexGridVisual()
{
    // destroy buffers
    glDeleteBuffers (4, bufobjs);
    delete (this->bufobjs);
    delete (this->vaos);
}

void
morph::HexGridVisual::initializeVertices (void)
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
        cout << "Hex " << hi << endl;
        // First push the 7 positions of the triangle vertices, starting with the centre
        this->vertex_push (this->hg->d_x[hi],
                           this->hg->d_y[hi],
                           (*this->data)[hi], this->vertexPositions);
        cout << "centre vertex " << this->hg->d_x[hi] << "," << this->hg->d_y[hi] << "," << (*this->data)[hi] << endl;
        // NE
        this->vertex_push (this->hg->d_x[hi]+sr,
                           this->hg->d_y[hi]+vne,
                           (*this->data)[hi], this->vertexPositions);
        cout << "NE vertex " << this->hg->d_x[hi]+sr << "," << this->hg->d_y[hi]+vne << "," << (*this->data)[hi] << endl;

        // SE
        this->vertex_push (this->hg->d_x[hi]+sr,
                           this->hg->d_y[hi]-vne,
                           (*this->data)[hi], this->vertexPositions);
        // S
        this->vertex_push (this->hg->d_x[hi],
                           this->hg->d_y[hi]-lr,
                           (*this->data)[hi], this->vertexPositions);
        // SW
        this->vertex_push (this->hg->d_x[hi]-sr,
                           this->hg->d_y[hi]-vne,
                           (*this->data)[hi], this->vertexPositions);
        // NW
        this->vertex_push (this->hg->d_x[hi]-sr,
                           this->hg->d_y[hi]+vne,
                           (*this->data)[hi], this->vertexPositions);
        // N
        this->vertex_push (this->hg->d_x[hi],
                           this->hg->d_y[hi]+lr,
                           (*this->data)[hi], this->vertexPositions);

        // All normal point up
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);

        // All vertices same colour value to begin with, but should set from data.
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexColors);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexColors);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexColors);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexColors);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexColors);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexColors);

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

        cout << "vertexPositions size: " << vertexPositions.size() << endl;
        cout << "vertexNormals size: " << vertexNormals.size() << endl;
        cout << "vertexColors size: " << vertexPositions.size() << endl;
        cout << "indices size: " << indices.size() << endl;
    }
    cout << "At END" << endl;
    cout << "vertexPositions size: " << vertexPositions.size() << endl;
    cout << "vertexNormals size: " << vertexNormals.size() << endl;
    cout << "vertexColors size: " << vertexPositions.size() << endl;
    cout << "indices size: " << indices.size() << endl;
}

void
morph::HexGridVisual::render (void)
{
    glBindVertexArray (this->vaos[0]);
    glDrawElements (GL_TRIANGLES, this->indices.size(), VBO_ENUM_TYPE, 0);
    // glBindVertexArray(0);
}

void
morph::HexGridVisual::vertex_push (const float& x, const float& y, const float& z, vector<float>& vp)
{
    vp.push_back (x);
    vp.push_back (y);
    vp.push_back (z);
}
