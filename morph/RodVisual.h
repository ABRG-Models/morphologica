#pragma once

#include <morph/Vector.h>
#include <morph/VisualModel.h>
#include <morph/MathConst.h>
#include <array>

namespace morph {

    //! This class creates the vertices for a cylindrical 'rod' in a 3D scene.
    class RodVisual : public VisualModel
    {
    public:
        RodVisual (void) { this->mv_offset = {0.0, 0.0, 0.0}; }

        //! Initialise with offset, start and end coordinates, radius and a single colour.
        RodVisual(GLuint sp, const Vector<float, 3> _offset,
                  const Vector<float, 3> _start_coord, const Vector<float, 3> _end_coord, const float _radius,
                  const std::array<float, 3> _col)
        {
            this->init (sp, _offset, _start_coord, _end_coord, _radius, _col, _col);
        }

        //! Initialise with offset, start and end coordinates, radius and start and end colours.
        RodVisual(GLuint sp, const Vector<float, 3> _offset,
                  const Vector<float, 3> _start_coord, const Vector<float, 3> _end_coord, const float _radius,
                  const std::array<float, 3> _start_col, const std::array<float, 3> _end_col)
        {
            this->init (sp, _offset, _start_coord, _end_coord, _radius, _start_col, _end_col);
        }

        virtual ~RodVisual ()
        {
            std::cout << "~RodVisual()\n";
        }

        void init (GLuint sp, const Vector<float, 3> _offset,
                   const Vector<float, 3> _start_coord, const Vector<float, 3> _end_coord, const float _radius,
                   const std::array<float, 3> _start_col, const std::array<float, 3> _end_col)
        {
            // Set up...
            this->shaderprog = sp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);

            this->start_coord = _start_coord;
            this->end_coord = _end_coord;
            this->radius = _radius;
            this->start_col = _start_col;
            this->end_col = _end_col;

            // Initialize the vertices that will represent the object
            this->initializeVertices();

            this->postVertexInit();
        }

        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices (void)
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();

            // The indices index
            VBOint idx = 0;
            // Draw a tube. That's it!
#if 1
            this->computeTube (idx, this->mv_offset+this->start_coord, this->mv_offset+this->end_coord,
                               this->start_col, this->end_col, this->radius, 12);
#else
            // Can alternatively use the 'oriented' tube
            this->computeTube (idx, this->mv_offset+this->start_coord, this->mv_offset+this->end_coord,
                               {0,1,0}, {0,0,1},
                               this->start_col, this->end_col, this->radius, 6, morph::PI_F/6.0f);
#endif
        }

        //! The position of the start of the rod, given with respect to the parent's offset
        Vector<float, 3> start_coord = {0.0f, 0.0f, 0.0f};
        //! The position of the end of the rod, given with respect to the parent's offset
        Vector<float, 3> end_coord = {1.0f, 0.0f, 0.0f};
        //! The radius of the rod
        float radius = 1.0f;

        //! The colours of the rod.
        std::array<float, 3> start_col = {1.0f, 0.0f, 0.0f};
        std::array<float, 3> end_col = {0.0f, 0.0f, 1.0f};
    };

} // namespace morph
