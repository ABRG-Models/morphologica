#pragma once

#include <array>
#include <morph/mathconst.h>
#include <morph/vec.h>
#include <morph/Grid.h>
#include <morph/GridVisual.h>

namespace morph {

    // Draw a curved CartGrid like a curved TV. You make a cylinder if you make the
    // rotation right. Frames can be drawn around the CartGrid.
    template <typename T, int glver = morph::gl::version_4_1>
    struct CurvyTellyVisual : public morph::GridVisual<T, glver>
    {
        // The radius of the curved surface representing the CartGrid
        T radius = T{1};
        // What angle around the circle to draw the telly. 2pi gives a cylinder and is the default.
        T angle_to_subtend = morph::mathconst<T>::two_pi;
        // rotational offset in radians for the rendering. This allows you to arrange the 'centre' of the telly.
        float rotoff = 0.0f;
        // Set this to prevent the edges of the telly from being drawn
        float max_abs_x = std::numeric_limits<float>::max();
        // Draw a frame?
        bool tb_frames = true;
        bool lr_frames = true;
        std::array<float, 3> frame_clr = {0,0,0};
        float frame_width = 0.01f;

        // Note constructor forces centralize to be true, which is important when drawing a curvy CartGrid
        CurvyTellyVisual(const morph::Grid* _cg, const morph::vec<float> _offset)
            : morph::GridVisual<T, glver>(_cg, _offset) { this->centralize = true; }

        void drawcurvygrid()
        {
            morph::vec<float, 2> dx = this->grid->get_dx();
            float hx = 0.5f * dx[0];
            float vy = 0.5f * dx[1];

            unsigned int nrect = this->grid->n;
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
                this->colourScale.reset();
                this->colourScale.transform (this->dcolour2, this->dcolour2);
                this->colourScale.transform (this->dcolour3, this->dcolour3);
            } else {
                std::cout << "No data to set up dcolours\n";
            }

            float _x = 0.0f;
            morph::vec<float> vtx_0; // centre of a CartGrid element
            morph::vec<float> vtx_ne, vtx_nw, vtx_se, vtx_sw;

            float angle_per_distance = this->angle_to_subtend / (dx[0]+this->grid->width());

            // Use extents to plot a frame, if required. extents are { xmin, xmax, ymin, ymax }
            morph::vec<float, 4> extents = this->grid->extents(); // Have to centre
            float xmin = extents[0] + this->centering_offset[0];
            float xmax = extents[1] + this->centering_offset[0];
            float ymin = extents[2] + this->centering_offset[1];
            float ymax = extents[3] + this->centering_offset[1];
            //std::cout << "ymin: " << ymin << " and ymax: " << ymax << std::endl;

            for (unsigned int ri = 0; ri < nrect; ++ri) {

                float T_border = false;
                float B_border = false;
                float L_border = false;
                float R_border = false;

                // Use a single colour for each rect, even though rectangle's z
                // positions are interpolated. Do the _colour_ scaling:
                std::array<float, 3> clr = this->setColour (ri);

                // First push the 5 positions of the triangle vertices, starting with the centre
                _x = -((*this->grid)[ri][0]+this->centering_offset[0]); // why mult by -1? Because -x on CartGrid becomes +angle on CurvyTelly

                // Here we test if we should omit this rectangle.
                if (std::abs(_x) > this->max_abs_x) { continue; }

                // For central vertex, reduce radius down
                float rprime = this->radius * std::cos (hx*angle_per_distance);
                vtx_0 = {
                    rprime * std::cos (this->rotoff + _x*angle_per_distance),
                    rprime * std::sin (this->rotoff + _x*angle_per_distance),
                    (*this->grid)[ri][1]+this->centering_offset[1]
                };
                this->vertex_push (vtx_0, this->vertexPositions);
                // Use the centre position as the first location for finding the normal vector

                // NE vertex
                _x += hx;
                vtx_ne = {
                    this->radius * std::cos (this->rotoff + _x*angle_per_distance),
                    this->radius * std::sin (this->rotoff + _x*angle_per_distance),
                    (*this->grid)[ri][1]+vy+this->centering_offset[1]
                };
                this->vertex_push (vtx_ne, this->vertexPositions);

                // With the NE vertex, figure out if we're at the top/right
                if (vtx_ne[2] > ymax && this->tb_frames == true) { T_border = true; }
                if ((_x > xmax || _x > max_abs_x) && this->lr_frames == true) { R_border = true; }

                // SE vertex
                vtx_se = vtx_ne; // x/y unchanged
                vtx_se[2] = (*this->grid)[ri][1]-vy+this->centering_offset[1];
                this->vertex_push (vtx_se, this->vertexPositions);

                // SW vertex
                _x = -((*this->grid)[ri][0]+this->centering_offset[0])-hx;
                vtx_sw = {
                    this->radius * std::cos (this->rotoff + _x*angle_per_distance),
                    this->radius * std::sin (this->rotoff + _x*angle_per_distance),
                    (*this->grid)[ri][1]-vy+this->centering_offset[1] // same as vtx_2[2]
                };
                this->vertex_push (vtx_sw, this->vertexPositions);

                // With the SW vertex, figure out if we're at the bottom/left
                if (vtx_sw[2] < ymin && this->tb_frames == true) { B_border = true; }
                if ((_x < xmin || std::abs(_x)+std::numeric_limits<float>::epsilon() >= max_abs_x) && this->lr_frames == true) {
                    L_border = true;
                }

                // NW vertex
                vtx_nw = vtx_sw; // x/y unchanged
                vtx_nw[2] = (*this->grid)[ri][1]+vy+this->centering_offset[1];
                this->vertex_push (vtx_nw, this->vertexPositions);

                // From vtx_0,1,2 compute normal. This sets the correct normal, but note
                // that there is only one 'layer' of vertices; the back of the
                // HexGridVisual will be coloured the same as the front. To get lighting
                // effects to look really good, the back of the surface could need the
                // opposite normal.
                morph::vec<float> plane1 = vtx_ne - vtx_0;
                morph::vec<float> plane2 = vtx_se - vtx_0;
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

                if (T_border) { this->draw_top_border (vtx_nw, vtx_ne); }
                if (B_border) { this->draw_bottom_border (vtx_sw, vtx_se); }
                if (L_border) { this->draw_edge_border (vtx_nw, vtx_sw, vtx_ne); }
                if (R_border) { this->draw_edge_border (vtx_ne, vtx_se, vtx_nw); }
            }
        }

        void draw_top_border (const morph::vec<float> vtx_nw, const morph::vec<float> vtx_ne)
        {
            morph::vec<float> vtx_nw_up = vtx_nw;
            morph::vec<float> vtx_ne_up = vtx_ne;
            vtx_nw_up[2] += this->frame_width;
            vtx_ne_up[2] += this->frame_width;
            this->computeFlatQuad (this->idx, vtx_nw, vtx_nw_up, vtx_ne_up, vtx_ne, this->frame_clr);
        }

        void draw_bottom_border (const morph::vec<float> vtx_sw, const morph::vec<float> vtx_se)
        {
            morph::vec<float> vtx_sw_d = vtx_sw;
            morph::vec<float> vtx_se_d = vtx_se;
            vtx_sw_d[2] -= this->frame_width;
            vtx_se_d[2] -= this->frame_width;
            this->computeFlatQuad (this->idx, vtx_sw, vtx_sw_d, vtx_se_d, vtx_se, this->frame_clr);
        }

        void draw_edge_border (morph::vec<float> vtx_a, morph::vec<float> vtx_b, const morph::vec<float> vtx_c)
        {
            // vtx_a is the upper vertex
            vtx_a[2] += this->frame_width;
            vtx_b[2] -= this->frame_width;
            morph::vec<float> vtx_a_l = vtx_a;
            morph::vec<float> vtx_b_l = vtx_b;
            morph::vec<float> vtx_c_dirn = vtx_c - vtx_a;
            vtx_c_dirn.renormalize();
            vtx_c_dirn *= this->frame_width;
            vtx_a_l -= vtx_c_dirn;
            vtx_b_l -= vtx_c_dirn;
            this->computeFlatQuad (this->idx, vtx_a, vtx_a_l, vtx_b_l, vtx_b, this->frame_clr);
        }

        virtual void initializeVertices()
        {
            // Compute the offset to ensure that the cartgrid is centred about the mv_offset.
            if (this->centralize == true) {
                float left_lim = -this->grid->width()/2.0f;
                float bot_lim = -this->grid->height()/2.0f;
                morph::vec<float, 2> coord0 = (*this->grid)[0];
                this->centering_offset[0] = left_lim - coord0[0];
                this->centering_offset[1] = bot_lim - coord0[1];
                //std::cout << "centering_offset is " << this->centering_offset << std::endl;
            }
            this->drawcurvygrid();
        }
    };
} // namespace
