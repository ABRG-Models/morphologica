#pragma once

#include <morph/vec.h>
#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <array>

namespace morph {

    //! This class creates the vertices for a polygonal object in a 3D scene
    class PolygonVisual : public VisualModel
    {
    public:
        PolygonVisual() { this->mv_offset = {0.0, 0.0, 0.0}; }

        PolygonVisual(const vec<float, 3> _offset,
                      const vec<float, 3> _position, const vec<float, 3> _vertex,
                      const float _radius, const float _thickness,
                      const std::array<float, 3> _col, const int _n)
        {
            this->init (_offset, _position, _vertex, _radius, _thickness, _col, _n);
        }

        virtual ~PolygonVisual () {}

        void init (const vec<float, 3> _offset,
                   const vec<float, 3> _position, const vec<float, 3> _vertex,
                   const float _radius, const float _thickness,
                   const std::array<float, 3> _col, const int _n)
        {
            // Set up...
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);

            this->position = _position;
            this->vertex = _vertex;
            this->radius = _radius;
            this->thickness = _thickness;
            this->col = _col;
            this->n = _n;
        }

        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();

            // The indices index
            VBOint idx = 0;

            // Always draw a full 3D polygon
            morph::vec<float> pend = this->position;
            pend[2] += this->thickness;
            // Figure out ux, uy from position and vertex. Let ux be like dirn to vertex
            this->_ux = this->vertex - this->position;
            this->_uy = this->_ux.cross(this->uz);
            this->computeTube (idx, this->position, pend, this->_ux, this->_uy,
                               this->col, this->col,
                               this->radius, this->n);
        }

        //! The position of the start of the rod, given with respect to the parent's offset
        vec<float, 3> position = {0.0f, 0.0f, 0.0f};
        //! Direction to the first vertex.
        vec<float, 3> vertex = {1.0f, 0.0f, 0.0f};
        //! The radius of the polygonal puck's enclosing circle
        float radius = 1.0f;
        //! The thickness of the polygonal puck
        float thickness = 0.01f;
        //! Number of segments in this polygon
        int n = 4;

        // Some axes
        morph::vec<float> _ux = {1,0,0};
        morph::vec<float> _uy = {0,1,0};

        //! The colour of the thing.
        std::array<float, 3> col = {1.0f, 0.0f, 0.0f};
    };

} // namespace morph
