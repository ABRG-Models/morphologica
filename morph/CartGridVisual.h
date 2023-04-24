#pragma once

#ifndef USE_GLEW
#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#endif
#include <morph/tools.h>
#include <morph/VisualDataModel.h>
#include <morph/ColourMap.h>
#include <morph/CartGrid.h>
#include <morph/MathAlgo.h>
#include <morph/vec.h>
#include <iostream>
#include <vector>
#include <array>

#define R_NE(hi) (this->cg->d_ne[hi])
#define R_HAS_NE(hi) (this->cg->d_ne[hi] == -1 ? false : true)

#define R_NW(hi) (this->cg->d_nw[hi])
#define R_HAS_NW(hi) (this->cg->d_nw[hi] == -1 ? false : true)

#define R_NNE(hi) (this->cg->d_nne[hi])
#define R_HAS_NNE(hi) (this->cg->d_nne[hi] == -1 ? false : true)

#define R_NN(hi) (this->cg->d_nn[hi])
#define R_HAS_NN(hi) (this->cg->d_nn[hi] == -1 ? false : true)

#define R_NNW(hi) (this->cg->d_nnw[hi])
#define R_HAS_NNW(hi) (this->cg->d_nnw[hi] == -1 ? false : true)

#define R_NSE(hi) (this->cg->d_nse[hi])
#define R_HAS_NSE(hi) (this->cg->d_nse[hi] == -1 ? false : true)

#define R_NS(hi) (this->cg->d_ns[hi])
#define R_HAS_NS(hi) (this->cg->d_ns[hi] == -1 ? false : true)

#define R_NSW(hi) (this->cg->d_nsw[hi])
#define R_HAS_NSW(hi) (this->cg->d_nsw[hi] == -1 ? false : true)

namespace morph {

    enum class CartVisMode
    {
        Triangles, // Render triangles with a triangle vertex at the centre of each Rect.
        RectInterp  // Render each rect as an actual rectangle made of 4 triangles.
    };

    //! The template argument T is the type of the data which this HexGridVisual
    //! will visualize.
    template <class T>
    class CartGridVisual : public VisualDataModel<T>
    {
    public:
        //! Single constructor for simplicity
        CartGridVisual(morph::gl::shaderprogs& _shaders, const CartGrid* _cg, const vec<float> _offset)
        {
            // Set up...
            this->shaders = _shaders;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            // Defaults for z and colourScale
            this->zScale.setParams (1, 0);
            this->colourScale.do_autoscale = true;
            this->cg = _cg;
            // Note: VisualModel::finalize() should be called before rendering
        }

        //! Do the computations to initialize the vertices that will represent the
        //! HexGrid.
        virtual void initializeVertices()
        {
            // Optionally compute an offset to ensure that the cartgrid is centred about the mv_offset.
            if (this->centralize == true) {
                float left_lim = -this->cg->width()/2.0f;
                float bot_lim = -this->cg->depth()/2.0f;
                this->centering_offset[0] = left_lim - this->cg->d_x[0];
                this->centering_offset[1] = bot_lim - this->cg->d_y[0];
            }

            switch (this->cartVisMode) {
            case CartVisMode::Triangles:
            {
                this->initializeVerticesTris();
                break;
            }
            case CartVisMode::RectInterp:
            default:
            {
                this->initializeVerticesRectsInterpolated();
                break;
            }
            }
        }

        // Initialize vertex buffer objects and vertex array object.

        //! Initialize as a minimal, triangled surface
        void initializeVerticesTris()
        {
            unsigned int nrect = this->cg->num();

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
                for (unsigned int i = 0; i < this->vectorData->size(); ++i) {
                    this->dcolour[i] = (*this->vectorData)[i][0];
                    this->dcolour2[i] = (*this->vectorData)[i][1];
                    // Could also extract a third colour for Trichrome vs Duochrome
                    this->dcolour3[i] = (*this->vectorData)[i][2];
                }
                this->colourScale.transform (this->dcolour, this->dcolour);
                this->colourScale.autoscaled = false;
                this->colourScale.transform (this->dcolour2, this->dcolour2);
            }

            for (unsigned int ri = 0; ri < nrect; ++ri) {
                std::array<float, 3> clr = this->setColour (ri);
                this->vertex_push (this->cg->d_x[ri]+centering_offset[0], this->cg->d_y[ri]+centering_offset[1], dcopy[ri], this->vertexPositions);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
            }

            // Build indices based on neighbour relations in the CartGrid
            for (unsigned int ri = 0; ri < nrect; ++ri) {
                if (R_HAS_NNE(ri) && R_HAS_NE(ri)) {
                    this->indices.push_back (ri);
                    this->indices.push_back (R_NNE(ri));
                    this->indices.push_back (R_NE(ri));
                }

                if (R_HAS_NW(ri) && R_HAS_NSW(ri)) {
                    this->indices.push_back (ri);
                    this->indices.push_back (R_NW(ri));
                    this->indices.push_back (R_NSW(ri));
                }
            }

            this->idx = this->vertexPositions.size();
        }

        //! Show a set of hexes at the zero?
        bool zerogrid = false;

        //! Initialize as a rectangle made of 4 triangles for each rect, with z position
        //! of each of the 4 outer edges of the triangles interpolated, but a single colour
        //! for each rectangle. Gives a smooth surface in which you can see the pixels.
        void initializeVerticesRectsInterpolated()
        {
            float dx = this->cg->getd();
            float hx = 0.5f * dx;
            float dy = this->cg->getv();
            float vy = 0.5f * dy;

            unsigned int nrect = this->cg->num();
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
                for (unsigned int i = 0; i < this->vectorData->size(); ++i) {
                    this->dcolour[i] = (*this->vectorData)[i][0];
                    this->dcolour2[i] = (*this->vectorData)[i][1];
                    // Could also extract a third colour for Trichrome vs Duochrome (or for raw RGB signal)
                    this->dcolour3[i] = (*this->vectorData)[i][2];
                }

                // Handle case where this->cm.getType() == morph::ColourMapType::RGB and there is
                // exactly one colour. ColourMapType::RGB assumes R/G/B data all in range 0->1
                // ALREADY and therefore they don't need to be re-scaled with this->colourScale.
                if (this->cm.getType() != morph::ColourMapType::RGB) {
                    this->colourScale.transform (this->dcolour, this->dcolour);
                    this->colourScale.autoscaled = false;
                    this->colourScale.transform (this->dcolour2, this->dcolour2);
                    this->colourScale.transform (this->dcolour3, this->dcolour3);
                } // else assume dcolour/dcolour2/dcolour3 are all in range 0->1 already
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
            for (unsigned int ri = 0; ri < nrect; ++ri) {

                // Use the linear scaled copy of the data, dcopy.
                datumC  = dcopy[ri];
                datumNE =  R_HAS_NE(ri)  ? dcopy[R_NE(ri)] : datumC;
                //std::cout << "NE? " << (R_HAS_NE(ri) ? "yes\n" : "no\n");
                datumNN =  R_HAS_NN(ri)  ? dcopy[R_NN(ri)] : datumC;
                datumNW =  R_HAS_NW(ri)  ? dcopy[R_NW(ri)] : datumC;
                //std::cout << "NW? " << (R_HAS_NW(ri) ? "yes\n" : "no\n");
                datumNS =  R_HAS_NS(ri)  ? dcopy[R_NS(ri)] : datumC;
                datumNNE = R_HAS_NNE(ri) ? dcopy[R_NNE(ri)] : datumC;
                datumNNW = R_HAS_NNW(ri) ? dcopy[R_NNW(ri)] : datumC;
                datumNSW = R_HAS_NSW(ri) ? dcopy[R_NSW(ri)] : datumC;
                datumNSE = R_HAS_NSE(ri) ? dcopy[R_NSE(ri)] : datumC;

                // Use a single colour for each rect, even though rectangle's z
                // positions are interpolated. Do the _colour_ scaling:
                std::array<float, 3> clr = this->setColour (ri);

                // First push the 5 positions of the triangle vertices, starting with the centre
                this->vertex_push (this->cg->d_x[ri]+centering_offset[0], this->cg->d_y[ri]+centering_offset[1], datumC, this->vertexPositions);

                // Use the centre position as the first location for finding the normal vector
                vtx_0 = {{this->cg->d_x[ri]+centering_offset[0], this->cg->d_y[ri]+centering_offset[1], datumC}};

                // NE vertex
                // Compute mean of this->data[ri] and N, NE and E elements
                //datum = 0.25f * (datumC + datumNN + datumNE + datumNNE);
                if (R_HAS_NN(ri) && R_HAS_NE(ri) && R_HAS_NNE(ri)) {
                    datum = 0.25f * (datumC + datumNN + datumNE + datumNNE);
                } else if (R_HAS_NE(ri)) {
                    // Assume no NN and no NNE
                    datum = 0.5f * (datumC + datumNE);
                } else if (R_HAS_NN(ri)) {
                    // Assume no NE and no NNE
                    datum = 0.5f * (datumC + datumNN);
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->cg->d_x[ri]+hx+centering_offset[0], this->cg->d_y[ri]+vy+centering_offset[1], datum, this->vertexPositions);
                vtx_1 = {{this->cg->d_x[ri]+hx+centering_offset[0], this->cg->d_y[ri]+vy+centering_offset[1], datum}};

                // SE vertex
                //datum = 0.25f * (datumC + datumNS + datumNE + datumNSE);
                // SE vertex
                if (R_HAS_NS(ri) && R_HAS_NE(ri) && R_HAS_NSE(ri)) {
                    datum = 0.25f * (datumC + datumNS + datumNE + datumNSE);
                } else if (R_HAS_NE(ri)) {
                    // Assume no NS and no NSE
                    datum = 0.5f * (datumC + datumNE);
                } else if (R_HAS_NS(ri)) {
                    // Assume no NE and no NSE
                    datum = 0.5f * (datumC + datumNS);
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->cg->d_x[ri]+hx+centering_offset[0], this->cg->d_y[ri]-vy+centering_offset[1], datum, this->vertexPositions);
                vtx_2 = {{this->cg->d_x[ri]+hx+centering_offset[0], this->cg->d_y[ri]-vy+centering_offset[1], datum}};


                // SW vertex
                //datum = 0.25f * (datumC + datumNS + datumNW + datumNSW);
                if (R_HAS_NS(ri) && R_HAS_NW(ri) && R_HAS_NSW(ri)) {
                    datum = 0.25f * (datumC + datumNS + datumNW + datumNSW);
                } else if (R_HAS_NW(ri)) {
                    datum = 0.5f * (datumC + datumNW);
                } else if (R_HAS_NS(ri)) {
                    datum = 0.5f * (datumC + datumNS);
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->cg->d_x[ri]-hx+centering_offset[0], this->cg->d_y[ri]-vy+centering_offset[1], datum, this->vertexPositions);

                // NW vertex
                //datum = 0.25f * (datumC + datumNN + datumNW + datumNNW);
                if (R_HAS_NN(ri) && R_HAS_NW(ri) && R_HAS_NNW(ri)) {
                    datum = 0.25f * (datumC + datumNN + datumNW + datumNNW);
                } else if (R_HAS_NW(ri)) {
                    datum = 0.5f * (datumC + datumNW);
                } else if (R_HAS_NN(ri)) {
                    datum = 0.5f * (datumC + datumNN);
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->cg->d_x[ri]-hx+centering_offset[0], this->cg->d_y[ri]+vy+centering_offset[1], datum, this->vertexPositions);

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

#if 0
            // Show a Flat surface for the zero plane? This is expensively plotting out all the hexes...
            // Instead use cg->getExtents() and plot two triangles.
            if (this->zerogrid == true) {
                for (unsigned int hi = 0; hi < nrect; ++hi) {

                    // z position is always 0
                    datum = 0.0f;
                    // Use a single colour for the zero grid
                    std::array<float, 3> clr = { .8f, .8f, .8f};

                    // First push the 7 positions of the triangle vertices, starting with the centre
                    this->vertex_push (this->cg->d_x[hi], this->cg->d_y[hi], datum, this->vertexPositions);

                    // Use the centre position as the first location for finding the normal vector
                    vtx_0 = {{this->cg->d_x[hi], this->cg->d_y[hi], datum}};
                    // NE vertex
                    this->vertex_push (this->cg->d_x[hi]+sr, this->cg->d_y[hi]+vne, datum, this->vertexPositions);
                    vtx_1 = {{this->cg->d_x[hi]+sr, this->cg->d_y[hi]+vne, datum}};
                    // SE vertex
                    this->vertex_push (this->cg->d_x[hi]+sr, this->cg->d_y[hi]-vne, datum, this->vertexPositions);
                    vtx_2 = {{this->cg->d_x[hi]+sr, this->cg->d_y[hi]-vne, datum}};
                    // S
                    this->vertex_push (this->cg->d_x[hi], this->cg->d_y[hi]-lr, datum, this->vertexPositions);
                    // SW
                    this->vertex_push (this->cg->d_x[hi]-sr, this->cg->d_y[hi]-vne, datum, this->vertexPositions);
                    // NW
                    this->vertex_push (this->cg->d_x[hi]-sr, this->cg->d_y[hi]+vne, datum, this->vertexPositions);
                    // N
                    this->vertex_push (this->cg->d_x[hi], this->cg->d_y[hi]+lr, datum, this->vertexPositions);

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
                    this->vertex_push (vnorm, this->vertexNormals);
                    this->vertex_push (vnorm, this->vertexNormals);

                    // Seven vertices with the same colour
                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (clr, this->vertexColors);

                    // Define indices now to produce the 6 triangles in the hex
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
                    this->indices.push_back (this->idx+5);

                    this->indices.push_back (this->idx+5);
                    this->indices.push_back (this->idx);
                    this->indices.push_back (this->idx+6);

                    this->indices.push_back (this->idx+6);
                    this->indices.push_back (this->idx);
                    this->indices.push_back (this->idx+1);

                    this->idx += 7; // 7 vertices (each of 3 floats for x/y/z), 18 indices.
                }
            }
            // End trial grid
#endif
        }

        //! How to render the elements. Triangles are faster.
        CartVisMode cartVisMode = CartVisMode::Triangles;

        // Set this to true to adjust the positions that the CartGridVisual uses to plot
        // the CartGrid so that the CartGrid is centralised around the
        // VisualModel::mv_offset.
        bool centralize = false;

    protected:
        //! An overridable function to set the colour of hex hi
        virtual std::array<float, 3> setColour (unsigned int hi)
        {
            std::array<float, 3> clr = { 0.0f, 0.0f, 0.0f };
            if (this->cm.numDatums() == 3) {
                clr = this->cm.convert (this->dcolour[hi], this->dcolour2[hi], this->dcolour3[hi]);
            } else if (this->cm.numDatums() == 2) {
                // Use vectorData
                clr = this->cm.convert (this->dcolour[hi], this->dcolour2[hi]);
            } else {
                clr = this->cm.convert (this->dcolour[hi]);
            }
            return clr;
        }

        //! The CartGrid to visualize
        const CartGrid* cg;

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

    //! Extended CartGridVisual class for plotting with individual red, green and blue
    //! values (i.e., without a ColourMap).
    template <class T>
    class CartGridVisualManual : public morph::CartGridVisual<T>
    {
    public:
        //! Individual colour values for plotting
        std::vector<float> R, G, B;

        CartGridVisualManual(GLuint sp, GLuint tsp,
                             const morph::CartGrid* _cg,
                             const morph::vec<float> _offset)
            : morph::CartGridVisual<T>(sp, tsp, _cg, _offset)
        {
            R.resize (this->cg->num(), 0.0f);
            G.resize (this->cg->num(), 0.0f);
            B.resize (this->cg->num(), 0.0f);
        };

    protected:
        //! In this manual-colour-setting CartGridVisual we override this:
        std::array<float, 3> setColour (unsigned int hi) override
        {
            std::array<float, 3> clr = {R[hi], G[hi], B[hi]};
            return clr;
        }
    };

} // namespace morph
