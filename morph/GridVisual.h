#pragma once

#ifndef USE_GLEW
#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#endif
#include <morph/ColourMap.h>
#include <morph/VisualDataModel.h>
#include <morph/Grid.h>
#include <morph/vec.h>
#include <iostream>
#include <vector>
#include <array>

namespace morph {

    enum class GridVisMode
    {
        Triangles, // Render triangles with a triangle vertex at the centre of each Rect.
        RectInterp  // Render each rect as an actual rectangle made of 4 triangles.
    };

    /*!
     * The template argument T is the type of the *data* which this GridVisual will visualize.
     *
     * GridVisual (like CartGridVisual and HexGridVisual) derives from VisualDataModel, allowing the
     * data (1D, 2D or 3D) to visualize for the Grid to be stored.
     *
     * Right now, I have to pass in all the template arguments for the Grid that will be
     * visualized. This seems clunky.
     */
    template <typename T, size_t n_x, size_t n_y,
              morph::vec<float, 2> dx, morph::vec<float, 2> g_offset,
              bool memory_coords,
              CartDomainWrap d_wrap, GridOrder g_order,
              int glver = morph::gl::version_4_1>
    class GridVisual : public VisualDataModel<T, glver>
    {
    public:

        GridVisual(const morph::Grid<n_x, n_y, dx, g_offset, memory_coords, d_wrap, g_order>* _grid, const vec<float> _offset)
        {
            // Set up...
            morph::vec<float> pixel_offset = dx.plus_one_dim (0.0f);
            this->mv_offset = _offset + pixel_offset;
            this->viewmatrix.translate (this->mv_offset);
            // Defaults for z and colourScale
            this->zScale.setParams (1, 0);
            this->colourScale.do_autoscale = true;
            this->colourScale2.do_autoscale = true;
            this->colourScale3.do_autoscale = true;
            this->grid = _grid;
            // Note: VisualModel::finalize() should be called before rendering
        }

        //! Do the computations to initialize the vertices that will represent the HexGrid.
        virtual void initializeVertices()
        {
            // Optionally compute an offset to ensure that the cartgrid is centred about the mv_offset.
            if (this->centralize == true) { this->centering_offset = -this->grid->centre().plus_one_dim(); }

            switch (this->gridVisMode) {
            case GridVisMode::Triangles:
            {
                this->initializeVerticesTris();
                break;
            }
            case GridVisMode::RectInterp:
            default:
            {
                this->initializeVerticesRectsInterpolated();
                break;
            }
            }

            if (this->showborder == true) {
                // Draw around the outside.
                morph::vec<float, 4> cg_extents = this->grid->extents(); // {xmin, xmax, ymin, ymax}
                float bthick    = this->border_thickness_fixed ? this->border_thickness_fixed : dx[0] * this->border_thickness;
                float bz = dx[0] / 10.0f;
                float half_bthick = bthick/2.0f;
                float left  = cg_extents[0] - half_bthick - (dx[0]/2.0f) + this->centering_offset[0];
                float right = cg_extents[1] + half_bthick + (dx[0]/2.0f) + this->centering_offset[0];
                float bot   = cg_extents[2] - half_bthick - (dx[1]/2.0f) + this->centering_offset[1];
                float top   = cg_extents[3] + half_bthick + (dx[1]/2.0f) + this->centering_offset[1];
                morph::vec<float> lb = {{left, bot, bz}}; // z?
                morph::vec<float> lt = {{left, top, bz}};
                morph::vec<float> rt = {{right, top, bz}};
                morph::vec<float> rb = {{right, bot, bz}};
                this->computeTube (this->idx, lb, lt, this->border_colour, this->border_colour, bthick, 12);
                this->computeTube (this->idx, lt, rt, this->border_colour, this->border_colour, bthick, 12);
                this->computeTube (this->idx, rt, rb, this->border_colour, this->border_colour, bthick, 12);
                this->computeTube (this->idx, rb, lb, this->border_colour, this->border_colour, bthick, 12);
            }
        }

        // Initialize vertex buffer objects and vertex array object.

        //! Initialize as a minimal, triangled surface
        void initializeVerticesTris()
        {
            this->idx = 0;

            if (this->scalarData != nullptr) {
                this->dcopy.resize (this->scalarData->size());
                this->zScale.transform (*(this->scalarData), dcopy);
                this->dcolour.resize (this->scalarData->size());
                this->colourScale.transform (*(this->scalarData), dcolour);
            } else if (this->vectorData != nullptr) {
                this->dcopy.resize (this->vectorData->size());
                this->dcolour.resize (this->vectorData->size());
                this->dcolour2.resize (this->vectorData->size());
                this->dcolour3.resize (this->vectorData->size());
                std::vector<float> veclens(dcopy);
                for (unsigned int i = 0; i < this->vectorData->size(); ++i) {
                    veclens[i] = (*this->vectorData)[i].length();
                    this->dcolour[i] = (*this->vectorData)[i][0];
                    this->dcolour2[i] = (*this->vectorData)[i][1];
                    // Could also extract a third colour for Trichrome vs Duochrome
                    this->dcolour3[i] = (*this->vectorData)[i][2];
                }
                this->zScale.transform (veclens, this->dcopy);
                if (this->cm.getType() != morph::ColourMapType::RGB) {
                    this->colourScale.transform (this->dcolour, this->dcolour);
                    this->colourScale2.transform (this->dcolour2, this->dcolour2);
                    this->colourScale3.transform (this->dcolour3, this->dcolour3);
                }
            }

            for (unsigned int ri = 0; ri < this->grid->n; ++ri) {
                std::array<float, 3> clr = this->setColour (ri);
                this->vertex_push ((*this->grid)[ri][0]+centering_offset[0],
                                   (*this->grid)[ri][1]+centering_offset[1], dcopy[ri], this->vertexPositions);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
            }

            // Build indices based on neighbour relations in the CartGrid
            for (unsigned int ri = 0; ri < this->grid->n; ++ri) {
                if (this->grid->has_nne(ri) && this->grid->has_ne(ri)) {
                    this->indices.push_back (ri);
                    this->indices.push_back (this->grid->index_nne(ri));
                    this->indices.push_back (this->grid->index_ne(ri));
                }

                if (this->grid->has_nw(ri) && this->grid->has_nsw(ri)) {
                    this->indices.push_back (ri);
                    this->indices.push_back (this->grid->index_nw(ri));
                    this->indices.push_back (this->grid->index_nsw(ri));
                }
            }

            this->idx += this->grid->n;
        }

        //! Show a set of hexes at the zero?
        bool zerogrid = false;

        //! Initialize as a rectangle made of 4 triangles for each rect, with z position
        //! of each of the 4 outer edges of the triangles interpolated, but a single colour
        //! for each rectangle. Gives a smooth surface in which you can see the pixels.
        void initializeVerticesRectsInterpolated()
        {
            float hx = 0.5f * dx[0];
            float vy = 0.5f * dx[1];

            this->idx = 0;

            if (this->scalarData != nullptr) {
                this->dcopy.resize (this->scalarData->size());
                this->zScale.transform (*(this->scalarData), dcopy);
                this->dcolour.resize (this->scalarData->size());
                this->colourScale.transform (*(this->scalarData), dcolour);
            } else if (this->vectorData != nullptr) {
                this->dcopy.resize (this->vectorData->size());
                this->dcolour.resize (this->vectorData->size());
                this->dcolour2.resize (this->vectorData->size());
                this->dcolour3.resize (this->vectorData->size());
                std::vector<float> veclens(dcopy);
                for (unsigned int i = 0; i < this->vectorData->size(); ++i) {
                    veclens[i] = (*this->vectorData)[i].length();
                    this->dcolour[i] = (*this->vectorData)[i][0];
                    this->dcolour2[i] = (*this->vectorData)[i][1];
                    // Could also extract a third colour for Trichrome vs Duochrome (or for raw RGB signal)
                    this->dcolour3[i] = (*this->vectorData)[i][2];
                }
                this->zScale.transform (veclens, this->dcopy);

                // Handle case where this->cm.getType() == morph::ColourMapType::RGB and there is
                // exactly one colour. ColourMapType::RGB assumes R/G/B data all in range 0->1
                // ALREADY and therefore they don't need to be re-scaled with this->colourScale.
                if (this->cm.getType() != morph::ColourMapType::RGB) {
                    this->colourScale.transform (this->dcolour, this->dcolour);
                    // Dual axis colour maps like Duochrome and HSV will need to use colourScale2 to
                    // transform their second colour/axis,
                    this->colourScale2.transform (this->dcolour2, this->dcolour2);
                    // Similarly for Triple axis maps
                    this->colourScale3.transform (this->dcolour3, this->dcolour3);
                } // else assume dcolour/dcolour2/dcolour3 are all in range 0->1 (or 0-255) already
            }
            float datumC = 0.0f;   // datum at the centre
            float datumNE = 0.0f;  // datum at the hex to the east.
            float datumNNE = 0.0f;
            float datumNN = 0.0f;
            float datumNNW = 0.0f;
            float datumNW = 0.0f;
            float datumNSW = 0.0f;
            float datumNS = 0.0f;
            float datumNSE = 0.0f;

            float datum = 0.0f;

            morph::vec<float> vtx_0, vtx_1, vtx_2;

            for (size_t ri = 0; ri < this->grid->n; ++ri) {

                // Use the linear scaled copy of the data, dcopy.
                datumC  = dcopy[ri];
                datumNE =  this->grid->has_ne(ri)  ? dcopy[this->grid->index_ne(ri)] : datumC;
                datumNN =  this->grid->has_nn(ri)  ? dcopy[this->grid->index_nn(ri)] : datumC;
                datumNW =  this->grid->has_nw(ri)  ? dcopy[this->grid->index_nw(ri)] : datumC;
                datumNS =  this->grid->has_ns(ri)  ? dcopy[this->grid->index_ns(ri)] : datumC;
                datumNNE = this->grid->has_nne(ri) ? dcopy[this->grid->index_nne(ri)] : datumC;
                datumNNW = this->grid->has_nnw(ri) ? dcopy[this->grid->index_nnw(ri)] : datumC;
                datumNSW = this->grid->has_nsw(ri) ? dcopy[this->grid->index_nsw(ri)] : datumC;
                datumNSE = this->grid->has_nse(ri) ? dcopy[this->grid->index_nse(ri)] : datumC;

                // Use a single colour for each rect, even though rectangle's z
                // positions are interpolated. Do the _colour_ scaling:
                std::array<float, 3> clr = this->setColour (ri);

                // First push the 5 positions of the triangle vertices, starting with the centre
                this->vertex_push ((*this->grid)[ri][0] + centering_offset[0], (*this->grid)[ri][1] + centering_offset[1], datumC, this->vertexPositions);

                // Use the centre position as the first location for finding the normal vector
                vtx_0 = {{(*this->grid)[ri][0] + centering_offset[0], (*this->grid)[ri][1] + centering_offset[1], datumC}};

                // NE vertex
                // Compute mean of this->data[ri] and N, NE and E elements
                //datum = 0.25f * (datumC + datumNN + datumNE + datumNNE);
                if (this->grid->has_nn(ri) && this->grid->has_ne(ri) && this->grid->has_nne(ri)) {
                    datum = 0.25f * (datumC + datumNN + datumNE + datumNNE);
                } else if (this->grid->has_ne(ri)) {
                    // Assume no NN and no NNE
                    datum = 0.5f * (datumC + datumNE);
                } else if (this->grid->has_nn(ri)) {
                    // Assume no NE and no NNE
                    datum = 0.5f * (datumC + datumNN);
                } else {
                    datum = datumC;
                }
                this->vertex_push ((*this->grid)[ri][0]+hx+centering_offset[0], (*this->grid)[ri][1]+vy+centering_offset[1], datum, this->vertexPositions);
                vtx_1 = {{(*this->grid)[ri][0]+hx+centering_offset[0], (*this->grid)[ri][1]+vy+centering_offset[1], datum}};

                // SE vertex
                //datum = 0.25f * (datumC + datumNS + datumNE + datumNSE);
                // SE vertex
                if (this->grid->has_ns(ri) && this->grid->has_ne(ri) && this->grid->has_nse(ri)) {
                    datum = 0.25f * (datumC + datumNS + datumNE + datumNSE);
                } else if (this->grid->has_ne(ri)) {
                    // Assume no NS and no NSE
                    datum = 0.5f * (datumC + datumNE);
                } else if (this->grid->has_ns(ri)) {
                    // Assume no NE and no NSE
                    datum = 0.5f * (datumC + datumNS);
                } else {
                    datum = datumC;
                }
                this->vertex_push ((*this->grid)[ri][0]+hx+centering_offset[0], (*this->grid)[ri][1]-vy+centering_offset[1], datum, this->vertexPositions);
                vtx_2 = {{(*this->grid)[ri][0]+hx+centering_offset[0], (*this->grid)[ri][1]-vy+centering_offset[1], datum}};


                // SW vertex
                //datum = 0.25f * (datumC + datumNS + datumNW + datumNSW);
                if (this->grid->has_ns(ri) && this->grid->has_nw(ri) && this->grid->has_nsw(ri)) {
                    datum = 0.25f * (datumC + datumNS + datumNW + datumNSW);
                } else if (this->grid->has_nw(ri)) {
                    datum = 0.5f * (datumC + datumNW);
                } else if (this->grid->has_ns(ri)) {
                    datum = 0.5f * (datumC + datumNS);
                } else {
                    datum = datumC;
                }
                this->vertex_push ((*this->grid)[ri][0]-hx+centering_offset[0], (*this->grid)[ri][1]-vy+centering_offset[1], datum, this->vertexPositions);

                // NW vertex
                //datum = 0.25f * (datumC + datumNN + datumNW + datumNNW);
                if (this->grid->has_nn(ri) && this->grid->has_nw(ri) && this->grid->has_nnw(ri)) {
                    datum = 0.25f * (datumC + datumNN + datumNW + datumNNW);
                } else if (this->grid->has_nw(ri)) {
                    datum = 0.5f * (datumC + datumNW);
                } else if (this->grid->has_nn(ri)) {
                    datum = 0.5f * (datumC + datumNN);
                } else {
                    datum = datumC;
                }
                this->vertex_push ((*this->grid)[ri][0]-hx+centering_offset[0], (*this->grid)[ri][1]+vy+centering_offset[1], datum, this->vertexPositions);

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

        //! How to render the elements. Triangles are faster.
        GridVisMode gridVisMode = GridVisMode::Triangles;

        //! Set this to true to adjust the positions that the GridVisual uses to plot the Grid so
        //! that the Grid is centralised around the VisualModel::mv_offset.
        bool centralize = false;

        //! Set true to draw a border around the outside
        bool showborder = false;

        //! The colour for the border
        std::array<float, 3> border_colour = morph::colour::grey80;

        //! The border thickness in multiples of a pixel in the CartGrid
        float border_thickness = 0.33f;

        //! If you need to override the pixels-relationship to the border thickness, set it here
        float border_thickness_fixed = 0.0f;

    protected:
        //! An overridable function to set the colour of rect ri
        std::array<float, 3> setColour (unsigned int ri)
        {
            std::array<float, 3> clr = { 0.0f, 0.0f, 0.0f };
            if (this->cm.numDatums() == 3) {
                //if constexpr (std::is_same<std::decay_t<T>, unsigned char>::value == true) {
                if constexpr (std::is_integral<std::decay_t<T>>::value) {
                    // Differs from above as we divide by 255 to get value in range 0-1
                    clr = this->cm.convert (this->dcolour[ri]/255.0f, this->dcolour2[ri]/255.0f, this->dcolour3[ri]/255.0f);
                } else {
                    clr = this->cm.convert (this->dcolour[ri], this->dcolour2[ri], this->dcolour3[ri]);
                }
            } else if (this->cm.numDatums() == 2) {
                // Use vectorData
                clr = this->cm.convert (this->dcolour[ri], this->dcolour2[ri]);
            } else {
                clr = this->cm.convert (this->dcolour[ri]);
            }
            return clr;
        }

        //! The morph::Grid to visualize
        const morph::Grid<n_x, n_y, dx, g_offset, memory_coords, d_wrap, g_order>* grid;

        //! A copy of the scalarData which can be transformed suitably to be the z value of the surface
        std::vector<float> dcopy;
        //! A copy of the scalarData (or first field of vectorData), scaled to be a colour value
        std::vector<float> dcolour;
        std::vector<float> dcolour2;
        std::vector<float> dcolour3;

        // A centering offset to make sure that the Cartgrid is centred on
        // this->mv_offset. This is computed so that you *add* centering_offset to each
        // computed x/y/z position for a rectangle, and this means that the rectangle
        // will be centered around mv_offset.
        morph::vec<float, 3> centering_offset = { 0.0f, 0.0f, 0.0f };
    };

} // namespace morph
