#pragma once

#include <morph/ColourMap.h>
#include <morph/VisualDataModel.h>
#include <morph/Grid.h>
#include <morph/GridFeatures.h>
#include <morph/vec.h>
#include <iostream>
#include <vector>
#include <array>

namespace morph {

    /*!
     * GridVisual a visualizer for the morph::Grid class
     *
     * \tparam T the type of the *data* which this GridVisual will visualize.
     *
     * \tparam I The type for the Grid indexing (defaults to unsigned int)
     *
     * \tparam C The type for the Grid coordinates (default float, must be a signed type)
     *
     * \tparam glver The OpenGL version in use in your program
     */
    template <typename T, typename I = unsigned int, typename C = float, int glver = morph::gl::version_4_1>
    class GridVisual : public VisualDataModel<T, glver>
    {
    public:

        GridVisual(const morph::Grid<I, C>* _grid, const vec<float> _offset)
        {
            // Set up...
            morph::vec<float> pixel_offset = _grid->get_dx().plus_one_dim (0.0f);
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

        // If you only need to change the colours in your GridVisual (for example, if you are
        // visualizing it flat), then it is about 4 times faster to only update the colours.
        void reinitColours()
        {
            if (this->grid == nullptr) {
                throw std::runtime_error ("grid is nullptr in reinitColours()");
            }

            std::size_t n_data = static_cast<std::size_t>(this->grid->n());
            std::size_t n_cvertices_per_datum = 0;
            // Different gridVisModes will have generated different numbers of OpenGL colour vertices
            switch (this->gridVisMode) {
            case GridVisMode::Triangles:
            {
                n_cvertices_per_datum = 1; // initializeVertices used initializeVerticesTris
                break;
            }
            case GridVisMode::Columns:
            {
                n_cvertices_per_datum = 13; // used initializeVerticesCols
                break;
            }
            case GridVisMode::Pixels:
            case GridVisMode::RectInterp:
            default:
            {
                n_cvertices_per_datum = 5; // used initializeVerticesRectsInterpolated or initializeVerticesPixels
                break;
            }
            }
            if (this->vertexColors.size() < n_data * n_cvertices_per_datum * 3) {
                throw std::runtime_error ("vertexColors is not big enough to reinitColours()");
            }

            // now sub-call the scalar or vector reinit colours function
            if (this->scalarData != nullptr) {
                this->reinitColoursScalar (n_data, n_cvertices_per_datum);
            } else if (this->vectorData != nullptr) {
                this->reinitColoursVector (n_data, n_cvertices_per_datum);
            } else {
                throw std::runtime_error ("No data to reinitColours()");
            }
        }

    public:
        // function that draw a border around the whole image
        void drawBorder()
        {
            // Draw around the outside.
            morph::vec<float, 4> cg_extents = this->grid->extents(); // {xmin, xmax, ymin, ymax}
            morph::vec<float, 2> dx = this->grid->get_dx();
            float bthick    = this->border_thickness_fixed ? this->border_thickness_fixed : dx[0] * this->border_thickness;
            float left  = cg_extents[0] - (dx[0]/2.0f) + this->centering_offset[0];
            float right = cg_extents[1] + (dx[0]/2.0f) + this->centering_offset[0];
            float bot   = cg_extents[2] - (dx[1]/2.0f) + this->centering_offset[1];
            float top   = cg_extents[3] + (dx[1]/2.0f) + this->centering_offset[1];
            morph::vec<float, 4> r_extents = { left, right, bot, top };
            this->rectangularBorder (r_extents, this->border_z_offset, bthick, this->border_colour);
        }

        //! function to draw the grid (border around each pixel)
        void drawGrid()
        {
            // Draw around all pixels
            morph::vec<float, 4> cg_extents = this->grid->extents(); // {xmin, xmax, ymin, ymax}
            morph::vec<float, 2> dx = this->grid->get_dx();
            float gridthick    = this->grid_thickness_fixed ? this->grid_thickness_fixed : dx[0] * this->grid_thickness;
            float bz = 0.01f;
            // loop through each pixel
            for (float left = cg_extents[0] - (dx[0]/2.0f); left < cg_extents[1]; left += dx[0]) {
                for (float bot = cg_extents[2] - (dx[1]/2.0f); bot < cg_extents[3] + (dx[1]/2.0f); bot += dx[1]) {
                    float right = left + dx[0];
                    float top = bot + dx[1];

                    morph::vec<float> lb = {{left + this->centering_offset[0], bot + this->centering_offset[0], bz}}; // z?
                    morph::vec<float> lt = {{left + this->centering_offset[0], top + this->centering_offset[0], bz}};
                    morph::vec<float> rt = {{right + this->centering_offset[0], top + this->centering_offset[0], bz}};
                    morph::vec<float> rb = {{right + this->centering_offset[0], bot + this->centering_offset[0], bz}};

                    // draw the vertical from bottom left to top left
                    this->computeFlatLine(lb, lt, rb, rt, this->uz, this->grid_colour, gridthick);
                    // draw the horizontal from bottom left to bottom right
                    this->computeFlatLine(rb, lb, rt, lt, this->uz, this->grid_colour, gridthick);

                    // complete the last right border (from bottom right to top right)
                    if (right >= cg_extents[1]) {
                        this->computeFlatLine(rt, rb, lt, lb, this->uz, this->grid_colour, gridthick);
                    }
                    // complete the last top border (from top left to top right)
                    if (top >= cg_extents[3] + (dx[1]/2.0f)) {
                        this->computeFlatLine(lt, rt, lb, rb, this->uz, this->grid_colour, gridthick);
                    }
                }
            }
        }

        //! function to draw the border around selected pixels
        void drawSelectedPixBorder()
        {
            // Draw around all pixels
            morph::vec<float, 4> cg_extents = this->grid->extents(); // {xmin, xmax, ymin, ymax}
            morph::vec<float, 2> dx = this->grid->get_dx();
            float gridthick = this->grid_thickness_fixed ? this->grid_thickness_fixed : dx[0] * this->grid_thickness;

            unsigned int pix_width = static_cast<unsigned int>(std::round((cg_extents[1] - cg_extents[0] + dx[0])/dx[0]));

            // If user has NOT resized and populated selected_pix_border_colour AND
            // selected_pix_indexes, resize and default the colour here.
            if (this->selected_pix_border_colour.size() < this->selected_pix_indexes.size()) {
                this->selected_pix_border_colour.resize (this->selected_pix_indexes.size(), this->border_colour);
            }

            float grid_left  = cg_extents[0] - (dx[0]/2.0f) + this->centering_offset[0];
            float grid_bot   = cg_extents[2] - (dx[1]/2.0f) + this->centering_offset[1];

            // loop through each pixel
            for (unsigned int i=0; i < selected_pix_indexes.size(); ++i ) {
                unsigned int r = selected_pix_indexes[i] % pix_width;
                unsigned int c = selected_pix_indexes[i] / pix_width;
                // {xmin, xmax, ymin, ymax}
                morph::vec<float, 4> r_extents = { (grid_left + r * dx[0]), (grid_left + (r+1) * dx[0]), (grid_bot + c * dx[1]), (grid_bot + (c+1) * dx[1]) };
                this->rectangularBorder (r_extents, this->grid_z_offset, gridthick, this->selected_pix_border_colour[i]);
            }
        }

        // Draw a GridVisual border
        void rectangularBorder (const morph::vec<float, 4>& r_extents,
                                const float bz, const float linethickness,
                                const std::array<float, 3>& clr)
        {
                morph::vec<float> lb = {{r_extents[0], r_extents[2], bz}};
                morph::vec<float> lt = {{r_extents[0], r_extents[3], bz}};
                morph::vec<float> rt = {{r_extents[1], r_extents[3], bz}};
                morph::vec<float> rb = {{r_extents[1], r_extents[2], bz}};
                // draw the vertical from bottom left to top left
                this->computeFlatLine(lb, lt, rb, rt, this->uz, clr, linethickness);
                // draw the horizontal from bottom left to bottom right
                this->computeFlatLine(rb, lb, rt, lt, this->uz, clr, linethickness);
                // draw the vertical from bottom right to top right
                this->computeFlatLine(rt, rb, lt, lb, this->uz, clr, linethickness);
                // draw the horizontal from top left to top right
                this->computeFlatLine(lt, rt, lb, rb, this->uz, clr, linethickness);
        }

        //! Draw a border around the selected pixels, using the first selected pix colour
        void drawSelectedPixBorderEnclosing()
        {
            // Draw around all pixels
            morph::vec<float, 4> cg_extents = this->grid->extents(); // {xmin, xmax, ymin, ymax}
            morph::vec<float, 2> dx = this->grid->get_dx();
            float gridthick = this->grid_thickness_fixed ? this->grid_thickness_fixed : dx[0] * this->grid_thickness;

            unsigned int pix_width = static_cast<unsigned int>(std::round((cg_extents[1] - cg_extents[0] + dx[0])/dx[0]));

            if (this->selected_pix_border_colour.empty()) {
                this->selected_pix_border_colour.push_back (this->border_colour);
            }

            float grid_left  = cg_extents[0] - (dx[0]/2.0f) + this->centering_offset[0];
            float grid_bot   = cg_extents[2] - (dx[1]/2.0f) + this->centering_offset[1];

            morph::range<float> l_r; // left extent range
            l_r.search_init();
            morph::range<float> r_r;
            r_r.search_init();
            morph::range<float> b_r;
            b_r.search_init();
            morph::range<float> t_r;
            t_r.search_init();

            // Find extents of our selected pixels
            for (I i = 0; i < this->selected_pix_indexes.size(); ++i) {
                I r = this->selected_pix_indexes[i] % pix_width;
                I c = this->selected_pix_indexes[i] / pix_width;
                float left = grid_left + (r * dx[0]);
                float right = left + dx[0];
                float bot = grid_bot + (c * dx[1]);
                float top = bot + dx[1];
                l_r.update (left);
                r_r.update (right);
                b_r.update (bot);
                t_r.update (top);
            }

            // xmin xmax ymin ymax
            morph::vec<float, 4> r_extents = { l_r.min, r_r.max, b_r.min, t_r.max };
            this->rectangularBorder (r_extents, this->grid_z_offset, gridthick, this->selected_pix_border_colour[0]);
        }

        // Common function to setup scaling. Called by all initializeVertices subroutines. Also
        // checks size of scalar/vectorData and the Grid match.
        void setupScaling()
        {
            if (this->grid == nullptr) {
                throw std::runtime_error ("GridVisual error: grid is a nullptr");
            }
            if (this->scalarData != nullptr) {
                // Check scalar data has same size as Grid
                if (this->scalarData->size() != static_cast<std::size_t>(this->grid->n())) {
                    throw std::runtime_error ("GridVisual error: grid size does not match scalarData size");
                }

                this->dcopy.resize (this->scalarData->size());
                this->zScale.transform (*(this->scalarData), dcopy);
                this->dcolour.resize (this->scalarData->size());
                this->colourScale.transform (*(this->scalarData), dcolour);

            } else if (this->vectorData != nullptr) {

                // Check vector data
                if (this->vectorData->size() != static_cast<std::size_t>(this->grid->n())) {
                    throw std::runtime_error ("GridVisual error: grid size does not match vectorData size");
                }

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
            case GridVisMode::Columns:
            {
                this->initializeVerticesCols();
                break;
            }
            case GridVisMode::Pixels:
            {
                this->initializeVerticesPixels();
                break;
            }
            case GridVisMode::RectInterp:
            default:
            {
                this->initializeVerticesRectsInterpolated();
                break;
            }
            }

            // Note: For reinitColours to work, it's important to do all border/grid drawing AFTER
            // the initializeVerticesTris/Cols/Pixels etc
            if (this->showborder == true) {
                this->drawBorder();
            }
            if (this->showgrid == true) {
                this->drawGrid();
            }
            if (this->showselectedpixborder == true) {
                this->drawSelectedPixBorder();
            }
            if (this->showselectedpixborder_enclosing == true) {
                this->drawSelectedPixBorderEnclosing();
            }
            if (this->showorigin == true) {
                this->computeSphere (morph::vec<float>{0, 0, 0}, morph::colour::crimson, 0.25f * this->grid->get_dx()[0]);
            }
        }

        //! Initialize as a minimal, triangled surface
        void initializeVerticesTris()
        {
            this->idx = 0;
            this->setupScaling();

            for (I ri = 0; ri < this->grid->n(); ++ri) {
                std::array<float, 3> clr = this->setColour (ri);
                this->vertex_push ((*this->grid)[ri][0]+centering_offset[0],
                                   (*this->grid)[ri][1]+centering_offset[1], dcopy[ri], this->vertexPositions);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
            }

            // Build indices row by row.
            auto dims = this->grid->get_dims();
            if (this->grid->get_order() == morph::GridOrder::bottomleft_to_topright) {
                for (I ri = 0; ri < dims[1]-1; ++ri) {
                    for (I ci = 0; ci < dims[0]-1; ++ci) {
                        // Triangle 1
                        I ii = ri * dims[0] + ci;
                        this->indices.push_back (ii);
                        this->indices.push_back (ii + dims[0] + 1); // NNE
                        this->indices.push_back (ii + 1);           // NE
                        // Triangle 2
                        this->indices.push_back (ii);
                        this->indices.push_back (ii + dims[0]);     // NN
                        this->indices.push_back (ii + dims[0] + 1); // NNE
                    }
                }
            } else if (this->grid->get_order() == morph::GridOrder::topleft_to_bottomright) {
                for (I ri = 0; ri < dims[1]-1; ++ri) {
                    for (I ci = 0; ci < dims[0]-1; ++ci) {
                        // Triangle 1
                        I ii = ri * dims[0] + ci;
                        this->indices.push_back (ii);
                        this->indices.push_back (ii + 1);
                        this->indices.push_back (ii + dims[0] + 1); // NSE
                        // Triangle 2
                        this->indices.push_back (ii);
                        this->indices.push_back (ii + dims[0] + 1); // NSE
                        this->indices.push_back (ii + dims[0]);     // NS
                    }
                }
            } else if (this->grid->get_order() == morph::GridOrder::bottomleft_to_topright_colmaj) {
                for (I ri = 0; ri < dims[1]-1; ++ri) {
                    for (I ci = 0; ci < dims[0]-1; ++ci) {
                        // Triangle 1
                        I ii = ci * dims[1] + ri;
                        this->indices.push_back (ii);
                        this->indices.push_back (ii + dims[1] + 1); // NNE
                        this->indices.push_back (ii + dims[1]);     // NE
                        // Triangle 2
                        this->indices.push_back (ii);
                        this->indices.push_back (ii + 1);           // NN
                        this->indices.push_back (ii + dims[1] + 1); // NNE
                    }
                }
            } else if (this->grid->get_order() == morph::GridOrder::topleft_to_bottomright_colmaj) {
                for (I ri = 0; ri < dims[1]-1; ++ri) {
                    for (I ci = 0; ci < dims[0]-1; ++ci) {
                        // Triangle 1
                        I ii = ci * dims[1] + ri;
                        this->indices.push_back (ii);
                        this->indices.push_back (ii + dims[1]);     // NE
                        this->indices.push_back (ii + dims[1] + 1); // NSE
                        // Triangle 2
                        this->indices.push_back (ii);
                        this->indices.push_back (ii + dims[1] + 1); // NSE
                        this->indices.push_back (ii + 1);           // NS
                    }
                }

            } else {
                throw std::runtime_error ("morph::GridVisual: Unhandled morph::GridOrder");
            }

            this->idx += this->grid->n();
        }

        //! Initialize as a rectangle made of 4 triangles for each rect, with z position
        //! of each of the 4 outer edges of the triangles interpolated, but a single colour
        //! for each rectangle. Gives a smooth surface in which you can see the pixels.
        void initializeVerticesRectsInterpolated()
        {
            morph::vec<float, 2> dx = this->grid->get_dx();
            float hx = 0.5f * dx[0];
            float vy = 0.5f * dx[1];

            this->idx = 0;
            this->setupScaling();

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

            for (I ri = 0; ri < this->grid->n(); ++ri) {

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

                // Use a single colour for each rect, even though rectangle's z positions are
                // interpolated. Do the _colour_ scaling:
                std::array<float, 3> clr = this->setColour (ri);

                // First push the 5 positions of the triangle vertices, starting with the centre
                this->vertex_push ((*this->grid)[ri][0] + centering_offset[0], (*this->grid)[ri][1] + centering_offset[1], datumC, this->vertexPositions);

                // Use the centre position as the first location for finding the normal vector
                vtx_0 = {{(*this->grid)[ri][0] + centering_offset[0], (*this->grid)[ri][1] + centering_offset[1], datumC}};

                // NE vertex
                // Compute mean of this->data[ri] and N, NE and E elements
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
                this->vertex_push ((*this->grid)[ri][0] + hx + centering_offset[0], (*this->grid)[ri][1] + vy + centering_offset[1], datum, this->vertexPositions);
                vtx_1 = {{(*this->grid)[ri][0]+hx+centering_offset[0], (*this->grid)[ri][1]+vy+centering_offset[1], datum}};

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

                // From vtx_0,1,2 compute normal. This sets the correct normal, but note that there
                // is only one 'layer' of vertices; the back of the GridVisual will be coloured the
                // same as the front. To get lighting effects to look really good, the back of the
                // surface could need the opposite normal.
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

                // Define indices now to produce the 4 triangles in the pixel
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

        void initializeVerticesCols()
        {
            morph::vec<float, 2> dx = this->grid->get_dx();
            float hx = 0.5f * dx[0];
            float vy = 0.5f * dx[1];

            this->idx = 0;
            this->setupScaling();

            float datumC = 0.0f;   // datum at the centre
            float datumNE = 0.0f;  // datum at the hex to the east.
            float datumNN = 0.0f;

            morph::vec<float> vtx_0, vtx_1, vtx_2, vtx_3, vtx_4;

            for (I ri = 0; ri < this->grid->n(); ++ri) {

                // Use the linear scaled copy of the data, dcopy.
                datumC  = dcopy[ri];
                datumNE =  this->grid->has_ne(ri)  ? dcopy[this->grid->index_ne(ri)] : datumC;
                datumNN =  this->grid->has_nn(ri)  ? dcopy[this->grid->index_nn(ri)] : datumC;

                // Use a single colour for each rect, even though rectangle's z positions are
                // interpolated. Do the _colour_ scaling:
                std::array<float, 3> clr = this->setColour (ri);
                std::array<float, 3> clr_e;
                std::array<float, 3> clr_n;
                if (this->interpolate_colour_sides == true) {
                    clr_e = this->setColour (this->grid->has_ne(ri) ? this->grid->index_ne(ri) : ri);
                    clr_n = this->setColour (this->grid->has_nn(ri) ? this->grid->index_nn(ri) : ri);
                }

                // First push the 5 positions of the triangle vertices, starting with the centre
                this->vertex_push ((*this->grid)[ri][0] + centering_offset[0], (*this->grid)[ri][1] + centering_offset[1], datumC, this->vertexPositions);

                // Use the centre position as the first location for finding the normal vector
                vtx_0 = {{(*this->grid)[ri][0] + centering_offset[0], (*this->grid)[ri][1] + centering_offset[1], datumC}};

                // NE vertex
                this->vertex_push ((*this->grid)[ri][0] + hx + centering_offset[0], (*this->grid)[ri][1] + vy + centering_offset[1], datumC, this->vertexPositions);
                vtx_1 = {{(*this->grid)[ri][0]+hx+centering_offset[0], (*this->grid)[ri][1]+vy+centering_offset[1], datumC}};

                // SE vertex
                this->vertex_push ((*this->grid)[ri][0]+hx+centering_offset[0], (*this->grid)[ri][1]-vy+centering_offset[1], datumC, this->vertexPositions);
                vtx_2 = {{(*this->grid)[ri][0]+hx+centering_offset[0], (*this->grid)[ri][1]-vy+centering_offset[1], datumC}};


                // SW vertex
                this->vertex_push ((*this->grid)[ri][0]-hx+centering_offset[0], (*this->grid)[ri][1]-vy+centering_offset[1], datumC, this->vertexPositions);

                // NW vertex
                this->vertex_push ((*this->grid)[ri][0]-hx+centering_offset[0], (*this->grid)[ri][1]+vy+centering_offset[1], datumC, this->vertexPositions);

                // 4 Neighbour East vertices
                // NE
                this->vertex_push ((*this->grid)[ri][0] + hx + centering_offset[0], (*this->grid)[ri][1] + vy + centering_offset[1], datumC, this->vertexPositions);
                // SE
                this->vertex_push ((*this->grid)[ri][0]+hx+centering_offset[0], (*this->grid)[ri][1]-vy+centering_offset[1], datumC, this->vertexPositions);

                // NE
                this->vertex_push ((*this->grid)[ri][0] + hx + centering_offset[0], (*this->grid)[ri][1] + vy + centering_offset[1], datumNE, this->vertexPositions);
                vtx_3 = {{(*this->grid)[ri][0] + hx + centering_offset[0], (*this->grid)[ri][1] + vy + centering_offset[1], datumNE}};
                // SE
                this->vertex_push ((*this->grid)[ri][0]+hx+centering_offset[0], (*this->grid)[ri][1]-vy+centering_offset[1], datumNE, this->vertexPositions);

                // 4 Neighbour North vertices
                // NW high
                this->vertex_push ((*this->grid)[ri][0]-hx+centering_offset[0], (*this->grid)[ri][1]+vy+centering_offset[1], datumC, this->vertexPositions);
                // NE high
                this->vertex_push ((*this->grid)[ri][0] + hx + centering_offset[0], (*this->grid)[ri][1] + vy + centering_offset[1], datumC, this->vertexPositions);
                // NW low
                this->vertex_push ((*this->grid)[ri][0]-hx+centering_offset[0], (*this->grid)[ri][1]+vy+centering_offset[1], datumNN, this->vertexPositions);
                vtx_4 = {{(*this->grid)[ri][0]-hx+centering_offset[0], (*this->grid)[ri][1]+vy+centering_offset[1], datumNN}};
                // NE low
                this->vertex_push ((*this->grid)[ri][0] + hx + centering_offset[0], (*this->grid)[ri][1] + vy + centering_offset[1], datumNN, this->vertexPositions);

                // From vtx_0,1,2 compute normal. This sets the correct normal, but note that there
                // is only one 'layer' of vertices; the back of the GridVisual will be coloured the
                // same as the front. To get lighting effects to look really good, the back of the
                // surface could need the opposite normal.
                morph::vec<float> plane1 = vtx_1 - vtx_0;
                morph::vec<float> plane2 = vtx_2 - vtx_0;
                morph::vec<float> vnorm = plane2.cross (plane1);

                plane1 = vtx_1 - vtx_2;
                plane2 = vtx_3 - vtx_2;
                morph::vec<float> vnorm_e = plane2.cross (plane1);
                if (datumNE > datumC) { vnorm_e = -vnorm_e; }

                plane1 = vtx_1 - vtx_3;
                plane2 = vtx_4 - vtx_3;
                morph::vec<float> vnorm_n = plane2.cross (plane1);
                if (datumNN > datumC) { vnorm_n = -vnorm_n; }

                vnorm.renormalize();
                this->vertex_push (vnorm, this->vertexNormals);
                this->vertex_push (vnorm, this->vertexNormals);
                this->vertex_push (vnorm, this->vertexNormals);
                this->vertex_push (vnorm, this->vertexNormals);
                this->vertex_push (vnorm, this->vertexNormals);
                this->vertex_push (vnorm_e, this->vertexNormals);
                this->vertex_push (vnorm_e, this->vertexNormals);
                this->vertex_push (vnorm_e, this->vertexNormals);
                this->vertex_push (vnorm_e, this->vertexNormals);
                this->vertex_push (vnorm_n, this->vertexNormals);
                this->vertex_push (vnorm_n, this->vertexNormals);
                this->vertex_push (vnorm_n, this->vertexNormals);
                this->vertex_push (vnorm_n, this->vertexNormals);

                // Five vertices with the same colour
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);

                if (this->interpolate_colour_sides == true) {
                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (clr_e, this->vertexColors);
                    this->vertex_push (clr_e, this->vertexColors);

                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (clr_n, this->vertexColors);
                    this->vertex_push (clr_n, this->vertexColors);
                } else {
                    this->vertex_push (this->clr_east_column, this->vertexColors);
                    this->vertex_push (this->clr_east_column, this->vertexColors);
                    this->vertex_push (this->clr_east_column, this->vertexColors);
                    this->vertex_push (this->clr_east_column, this->vertexColors);

                    this->vertex_push (this->clr_north_column, this->vertexColors);
                    this->vertex_push (this->clr_north_column, this->vertexColors);
                    this->vertex_push (this->clr_north_column, this->vertexColors);
                    this->vertex_push (this->clr_north_column, this->vertexColors);
                }

                // Define indices now to produce the 4 triangles in the pixel
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

                // East face
                this->indices.push_back (this->idx + 5);
                this->indices.push_back (this->idx + 6);
                this->indices.push_back (this->idx + 7);

                this->indices.push_back (this->idx + 6);
                this->indices.push_back (this->idx + 8);
                this->indices.push_back (this->idx + 7);

                // North face
                this->indices.push_back (this->idx + 9);
                this->indices.push_back (this->idx + 10);
                this->indices.push_back (this->idx + 11);

                this->indices.push_back (this->idx + 10);
                this->indices.push_back (this->idx + 12);
                this->indices.push_back (this->idx + 11);

                this->idx += 13;
            }
        }

        //! Floating pixels
        void initializeVerticesPixels()
        {
            morph::vec<float, 2> dx = this->grid->get_dx();
            float hx = 0.5f * dx[0];
            float vy = 0.5f * dx[1];

            this->idx = 0;
            this->setupScaling();

            float datumC = 0.0f;   // datum at the centre

            morph::vec<float> vtx_0, vtx_1, vtx_2;

            for (I ri = 0; ri < this->grid->n(); ++ri) {

                // Use the linear scaled copy of the data, dcopy.
                datumC  = dcopy[ri];

                // Use a single colour for each rect, even though rectangle's z positions are
                // interpolated. Do the _colour_ scaling:
                std::array<float, 3> clr = this->setColour (ri);

                // First push the 5 positions of the triangle vertices, starting with the centre
                this->vertex_push ((*this->grid)[ri][0] + centering_offset[0], (*this->grid)[ri][1] + centering_offset[1], datumC, this->vertexPositions);

                // Use the centre position as the first location for finding the normal vector
                vtx_0 = {{(*this->grid)[ri][0] + centering_offset[0], (*this->grid)[ri][1] + centering_offset[1], datumC}};

                // NE vertex
                this->vertex_push ((*this->grid)[ri][0] + hx + centering_offset[0], (*this->grid)[ri][1] + vy + centering_offset[1], datumC, this->vertexPositions);
                vtx_1 = {{(*this->grid)[ri][0]+hx+centering_offset[0], (*this->grid)[ri][1]+vy+centering_offset[1], datumC}};

                // SE vertex
                this->vertex_push ((*this->grid)[ri][0]+hx+centering_offset[0], (*this->grid)[ri][1]-vy+centering_offset[1], datumC, this->vertexPositions);
                vtx_2 = {{(*this->grid)[ri][0]+hx+centering_offset[0], (*this->grid)[ri][1]-vy+centering_offset[1], datumC}};


                // SW vertex
                this->vertex_push ((*this->grid)[ri][0]-hx+centering_offset[0], (*this->grid)[ri][1]-vy+centering_offset[1], datumC, this->vertexPositions);

                // NW vertex
                this->vertex_push ((*this->grid)[ri][0]-hx+centering_offset[0], (*this->grid)[ri][1]+vy+centering_offset[1], datumC, this->vertexPositions);

                // From vtx_0,1,2 compute normal. This sets the correct normal, but note that there
                // is only one 'layer' of vertices; the back of the GridVisual will be coloured the
                // same as the front. To get lighting effects to look really good, the back of the
                // surface could need the opposite normal.
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

                // Define indices now to produce the 4 triangles in the pixel
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

        //! How to render the elements. Triangles are faster. RectInterp is chosen more often.
        GridVisMode gridVisMode = GridVisMode::RectInterp;

        //! Set this to true to adjust the positions that the GridVisual uses to plot the Grid so
        //! that the Grid is centralised around the VisualModel::mv_offset.
        bool centralize = false;

        //! Set true to draw a grid (border around each pixels)
        bool showgrid = false;

        //! Show a sphere at the grid's origin? This can be useful when placing several Grids with
        //! different sized pixels in a scene - it helps you to figure out the scene coordinates at
        //! which to place each grid.
        bool showorigin = false;

        //! The colour used for the grid (default is grey)
        std::array<float, 3> grid_colour = morph::colour::grey80;

        //! The grid thickness in multiples of a pixel in the Grid
        float grid_thickness = 0.33f;

        //! If you need to override the pixels-relationship to the grid thickness, set it here
        float grid_thickness_fixed = 0.0f;

        //! How far in z to locate the grid lines?
        float grid_z_offset = 0.02f;

        //! Set true to draw a border around the outside
        bool showborder = false;

        //! The colour for the border
        std::array<float, 3> border_colour = morph::colour::grey80;

        //! The border thickness in multiples of a pixel in the Grid
        float border_thickness = 0.33f;

        //! If you need to override the pixels-relationship to the border thickness, set it here
        float border_thickness_fixed = 0.0f;

        //! How far in z to locate the border lines?
        float border_z_offset = 0.02f;

        /*!
         * If true, draw a border around selected pixels (with a full border around each selected
         * pixel). The selected pixels are chosen by the client code, which should populate
         * selected_pix_indexes.
         */
        bool showselectedpixborder = false;

        //! If true, draw a rectangular border enclosing the selected pixels
        bool showselectedpixborder_enclosing = false;

        //! list of those pixel indices that should be drawn with a border
        std::vector<I> selected_pix_indexes;

        //! The colour for the border
        std::vector<std::array<float, 3>> selected_pix_border_colour;

        // If true, interpolate the colour of the sides of columns on a column grid
        bool interpolate_colour_sides = false;

        // User-modifiable colours for the columns if interpolated_colour_sides == false
        std::array<float, 3> clr_east_column = morph::colour::black;
        std::array<float, 3> clr_north_column = morph::colour::black;

    protected:

        //! Called by reinitColours when scalarData is not null
        void reinitColoursScalar (const std::size_t n_data, const std::size_t n_cvertices_per_datum)
        {
            if (this->colourScale.do_autoscale == true) { this->colourScale.reset(); }
            this->dcolour.resize (this->scalarData->size());
            this->colourScale.transform (*(this->scalarData), dcolour);

            // Replace elements of vertexColors
            for (std::size_t i = 0u; i < n_data; ++i) {
                auto c = this->cm.convert (this->dcolour[i]);
                std::size_t d_idx = 3 * i * n_cvertices_per_datum;
                for (std::size_t j = 0; j < n_cvertices_per_datum; ++j) {
                    this->vertexColors[d_idx + 3 * j] = c[0];
                    this->vertexColors[d_idx + 3 * j + 1] = c[1];
                    this->vertexColors[d_idx + 3 * j + 2] = c[2];
                }
            }

            // Lastly, this call copies vertexColors (etc) into the OpenGL memory space
            this->reinit_colour_buffer();
        }

        //! Called by reinitColours when vectorData is not null (vectors are probably RGB colour)
        void reinitColoursVector (const std::size_t n_data, const std::size_t n_cvertices_per_datum)
        {
            if (this->colourScale.do_autoscale == true) { this->colourScale.reset(); }
            for (unsigned int i = 0; i < this->vectorData->size(); ++i) {
                this->dcolour[i] = (*this->vectorData)[i][0];
                this->dcolour2[i] = (*this->vectorData)[i][1];
                this->dcolour3[i] = (*this->vectorData)[i][2];
            }
            if (this->cm.getType() != morph::ColourMapType::RGB) {
                this->colourScale.transform (this->dcolour, this->dcolour);
                this->colourScale2.transform (this->dcolour2, this->dcolour2);
                this->colourScale3.transform (this->dcolour3, this->dcolour3);
            } // else assume dcolour/dcolour2/dcolour3 are all in range 0->1 (or 0-255) already


            // Replace elements of vertexColors
            for (std::size_t i = 0u; i < n_data; ++i) {
                std::array<float, 3> c = this->setColour (i);
                std::size_t d_idx = 3 * i * n_cvertices_per_datum;
                for (std::size_t j = 0; j < n_cvertices_per_datum; ++j) {
                    this->vertexColors[d_idx + 3 * j] = c[0];
                    this->vertexColors[d_idx + 3 * j + 1] = c[1];
                    this->vertexColors[d_idx + 3 * j + 2] = c[2];
                }
            }

            // Lastly, this call copies vertexColors (etc) into the OpenGL memory space
            this->reinit_colour_buffer();
        }

        //! An overridable function to set the colour of rect ri
        std::array<float, 3> setColour (I ri)
        {
            std::array<float, 3> clr = { 0.0f, 0.0f, 0.0f };
            if (this->cm.numDatums() == 3) {
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

        //! The morph::Grid<> to visualize
        const morph::Grid<I, C>* grid;

        //! A copy of the scalarData which can be transformed suitably to be the z value of the surface
        std::vector<float> dcopy;
        //! A copy of the scalarData (or first field of vectorData), scaled to be a colour value
        std::vector<float> dcolour;
        std::vector<float> dcolour2;
        std::vector<float> dcolour3;

        // A centering offset to make sure that the grid is centred on
        // this->mv_offset. This is computed so that you *add* centering_offset to each
        // computed x/y/z position for a rectangle, and this means that the rectangle
        // will be centered around mv_offset.
        morph::vec<float, 3> centering_offset = { 0.0f, 0.0f, 0.0f };
    };

} // namespace morph
