#ifndef _COORDARROWS_H_
#define _COORDARROWS_H_

#include "VisualModel.h"

#include "MathConst.h"

#include <iostream>
using std::cout;
using std::endl;

#include <array>
using std::array;

#if 0
// Add +, - operators to std::array
template<class T, size_t N>
class Array : public array<T, N>
{
public:
    using array<T, N>::array;
    Array operator+(Array const& rhs) const {
        Array res;
        transform (this->begin(), this->end(), rhs.begin(), res.begin(), std::plus);
        return res;
    }
    Array operator-(Array const& rhs) const {
        Array res;
        transform (this->begin(), this->end(), rhs.begin(), res.begin(), std::minus);
        return res;
    }
};
#endif

namespace morph {

    //! This class creates the vertices for a set of coordinate arrows to be rendered
    //! in a 3-D scene.
    class CoordArrows : public VisualModel
    {
    public:
        CoordArrows (void) {
            this->scale = {1.0, 1.0, 1.0};
            this->offset = {0.0, 0.0, 0.0};
        }

        CoordArrows(GLuint sp,
                    const array<float, 3> _offset,
                    const array<float, 3> _scale) {
            this->init (sp, _offset, _scale);
        }

        virtual ~CoordArrows () {}

        void init (GLuint sp,
                   const array<float, 3> _offset,
                   const array<float, 3> _scale) {
            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);

            this->scale = _scale;

            // Do the computations to initialize the vertices that well
            // represent the HexGrid.
            this->initializeVertices();

            this->postVertexInit();
        }

        //! Initialize vertex buffer objects and vertex array object.
        //@{
        //! Initialize as triangled. Gives a smooth surface with much
        //! less comput than initializeVerticesHexesInterpolated.
        void initializeVertices (void) {

            // The indices index
            GLushort idx = 0;

            array<float, 3> white = {1.0,1.0,1.0};
            array<float, 3> red = {1.0,0.0,0.0};
            array<float, 3> blue = {0.0,0.0,1.0};
            array<float, 3> green = {0.0,1.0,0.0};

            // Draw four spheres to make up the coord frame
            array<float, 3> reloffset = this->offset;
            this->computeSphere (idx, this->offset, white, this->scale[0]/20.0);

            // x
            reloffset[0] += this->scale[0];
            this->computeSphere (idx, reloffset, red, this->scale[0]/40.0);
            this->computeTube (idx, this->offset, reloffset, red, red, this->scale[0]/80.0);

            // y
            reloffset[0] -= this->scale[0];
            reloffset[1] += this->scale[1];
            this->computeSphere (idx, reloffset, green, this->scale[0]/40.0);
            this->computeTube (idx, this->offset, reloffset, green, green, this->scale[0]/80.0);

            // z
            reloffset[1] -= this->scale[1];
            reloffset[2] += this->scale[2];
            this->computeSphere (idx, reloffset, blue, this->scale[0]/40.0);
            this->computeTube (idx, this->offset, reloffset, blue, blue, this->scale[0]/80.0);
        }

        //! The lengths of the x, y and z arrows.
        array<float, 3> scale;
    };

} // namespace morph

#endif // _COORDARROWS_H_
