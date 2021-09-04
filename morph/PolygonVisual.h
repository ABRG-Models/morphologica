#pragma once

#include <morph/Vector.h>
#include <morph/VisualModel.h>
#include <morph/MathConst.h>
#include <array>

namespace morph {

    //! This class creates the vertices for a polygonal object in a 3D scene
    class PolygonVisual : public VisualModel
    {
    public:
        PolygonVisual (void) { this->mv_offset = {0.0, 0.0, 0.0}; }

        PolygonVisual(GLuint sp, const Vector<float, 3> _offset,
                      const Vector<float, 3> _position, const Vector<float, 3> _vertex,
                      const float _radius, const float _thickness,
                      const std::array<float, 3> _col, const int _n)
        {
            this->init (sp, _offset, _position, _vertex, _radius, _thickness, _col, _n);
        }

        virtual ~PolygonVisual () {}

        void init (GLuint sp, const Vector<float, 3> _offset,
                   const Vector<float, 3> _position, const Vector<float, 3> _vertex,
                   const float _radius, const float _thickness,
                   const std::array<float, 3> _col, const int _n)
        {
            // Set up...
            this->shaderprog = sp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);

            this->position = _position;
            this->vertex = _vertex;
            this->radius = _radius;
            this->thickness = _thickness;
            this->col = _col;
            this->n = _n;

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
            this->indices.clear();

            // The indices index
            VBOint idx = 0;

            // Always draw a full 3D polygon
            morph::Vector<float> pend = this->position;
            pend[2] += this->thickness;
            // Figure out ux, uy from position and vertex. Let ux be like dirn to vertex
            this->ux = this->vertex - this->position;
            this->uy = this->ux.cross(this->uz);
            this->computeTube (idx, this->position, pend, this->ux, this->uy,
                               this->col, this->col,
                               this->radius, this->n);
        }

        //! The position of the start of the rod, given with respect to the parent's offset
        Vector<float, 3> position = {0.0f, 0.0f, 0.0f};
        //! Direction to the first vertex.
        Vector<float, 3> vertex = {1.0f, 0.0f, 0.0f};
        //! The radius of the polygonal puck's enclosing circle
        float radius = 1.0f;
        //! The thickness of the polygonal puck
        float thickness = 0.01f;
        //! Number of segments in this polygon
        int n = 4;

        // Some axes
        morph::Vector<float> ux = {1,0,0};
        morph::Vector<float> uy = {0,1,0};
        morph::Vector<float> uz = {0,0,1};

        //! The colour of the thing.
        std::array<float, 3> col = {1.0f, 0.0f, 0.0f};
    };

} // namespace morph
