/*!
 * \file
 *
 * Defines a coordinate arrow class
 *
 * \author Seb James
 * \date 2019
 */
#pragma once

#include "morph/Vector.h"
#include "morph/VisualModel.h"
#include "morph/MathConst.h"
#include <array>

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
                    const Vector<float, 3> _offset,
                    const Vector<float, 3> _scale) {
            this->init (sp, _offset, _scale);
        }

        virtual ~CoordArrows () {}

        void init (GLuint sp,
                   const Vector<float, 3> _offset,
                   const Vector<float, 3> _scale) {
            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);

            this->scale = _scale;

            // Initialize the vertices that will represent the object
            this->initializeVertices();

            this->postVertexInit();
        }

        //! Make sure coord arrow colours are ok on the given background colour
        void setColourForBackground (const std::array<float, 4>& bgcolour) {
            // For now, only worry about the centresphere:
            std::array<float, 3> cscol = {1.0f-bgcolour[0],
                                          1.0f-bgcolour[1],
                                          1.0f-bgcolour[2]};
            if (cscol != this->centresphere_col) {
                this->centresphere_col = cscol;
                this->initializeVertices();
                this->postVertexInit();
            }
        }

        //! Initialize vertex buffer objects and vertex array object.
        //@{
        void initializeVertices (void) {

            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();

            // The indices index
            GLushort idx = 0;

            // Draw four spheres to make up the coord frame
            Vector<float, 3> reloffset = this->offset;
            this->computeSphere (idx, this->offset, centresphere_col, this->scale[0]/20.0);

            // x
            reloffset[0] += this->scale[0];
            this->computeSphere (idx, reloffset, x_axis_col, this->scale[0]/40.0);
            this->computeTube (idx, this->offset, reloffset, x_axis_col, x_axis_col, this->scale[0]/80.0);

            // y
            reloffset[0] -= this->scale[0];
            reloffset[1] += this->scale[1];
            this->computeSphere (idx, reloffset, y_axis_col, this->scale[0]/40.0);
            this->computeTube (idx, this->offset, reloffset, y_axis_col, y_axis_col, this->scale[0]/80.0);

            // z
            reloffset[1] -= this->scale[1];
            reloffset[2] += this->scale[2];
            this->computeSphere (idx, reloffset, z_axis_col, this->scale[0]/40.0);
            this->computeTube (idx, this->offset, reloffset, z_axis_col, z_axis_col, this->scale[0]/80.0);
        }

        //! The lengths of the x, y and z arrows.
        Vector<float, 3> scale;

        //! The colours of the arrows, and of the centre sphere
        std::array<float, 3> centresphere_col = {1.0f, 1.0f, 1.0f};
        std::array<float, 3> x_axis_col = {1.0f, 0.0f, 0.0f};
        std::array<float, 3> z_axis_col = {0.0f, 0.0f, 1.0f};
        std::array<float, 3> y_axis_col = {0.0f, 1.0f, 0.0f};
    };

} // namespace morph
