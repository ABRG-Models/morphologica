#pragma once

#include <morph/vec.h>
#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <array>

namespace morph {

    //! This class creates the vertices for a rhombohedron
    class RhomboVisual : public VisualModel
    {
    public:
        //! Initialise with offset, three edges and a single colour.
        RhomboVisual(morph::gl::shaderprogs& sp, const vec<float, 3> _offset,
                     const vec<float, 3> _edge1, const vec<float, 3> _edge2, const vec<float, 3> _edge3,
                     const std::array<float, 3> _col)
            : VisualModel(sp, _offset)
            , edge1 (_edge1) , edge2 (_edge2) , edge3 (_edge3), col (_col) {}

        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices() override
        {
            // Compute the face normals
            vec<float> _n1 = this->edge1.cross (this->edge2);
            _n1.renormalize();
            vec<float> _n2 = this->edge2.cross (this->edge3);
            _n2.renormalize();
            vec<float> _n3 = this->edge1.cross (this->edge3);
            _n3.renormalize();

            // First corner of rhombohedron is at model-frame's origin
            vec<float> o = {0,0,0};

            // Push positions and normals for 24 vertices to make up the rhombohedron; 4 for each face.
            // Front face
            this->vertex_push (o,                              this->vertexPositions);
            this->vertex_push (o + this->edge1,                this->vertexPositions);
            this->vertex_push (o + this->edge3,                this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge3,  this->vertexPositions);
            for (size_t i = 0; i < 4; ++i) { this->vertex_push (_n3, this->vertexNormals); }
            // Top face
            this->vertex_push (o + this->edge3, this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge3,               this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge3,               this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge1 + this->edge3, this->vertexPositions);
            for (size_t i = 0; i < 4; ++i) { this->vertex_push (_n1, this->vertexNormals); }
            // Back face
            this->vertex_push (o + this->edge2 + this->edge3,               this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge1 + this->edge3, this->vertexPositions);
            this->vertex_push (o + this->edge2,                             this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge1,               this->vertexPositions);
            for (size_t i = 0; i < 4; ++i) { this->vertex_push (-_n3, this->vertexNormals); }
            // Bottom face
            this->vertex_push (o + this->edge2,                this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge1,  this->vertexPositions);
            this->vertex_push (o,                              this->vertexPositions);
            this->vertex_push (o + this->edge1,                this->vertexPositions);
            for (size_t i = 0; i < 4; ++i) { this->vertex_push (-_n1, this->vertexNormals); }
            // Left face
            this->vertex_push (o + this->edge2,                this->vertexPositions);
            this->vertex_push (o,                              this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge3,  this->vertexPositions);
            this->vertex_push (o + this->edge3,                this->vertexPositions);
            for (size_t i = 0; i < 4; ++i) { this->vertex_push (-_n2, this->vertexNormals); }
            // Right face
            this->vertex_push (o + this->edge1,                             this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge2,               this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge3,               this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge2 + this->edge3, this->vertexPositions);
            for (size_t i = 0; i < 4; ++i) { this->vertex_push (_n2, this->vertexNormals); }

            // Vertex colours are all the same
            for (size_t i = 0; i < 24; ++i) {
                this->vertex_push (this->col, this->vertexColors);
            }

            // Indices for 6 faces
            for (size_t i = 0; i < 6; ++i) {
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx--);
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx++);
            }
        }

        //! Three vectors define the Rhombohedron and we use a single colour
        vec<float, 3> edge1 = {0.0f, 0.0f, 0.0f};
        vec<float, 3> edge2 = {0.0f, 0.0f, 0.0f};
        vec<float, 3> edge3 = {0.0f, 0.0f, 0.0f};
        std::array<float, 3> col = {0.0f, 0.0f, 1.0f};
    };

} // namespace morph
