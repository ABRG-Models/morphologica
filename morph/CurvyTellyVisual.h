#pragma once

#include <array>
#include <morph/mathconst.h>
#include <morph/vec.h>
#include <morph/CartGrid.h>
#include <morph/CartGridVisual.h>

namespace morph {

    // Draw a curved CartGrid like a curved TV. You make a cylinder if you make the rotation right.
    template <typename T>
    struct CurvyTellyVisual : public morph::CartGridVisual<T>
    {
        using mc = morph::mathconst<float>;

        // The radius of the curved surface representing the CartGrid
        T radius = T{1};
        // What angle around the circle to draw the telly. 2pi gives a cylinder and is the default.
        T angle_to_subtend = morph::mathconst<T>::two_pi;
        // rotational offset in radians for the rendering. This allows you to arrange the 'centre' of the telly.
        float rotoff = 0.0f;

        // Note constructor forces centralize to be true, which is important when drawing a curvy CartGrid
        CurvyTellyVisual(morph::gl::shaderprogs& _shaders, const morph::CartGrid* _cg, const morph::vec<float> _offset)
            : morph::CartGridVisual<T>(_shaders, _cg, _offset) { this->centralize = true; }

        void drawcurvygrid()
        {
            float dx = this->cg->getd();
            float hx = 0.5f * dx;
            float dy = this->cg->getv();
            float vy = 0.5f * dy;

            unsigned int nrect = this->cg->num();
            this->idx = 0;

            if (this->scalarData != nullptr) {
                this->dcopy.resize (this->scalarData->size());
                this->zScale.transform (*(this->scalarData), this->dcopy);
                this->dcolour.resize (this->scalarData->size());
                this->colourScale.transform (*(this->scalarData), this->dcolour);
            } else if (this->vectorData != nullptr) {
                this->dcopy.resize (this->vectorData->size());
                this->dcolour.resize (this->vectorData->size());
                this->dcolour2.resize (this->vectorData->size());
                this->dcolour3.resize (this->vectorData->size());
                for (unsigned int i = 0; i < this->vectorData->size(); ++i) {
                    this->dcolour[i] = (*this->vectorData)[i][0];
                    this->dcolour2[i] = (*this->vectorData)[i][1];
                    // Could also extract a third colour for Trichrome vs Duochrome (or for raw RGB signal)
                    this->dcolour3[i] = (*this->vectorData)[i][2];
                }
                this->colourScale.transform (this->dcolour, this->dcolour);
                this->colourScale.autoscaled = false;
                this->colourScale.transform (this->dcolour2, this->dcolour2);
                this->colourScale.transform (this->dcolour3, this->dcolour3);
            }

            float _x = 0.0f;
            morph::vec<float> vtx_0, vtx_1, vtx_2, vtx_a, vtx_b;

            float angle_per_distance = this->angle_to_subtend / (dx+this->cg->width());

            for (unsigned int ri = 0; ri < nrect; ++ri) {

                // Use a single colour for each rect, even though rectangle's z
                // positions are interpolated. Do the _colour_ scaling:
                std::array<float, 3> clr = this->setColour (ri);

                // First push the 5 positions of the triangle vertices, starting with the centre
                _x = -(this->cg->d_x[ri]+this->centering_offset[0]); // why mult by -1? Because -x on CartGrid becomes +angle on CurvyTelly
                // For central vertex, reduce radius down:
                float rprime = this->radius * std::cos (hx*angle_per_distance);
                vtx_0 = {
                    rprime * std::cos (this->rotoff + _x*angle_per_distance),
                    rprime * std::sin (this->rotoff + _x*angle_per_distance),
                    this->cg->d_y[ri]+this->centering_offset[1]
                };
                this->vertex_push (vtx_0, this->vertexPositions);
                // Use the centre position as the first location for finding the normal vector

                // NE vertex
                _x += hx;
                vtx_1 = {
                    this->radius * std::cos (this->rotoff + _x*angle_per_distance),
                    this->radius * std::sin (this->rotoff + _x*angle_per_distance),
                    this->cg->d_y[ri]+vy+this->centering_offset[1]
                };
                this->vertex_push (vtx_1, this->vertexPositions);

                // SE vertex
                vtx_2 = vtx_1; // x/y unchanged
                vtx_2[2] = this->cg->d_y[ri]-vy+this->centering_offset[1];
                this->vertex_push (vtx_2, this->vertexPositions);

                // SW vertex
                _x = -(this->cg->d_x[ri]+this->centering_offset[0])-hx;
                vtx_a = {
                    this->radius * std::cos (this->rotoff + _x*angle_per_distance),
                    this->radius * std::sin (this->rotoff + _x*angle_per_distance),
                    this->cg->d_y[ri]-vy+this->centering_offset[1] // same as vtx_2[2]
                };
                this->vertex_push (vtx_a, this->vertexPositions);

                // NW vertex
                vtx_b = vtx_a; // x/y unchanged
                vtx_b[2] = this->cg->d_y[ri]+vy+this->centering_offset[1];
                this->vertex_push (vtx_b, this->vertexPositions);

                // From vtx_0,1,2 compute normal. This sets the correct normal, but note
                // that there is only one 'layer' of vertices; the back of the
                // HexGridVisual will be coloured the same as the front. To get lighting
                // effects to look really good, the back of the surface could need the
                // opposite normal.
                morph::vec<float> plane1 = vtx_1 - vtx_0;
                morph::vec<float> plane2 = vtx_2 - vtx_0;
                morph::vec<float> vnorm = plane2.cross (plane1);
                vnorm.renormalize();
                this->vertex_push (vnorm, this->vertexNormals);
                this->vertex_push (vnorm, this->vertexNormals);
                this->vertex_push (vnorm, this->vertexNormals);
                this->vertex_push (vnorm, this->vertexNormals);
                this->vertex_push (vnorm, this->vertexNormals);

                // Five vertices with the same colour
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);

                // Define indices now to produce the 4 triangles in the hex
                this->indices.push_back (this->idx+1);
                this->indices.push_back (this->idx);
                this->indices.push_back (this->idx+2);

                this->indices.push_back (this->idx+2);
                this->indices.push_back (this->idx);
                this->indices.push_back (this->idx+3);

                this->indices.push_back (this->idx+3);
                this->indices.push_back (this->idx);
                this->indices.push_back (this->idx+4);

                this->indices.push_back (this->idx+4);
                this->indices.push_back (this->idx);
                this->indices.push_back (this->idx+1);

                this->idx += 5; // 5 vertices (each of 3 floats for x/y/z), 15 indices.
            }
        }

        virtual void initializeVertices()
        {
            // Compute the offset to ensure that the cartgrid is centred about the mv_offset.
            if (this->centralize == true) {
                float left_lim = -this->cg->width()/2.0f;
                float bot_lim = -this->cg->depth()/2.0f;
                this->centering_offset[0] = left_lim - this->cg->d_x[0];
                this->centering_offset[1] = bot_lim - this->cg->d_y[0];
                //std::cout << "centering_offset is " << this->centering_offset << std::endl;
            }
            this->drawcurvygrid();
        }
    };

} // namespace
