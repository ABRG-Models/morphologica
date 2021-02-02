#pragma once

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#include <morph/tools.h>
#include <morph/VisualDataModel.h>
#include <morph/ColourMap.h>
#include <morph/CartGrid.h>
#include <morph/MathAlgo.h>
#include <morph/Vector.h>
#include <iostream>
#include <vector>
#include <array>

#define NE(hi) (this->cg->d_ne[hi])
#define HAS_NE(hi) (this->cg->d_ne[hi] == -1 ? false : true)

#define NW(hi) (this->cg->d_nw[hi])
#define HAS_NW(hi) (this->cg->d_nw[hi] == -1 ? false : true)

#define NNE(hi) (this->cg->d_nne[hi])
#define HAS_NNE(hi) (this->cg->d_nne[hi] == -1 ? false : true)

#define NN(hi) (this->cg->d_nn[hi])
#define HAS_NN(hi) (this->cg->d_nn[hi] == -1 ? false : true)

#define NNW(hi) (this->cg->d_nnw[hi])
#define HAS_NNW(hi) (this->cg->d_nnw[hi] == -1 ? false : true)

#define NSE(hi) (this->cg->d_nse[hi])
#define HAS_NSE(hi) (this->cg->d_nse[hi] == -1 ? false : true)

#define NS(hi) (this->cg->d_ns[hi])
#define HAS_NS(hi) (this->cg->d_ns[hi] == -1 ? false : true)

#define NSW(hi) (this->cg->d_nsw[hi])
#define HAS_NSW(hi) (this->cg->d_nsw[hi] == -1 ? false : true)

#define IF_HAS_NE(hi, yesval, noval)  (HAS_NE(hi)  ? yesval : noval)
#define IF_HAS_NNE(hi, yesval, noval) (HAS_NNE(hi) ? yesval : noval)
#define IF_HAS_NN(hi, yesval, noval) (HAS_NN(hi) ? yesval : noval)
#define IF_HAS_NNW(hi, yesval, noval) (HAS_NNW(hi) ? yesval : noval)
#define IF_HAS_NW(hi, yesval, noval)  (HAS_NW(hi)  ? yesval : noval)
#define IF_HAS_NSW(hi, yesval, noval) (HAS_NSW(hi) ? yesval : noval)
#define IF_HAS_NS(hi, yesval, noval) (HAS_NS(hi) ? yesval : noval)
#define IF_HAS_NSE(hi, yesval, noval) (HAS_NSE(hi) ? yesval : noval)

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
        CartGridVisual(GLuint sp, GLuint tsp, const CartGrid* _cg, const Vector<float> _offset)
        {
            // Set up...
            this->shaderprog = sp;
            this->tshaderprog = tsp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            // Defaults for z and colourScale
            this->zScale.setParams (1, 0);
            this->colourScale.do_autoscale = true;
            this->cg = _cg;
            // Note: CartGridVisual::finalize should be called before rendering
        }

        // Setters for features. Call these after constructor, but before finalize()
        void setScalarData (const std::vector<T>* _data) { this->scalarData = _data; }

        void setColourScale (const Scale<T, float>& cscale) { this->colourScale = cscale; }

        void setZScale (const Scale<T, float>& zscale) { this->zScale = zscale; }

        void setColourMap (ColourMapType _cmt, const float _hue = 0.0f)
        {
            this->cm.setHue (_hue);
            this->cm.setType (_cmt);
        }

        void setZeroGrid (const bool _zerogrid) { this->zerogrid = _zerogrid; }

        void finalize()
        {
            this->initializeVertices();
            this->postVertexInit();
        }

        //! Do the computations to initialize the vertices that will represent the
        //! HexGrid.
        void initializeVertices (void)
        {
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
        void initializeVerticesTris (void)
        {
            unsigned int nrect = this->cg->num();

            this->dcopy.resize (this->scalarData->size());
            this->zScale.transform (*(this->scalarData), dcopy);
            this->dcolour.resize (this->scalarData->size());
            this->colourScale.transform (*(this->scalarData), dcolour);

            for (unsigned int ri = 0; ri < nrect; ++ri) {
                std::array<float, 3> clr = this->setColour (ri);
                this->vertex_push (this->cg->d_x[ri], this->cg->d_y[ri], dcopy[ri], this->vertexPositions);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
            }

            // Build indices based on neighbour relations in the CartGrid
            for (unsigned int ri = 0; ri < nrect; ++ri) {
                if (HAS_NNE(ri) && HAS_NE(ri)) {
                    this->indices.push_back (ri);
                    this->indices.push_back (NNE(ri));
                    this->indices.push_back (NE(ri));
                }

                if (HAS_NW(ri) && HAS_NSW(ri)) {
                    this->indices.push_back (ri);
                    this->indices.push_back (NW(ri));
                    this->indices.push_back (NSW(ri));
                }
            }
        }

        //! Show a set of hexes at the zero?
        bool zerogrid = false;

        //! Initialize as hexes, with z position of each of the 6
        //! outer edges of the hexes interpolated, but a single colour
        //! for each hex. Gives a smooth surface.
        void initializeVerticesRectsInterpolated (void)
        {
#if 0
            float sr = this->cg->getSR();
            float vne = this->cg->getVtoNE();
            float lr = this->cg->getLR();

            unsigned int nhex = this->cg->num();
            unsigned int idx = 0;

            this->dcopy.resize (this->scalarData->size());
            this->zScale.transform (*(this->scalarData), dcopy);
            this->dcolour.resize (this->scalarData->size());
            this->colourScale.transform (*(this->scalarData), dcolour);

            // These Ts are all floats, right?
            float datumC = 0.0f;   // datum at the centre
            float datumNE = 0.0f;  // datum at the hex to the east.
            float datumNNE = 0.0f; // etc
            float datumNNW = 0.0f;
            float datumNW = 0.0f;
            float datumNSW = 0.0f;
            float datumNSE = 0.0f;

            float datum = 0.0f;
            float third = 0.3333333f;
            float half = 0.5f;
            morph::Vector<float> vtx_0, vtx_1, vtx_2;
            for (unsigned int hi = 0; hi < nhex; ++hi) {

                // Use the linear scaled copy of the data, dcopy.
                datumC   = dcopy[hi];
                datumNE  = HAS_NE(hi)  ? dcopy[NE(hi)]  : datumC; // datum Neighbour East
                datumNNE = HAS_NNE(hi) ? dcopy[NNE(hi)] : datumC; // datum Neighbour North East
                datumNNW = HAS_NNW(hi) ? dcopy[NNW(hi)] : datumC; // etc
                datumNW  = HAS_NW(hi)  ? dcopy[NW(hi)]  : datumC;
                datumNSW = HAS_NSW(hi) ? dcopy[NSW(hi)] : datumC;
                datumNSE = HAS_NSE(hi) ? dcopy[NSE(hi)] : datumC;

                // Use a single colour for each hex, even though hex z positions are
                // interpolated. Do the _colour_ scaling:
                std::array<float, 3> clr = this->setColour (hi);

                // First push the 7 positions of the triangle vertices, starting with the centre
                this->vertex_push (this->cg->d_x[hi], this->cg->d_y[hi], datumC, this->vertexPositions);

                // Use the centre position as the first location for finding the normal vector
                vtx_0 = {{this->cg->d_x[hi], this->cg->d_y[hi], datumC}};

                // NE vertex
                if (HAS_NNE(hi) && HAS_NE(hi)) {
                    // Compute mean of this->data[hi] and NE and E hexes
                    datum = third * (datumC + datumNNE + datumNE);
                } else if (HAS_NNE(hi) || HAS_NE(hi)) {
                    if (HAS_NNE(hi)) {
                        datum = half * (datumC + datumNNE);
                    } else {
                        datum = half * (datumC + datumNE);
                    }
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->cg->d_x[hi]+sr, this->cg->d_y[hi]+vne, datum, this->vertexPositions);
                vtx_1 = {{this->cg->d_x[hi]+sr, this->cg->d_y[hi]+vne, datum}};

                // SE vertex
                if (HAS_NE(hi) && HAS_NSE(hi)) {
                    datum = third * (datumC + datumNE + datumNSE);
                } else if (HAS_NE(hi) || HAS_NSE(hi)) {
                    if (HAS_NE(hi)) {
                        datum = half * (datumC + datumNE);
                    } else {
                        datum = half * (datumC + datumNSE);
                    }
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->cg->d_x[hi]+sr, this->cg->d_y[hi]-vne, datum, this->vertexPositions);
                vtx_2 = {{this->cg->d_x[hi]+sr, this->cg->d_y[hi]-vne, datum}};

                // S
                if (HAS_NSE(hi) && HAS_NSW(hi)) {
                    datum = third * (datumC + datumNSE + datumNSW);
                } else if (HAS_NSE(hi) || HAS_NSW(hi)) {
                    if (HAS_NSE(hi)) {
                        datum = half * (datumC + datumNSE);
                    } else {
                        datum = half * (datumC + datumNSW);
                    }
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->cg->d_x[hi], this->cg->d_y[hi]-lr, datum, this->vertexPositions);

                // SW
                if (HAS_NW(hi) && HAS_NSW(hi)) {
                    datum = third * (datumC + datumNW + datumNSW);
                } else if (HAS_NW(hi) || HAS_NSW(hi)) {
                    if (HAS_NW(hi)) {
                        datum = half * (datumC + datumNW);
                    } else {
                        datum = half * (datumC + datumNSW);
                    }
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->cg->d_x[hi]-sr, this->cg->d_y[hi]-vne, datum, this->vertexPositions);

                // NW
                if (HAS_NNW(hi) && HAS_NW(hi)) {
                    datum = third * (datumC + datumNNW + datumNW);
                } else if (HAS_NNW(hi) || HAS_NW(hi)) {
                    if (HAS_NNW(hi)) {
                        datum = half * (datumC + datumNNW);
                    } else {
                        datum = half * (datumC + datumNW);
                    }
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->cg->d_x[hi]-sr, this->cg->d_y[hi]+vne, datum, this->vertexPositions);

                // N
                if (HAS_NNW(hi) && HAS_NNE(hi)) {
                    datum = third * (datumC + datumNNW + datumNNE);
                } else if (HAS_NNW(hi) || HAS_NNE(hi)) {
                    if (HAS_NNW(hi)) {
                        datum = half * (datumC + datumNNW);
                    } else {
                        datum = half * (datumC + datumNNE);
                    }
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->cg->d_x[hi], this->cg->d_y[hi]+lr, datum, this->vertexPositions);

                // From vtx_0,1,2 compute normal. This sets the correct normal, but note
                // that there is only one 'layer' of vertices; the back of the
                // HexGridVisual will be coloured the same as the front. To get lighting
                // effects to look really good, the back of the surface could need the
                // opposite normal.
                morph::Vector<float> plane1 = vtx_1 - vtx_0;
                morph::Vector<float> plane2 = vtx_2 - vtx_0;
                morph::Vector<float> vnorm = plane2.cross (plane1);
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
                this->indices.push_back (idx+1);
                this->indices.push_back (idx);
                this->indices.push_back (idx+2);

                this->indices.push_back (idx+2);
                this->indices.push_back (idx);
                this->indices.push_back (idx+3);

                this->indices.push_back (idx+3);
                this->indices.push_back (idx);
                this->indices.push_back (idx+4);

                this->indices.push_back (idx+4);
                this->indices.push_back (idx);
                this->indices.push_back (idx+5);

                this->indices.push_back (idx+5);
                this->indices.push_back (idx);
                this->indices.push_back (idx+6);

                this->indices.push_back (idx+6);
                this->indices.push_back (idx);
                this->indices.push_back (idx+1);

                idx += 7; // 7 vertices (each of 3 floats for x/y/z), 18 indices.
            }

            // Show a Flat surface for the zero plane? This is expensively plotting out all the hexes...
            if (this->zerogrid == true) {
                for (unsigned int hi = 0; hi < nhex; ++hi) {

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
                    morph::Vector<float> plane1 = vtx_1 - vtx_0;
                    morph::Vector<float> plane2 = vtx_2 - vtx_0;
                    morph::Vector<float> vnorm = plane2.cross (plane1);
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
                    this->indices.push_back (idx+1);
                    this->indices.push_back (idx);
                    this->indices.push_back (idx+2);

                    this->indices.push_back (idx+2);
                    this->indices.push_back (idx);
                    this->indices.push_back (idx+3);

                    this->indices.push_back (idx+3);
                    this->indices.push_back (idx);
                    this->indices.push_back (idx+4);

                    this->indices.push_back (idx+4);
                    this->indices.push_back (idx);
                    this->indices.push_back (idx+5);

                    this->indices.push_back (idx+5);
                    this->indices.push_back (idx);
                    this->indices.push_back (idx+6);

                    this->indices.push_back (idx+6);
                    this->indices.push_back (idx);
                    this->indices.push_back (idx+1);

                    idx += 7; // 7 vertices (each of 3 floats for x/y/z), 18 indices.
                }
            }
            // End trial grid
#endif
        }

        //! How to render the elements. Triangles are faster.
        CartVisMode cartVisMode = CartVisMode::Triangles;

    protected:
        //! An overridable function to set the colour of hex hi
        virtual std::array<float, 3> setColour (unsigned int hi)
        {
            std::array<float, 3> clr = this->cm.convert (this->dcolour[hi]);
            return clr;
        }

        //! The CartGrid to visualize
        const CartGrid* cg;

        //! A copy of the scalarData which can be transformed suitably to be the z value of the surface
        std::vector<float> dcopy;
        //! A copy of the scalarData, scaled to be a colour value
        std::vector<float> dcolour;
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
                             const morph::Vector<float> _offset)
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
