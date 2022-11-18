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
#include <morph/HexGrid.h>
#include <morph/MathAlgo.h>
#include <morph/Vector.h>
#include <iostream>
#include <vector>
#include <array>

/*
 * Macros for testing neighbours. The step along for neighbours on the
 * rows above/below is given by:
 *
 * Dest  | step
 * ----------------------
 * NNE   | +rowlen
 * NNW   | +rowlen - 1
 * NSW   | -rowlen
 * NSE   | -rowlen + 1
 */
#define NE(hi) (this->hg->d_ne[hi])
#define HAS_NE(hi) (this->hg->d_ne[hi] == -1 ? false : true)

#define NW(hi) (this->hg->d_nw[hi])
#define HAS_NW(hi) (this->hg->d_nw[hi] == -1 ? false : true)

#define NNE(hi) (this->hg->d_nne[hi])
#define HAS_NNE(hi) (this->hg->d_nne[hi] == -1 ? false : true)

#define NNW(hi) (this->hg->d_nnw[hi])
#define HAS_NNW(hi) (this->hg->d_nnw[hi] == -1 ? false : true)

#define NSE(hi) (this->hg->d_nse[hi])
#define HAS_NSE(hi) (this->hg->d_nse[hi] == -1 ? false : true)

#define NSW(hi) (this->hg->d_nsw[hi])
#define HAS_NSW(hi) (this->hg->d_nsw[hi] == -1 ? false : true)

#define IF_HAS_NE(hi, yesval, noval)  (HAS_NE(hi)  ? yesval : noval)
#define IF_HAS_NNE(hi, yesval, noval) (HAS_NNE(hi) ? yesval : noval)
#define IF_HAS_NNW(hi, yesval, noval) (HAS_NNW(hi) ? yesval : noval)
#define IF_HAS_NW(hi, yesval, noval)  (HAS_NW(hi)  ? yesval : noval)
#define IF_HAS_NSW(hi, yesval, noval) (HAS_NSW(hi) ? yesval : noval)
#define IF_HAS_NSE(hi, yesval, noval) (HAS_NSE(hi) ? yesval : noval)

namespace morph {

    enum class HexVisMode
    {
        Triangles, // Render triangles with a triangle vertex at the centre of each Hex. Fast (x3.7 cf. HexInterp).
        HexInterp  // Render each hex as an actual hex made of 6 triangles.
        // Could add HexBars - like the Giant's Causeway in Co. Antrim
    };

    //! The template argument T is the type of the data which this HexGridVisual
    //! will visualize.
    template <class T>
    class HexGridVisual : public VisualDataModel<T>
    {
    public:
        //! Simplest constructor. Use this in all new code!
        HexGridVisual(GLuint sp, GLuint tsp, const HexGrid* _hg, const Vector<float> _offset)
        {
            this->shaderprog = sp;
            this->tshaderprog = tsp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->zScale.setParams (1, 0);
            this->colourScale.do_autoscale = true;
            this->hg = _hg;
        }

        //! Hexes to mark out. There are hex iterators, that I can do if (markedHexes.count(hi)) {}
        std::set<unsigned int> markedHexes;

        //! Mark a hex at location r,g,b=0 - it should be outlined with a ring, or
        //! something, so that it is visible.
        void markHex (unsigned int hi) { this->markedHexes.insert(hi); }

        unsigned int datasize = 0;
        //! Find datasize
        void set_datasize()
        {
            this->datasize = 0;
            if (this->vectorData != nullptr && !this->vectorData->empty()) {
                this->datasize = this->vectorData->size();
            } else if (this->scalarData != nullptr && !this->scalarData->empty()) {
                this->datasize = this->scalarData->size();
            } // else datasize remains 0
        }

        //! Zoom factor
        float zoom = 1.0f;

        //! Do the computations to initialize the vertices that will represent the
        //! HexGrid.
        void initializeVertices()
        {
            this->idx = 0;
            this->set_datasize();
            if (this->datasize == 0) { return; }

            switch (this->hexVisMode) {
            case HexVisMode::Triangles:
            {
                this->initializeVerticesTris();
                break;
            }
            case HexVisMode::HexInterp:
            default:
            {
                this->initializeVerticesHexesInterpolated();
                break;
            }
            }
        }

        unsigned int idx = 0;

        // Initialize vertex buffer objects and vertex array object.

        //! Initialize as triangled. Gives a smooth surface with much
        //! less compute than initializeVerticesHexesInterpolated.
        void initializeVerticesTris()
        {
            unsigned int nhex = this->hg->num();

            this->dcopy.resize (this->datasize, 0);
            this->dcolour.resize (this->datasize);

            // zScale and colourScale transform only for scalarData
            if (this->scalarData != nullptr) {
                this->zScale.transform (*(this->scalarData), dcopy);
                this->colourScale.transform (*(this->scalarData), dcolour);
            }

            std::array<float, 3> blkclr = {0,0,0};

            for (unsigned int hi = 0; hi < nhex; ++hi) {
                std::array<float, 3> clr = this->setColour (hi);
                this->vertex_push (this->zoom*this->hg->d_x[hi], this->zoom*this->hg->d_y[hi], this->zoom*dcopy[hi], this->vertexPositions);
                if (this->markedHexes.count(hi)) {
                    this->vertex_push (blkclr, this->vertexColors);
                } else {
                    this->vertex_push (clr, this->vertexColors);
                }
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
            }

            // Build indices based on neighbour relations in the HexGrid
            for (unsigned int hi = 0; hi < nhex; ++hi) {
                if (HAS_NNE(hi) && HAS_NE(hi)) {
                    //std::cout << "1st triangle " << hi << "->" << NNE(hi) << "->" << NE(hi) << std::endl;
                    this->indices.push_back (hi);
                    this->indices.push_back (NNE(hi));
                    this->indices.push_back (NE(hi));
                }

                if (HAS_NW(hi) && HAS_NSW(hi)) {
                    //std::cout << "2nd triangle " << hi << "->" << NW(hi) << "->" << NSW(hi) << std::endl;
                    this->indices.push_back (hi);
                    this->indices.push_back (NW(hi));
                    this->indices.push_back (NSW(hi));
                }
            }
            this->idx = nhex;
        }

        //! Show a set of hexes at the zero?
        bool zerogrid = false;

        //! Show boundary as 'marked' hexes?
        bool showboundary = false;

        //! Show centre hex as a 'marked' hex?
        bool showcentre = false;

        bool showoverlap = true;

        //! Initialize as hexes, with z position of each of the 6
        //! outer edges of the hexes interpolated, but a single colour
        //! for each hex. Gives a smooth surface.
        void initializeVerticesHexesInterpolated()
        {
            float sr = this->hg->getSR();
            float vne = this->hg->getVtoNE();
            float lr = this->hg->getLR();

            unsigned int nhex = this->hg->num();

            this->dcopy.resize (this->datasize, 0);
            this->dcolour.resize (this->datasize);

            if (this->scalarData != nullptr) {
                this->zScale.transform (*(this->scalarData), dcopy);
                this->colourScale.transform (*(this->scalarData), dcolour);
            }

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
                if (this->showboundary && (this->hg->vhexen[hi])->boundaryHex() == true) {
                    this->markHex (hi);
                }
                if (this->showcentre && this->hg->d_x[hi] == 0.0f && this->hg->d_y[hi] == 0.0f) {
                    this->markHex (hi);
                }
                std::array<float, 3> blkclr = {0,0,0};

                // First push the 7 positions of the triangle vertices, starting with the centre
                this->vertex_push (this->zoom*this->hg->d_x[hi], this->zoom*this->hg->d_y[hi], this->zoom*datumC, this->vertexPositions);

                // Use the centre position as the first location for finding the normal vector
                vtx_0 = {{this->zoom*this->hg->d_x[hi], this->zoom*this->hg->d_y[hi], this->zoom*datumC}};

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
                this->vertex_push (this->zoom*(this->hg->d_x[hi]+sr), this->zoom*(this->hg->d_y[hi]+vne), this->zoom*datum, this->vertexPositions);
                vtx_1 = {{this->zoom*(this->hg->d_x[hi]+sr), this->zoom*(this->hg->d_y[hi]+vne), datum}};

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
                this->vertex_push (this->zoom*(this->hg->d_x[hi]+sr), this->zoom*(this->hg->d_y[hi]-vne), this->zoom*datum, this->vertexPositions);
                vtx_2 = {{this->zoom*(this->hg->d_x[hi]+sr), this->zoom*(this->hg->d_y[hi]-vne), this->zoom*datum}};

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
                this->vertex_push (this->zoom*this->hg->d_x[hi], this->zoom*(this->hg->d_y[hi]-lr), this->zoom*datum, this->vertexPositions);

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
                this->vertex_push (this->zoom*(this->hg->d_x[hi]-sr), this->zoom*(this->hg->d_y[hi]-vne), this->zoom*datum, this->vertexPositions);

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
                this->vertex_push (this->zoom*(this->hg->d_x[hi]-sr), this->zoom*(this->hg->d_y[hi]+vne), this->zoom*datum, this->vertexPositions);

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
                this->vertex_push (this->zoom*this->hg->d_x[hi], this->zoom*(this->hg->d_y[hi]+lr), this->zoom*datum, this->vertexPositions);

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

                // Usually seven vertices with the same colour, but if the hex is
                // marked, then three of the vertices are given the colour black,
                // marking the hex out visually.
                this->vertex_push (clr, this->vertexColors);
                if (this->markedHexes.count(hi)) {
                    this->vertex_push (blkclr, this->vertexColors);
                } else {
                    this->vertex_push (clr, this->vertexColors);
                }
                this->vertex_push (clr, this->vertexColors);
                if (this->markedHexes.count(hi)) {
                    this->vertex_push (blkclr, this->vertexColors);
                } else {
                    this->vertex_push (clr, this->vertexColors);
                }
                this->vertex_push (clr, this->vertexColors);
                if (this->markedHexes.count(hi)) {
                    this->vertex_push (blkclr, this->vertexColors);
                } else {
                    this->vertex_push (clr, this->vertexColors);
                }
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

            if (this->showoverlap == true) {
                // Show points and lines for hex overlap/shift
                std::array<float, 3> clr = { 0.3, 0.5, 0.1 };
                std::array<float, 3> blk = { 0, 0, 0 };
                morph::Vector<float, 3> uz = {0.0f, 0.0f, 1.0f};

                float sw = this->hg->getd()/80.0f; // sphere radius (~width)
                float lw = this->hg->getd()/40.0f; // line width
                float lh = this->hg->getd()/60.0f; // line height

                // Vertices and lines of base hexagon
                this->computeSphere (this->idx, this->hg->sw_loc.plus_one_dim(), clr, sw, 14, 12);
                this->computeSphere (this->idx, this->hg->nw_loc.plus_one_dim(), clr, sw, 14, 12);
                this->computeLine (this->idx, this->hg->sw_loc.plus_one_dim(),
                                   this->hg->nw_loc.plus_one_dim(),
                                   uz, clr, clr, lw, lh);
                this->computeSphere (this->idx, this->hg->n_loc.plus_one_dim(), clr, sw, 14, 12);
                this->computeLine (this->idx,
                                   this->hg->nw_loc.plus_one_dim(),
                                   this->hg->n_loc.plus_one_dim(),
                                   uz, clr, clr, lw, lh);
                this->computeSphere (this->idx, this->hg->ne_loc.plus_one_dim(), clr, sw, 14, 12);
                this->computeLine (this->idx,
                                   this->hg->n_loc.plus_one_dim(),
                                   this->hg->ne_loc.plus_one_dim(),
                                   uz, clr, clr, lw, lh);
                this->computeSphere (this->idx, this->hg->se_loc.plus_one_dim(), clr, sw, 14, 12);
                this->computeLine (this->idx,
                                   this->hg->ne_loc.plus_one_dim(),
                                   this->hg->se_loc.plus_one_dim(),
                                   uz, clr, clr, lw, lh);
                this->computeSphere (this->idx, this->hg->s_loc.plus_one_dim(), clr, sw, 14, 12);
                this->computeLine (this->idx,
                                   this->hg->se_loc.plus_one_dim(),
                                   this->hg->s_loc.plus_one_dim(),
                                   uz, clr, clr, lw, lh);
                this->computeSphere (this->idx, this->hg->s_loc.plus_one_dim(), clr, sw, 14, 12);
                this->computeLine (this->idx,
                                   this->hg->s_loc.plus_one_dim(),
                                   this->hg->sw_loc.plus_one_dim(),
                                   uz, clr, clr, lw, lh);

                // Vertices and lines of shifted hexagon
                clr = { 0.9, 0.1, 0.1 };
                this->computeSphere (this->idx, (this->hg->sw_sft).plus_one_dim(), clr, sw, 14, 12);
                this->computeSphere (this->idx, (this->hg->nw_sft).plus_one_dim(), clr, sw, 14, 12);
                this->computeLine (this->idx, (this->hg->sw_sft).plus_one_dim(),
                                   (this->hg->nw_sft).plus_one_dim(),
                                   uz, clr, clr, lw, lh);
                this->computeSphere (this->idx, (this->hg->n_sft).plus_one_dim(), clr, sw, 14, 12);
                this->computeLine (this->idx,
                                   (this->hg->nw_sft).plus_one_dim(),
                                   (this->hg->n_sft).plus_one_dim(),
                                   uz, clr, clr, lw, lh);
                this->computeSphere (this->idx, (this->hg->ne_sft).plus_one_dim(), clr, sw, 14, 12);
                this->computeLine (this->idx,
                                   (this->hg->n_sft).plus_one_dim(),
                                   (this->hg->ne_sft).plus_one_dim(),
                                   uz, clr, clr, lw, lh);
                this->computeSphere (this->idx, (this->hg->se_sft).plus_one_dim(), clr, sw, 14, 12);
                this->computeLine (this->idx,
                                   (this->hg->ne_sft).plus_one_dim(),
                                   (this->hg->se_sft).plus_one_dim(),
                                   uz, clr, clr, lw, lh);
                this->computeSphere (this->idx, (this->hg->s_sft).plus_one_dim(), clr, sw, 14, 12);
                this->computeLine (this->idx,
                                   (this->hg->se_sft).plus_one_dim(),
                                   (this->hg->s_sft).plus_one_dim(),
                                   uz, clr, clr, lw, lh);
                this->computeSphere (this->idx, (this->hg->s_sft).plus_one_dim(), clr, sw, 14, 12);
                this->computeLine (this->idx,
                                   (this->hg->s_sft).plus_one_dim(),
                                   (this->hg->sw_sft).plus_one_dim(),
                                   uz, clr, clr, lw, lh);

                // Drawn lines for finding i1
                clr = blk;
                this->computeLine (this->idx,
                                   this->hg->p1.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   this->hg->q1.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   uz, blk, blk, lw/2.0f, lh);

                this->computeLine (this->idx,
                                   this->hg->p2.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   this->hg->q2.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   uz, blk, blk, lw/2.0f, lh);

                // finding i5
                this->computeLine (this->idx,
                                   this->hg->p3.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   this->hg->q3.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   uz, blk, blk, lw/2.0f, lh);

                this->computeLine (this->idx,
                                   this->hg->p4.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   this->hg->q4.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   uz, blk, blk, lw/2.0f, lh);

                // intersection points
                sw = this->hg->getd()/40.0f;
                this->computeSphere (this->idx, this->hg->i1.plus_one_dim(), clr, sw, 14, 12);
                this->computeSphere (this->idx, this->hg->i2.plus_one_dim(), clr, sw, 14, 12);
                this->computeSphere (this->idx, this->hg->i3.plus_one_dim(), clr, sw, 14, 12);
                this->computeSphere (this->idx, this->hg->i4.plus_one_dim(), clr, sw, 14, 12);
                this->computeSphere (this->idx, this->hg->i5.plus_one_dim(), clr, sw, 14, 12);

                // Draw grey triangles/rects for the relevant areas
                clr = {0.5f, 0.5f, 0.5f};
                // t1
                this->computeLine (this->idx,
                                   this->hg->a1_tl.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   this->hg->i1.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   uz, clr, clr, lw/2.0f, lh);
                this->computeLine (this->idx,
                                   this->hg->i1.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   this->hg->i2.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   uz, clr, clr, lw/2.0f, lh);
                this->computeLine (this->idx,
                                   this->hg->i2.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   this->hg->a1_tl.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   uz, clr, clr, lw/2.0f, lh);
                // t2
                this->computeLine (this->idx,
                                   this->hg->a1_bl.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   this->hg->i3.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   uz, clr, clr, lw/2.0f, lh);
                this->computeLine (this->idx,
                                   this->hg->i3.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   this->hg->i4.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   uz, clr, clr, lw/2.0f, lh);
                this->computeLine (this->idx,
                                   this->hg->i4.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   this->hg->a1_bl.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   uz, clr, clr, lw/2.0f, lh);

                // Sides of a1
                this->computeLine (this->idx,
                                   this->hg->a1_bl.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   this->hg->a1_bl.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   uz, clr, clr, lw/2.0f, lh);
                this->computeLine (this->idx,
                                   this->hg->i2.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   this->hg->i3.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   uz, clr, clr, lw/2.0f, lh);

                // Side of the central rectangle, from i5 and up
                this->computeLine (this->idx,
                                   this->hg->i5.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   this->hg->i6.plus_one_dim()+Vector<float>({0,0,0.02}),
                                   uz, clr, clr, lw/2.0f, lh);
            }

            // Show a Flat surface for the zero plane? This is expensively plotting out all the hexes...
            if (this->zerogrid == true) {
                for (unsigned int hi = 0; hi < nhex; ++hi) {

                    // z position is always 0
                    datum = 0.0f;
                    // Use a single colour for the zero grid
                    std::array<float, 3> clr = { .8f, .8f, .8f};

                    // First push the 7 positions of the triangle vertices, starting with the centre
                    this->vertex_push (this->hg->d_x[hi], this->hg->d_y[hi], datum, this->vertexPositions);

                    // Use the centre position as the first location for finding the normal vector
                    vtx_0 = {{this->hg->d_x[hi], this->hg->d_y[hi], datum}};
                    // NE vertex
                    this->vertex_push (this->hg->d_x[hi]+sr, this->hg->d_y[hi]+vne, datum, this->vertexPositions);
                    vtx_1 = {{this->hg->d_x[hi]+sr, this->hg->d_y[hi]+vne, datum}};
                    // SE vertex
                    this->vertex_push (this->hg->d_x[hi]+sr, this->hg->d_y[hi]-vne, datum, this->vertexPositions);
                    vtx_2 = {{this->hg->d_x[hi]+sr, this->hg->d_y[hi]-vne, datum}};
                    // S
                    this->vertex_push (this->hg->d_x[hi], this->hg->d_y[hi]-lr, datum, this->vertexPositions);
                    // SW
                    this->vertex_push (this->hg->d_x[hi]-sr, this->hg->d_y[hi]-vne, datum, this->vertexPositions);
                    // NW
                    this->vertex_push (this->hg->d_x[hi]-sr, this->hg->d_y[hi]+vne, datum, this->vertexPositions);
                    // N
                    this->vertex_push (this->hg->d_x[hi], this->hg->d_y[hi]+lr, datum, this->vertexPositions);

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
        }

        //! Initialize as hexes, with a step quad between each
        //! hex. Might look cool. Writeme.
        void initializeVerticesHexesStepped() {}

        //! How to render the hexes. Triangles are faster, HexInterp allows you to see
        //! the scale of the hexes in your sim
        HexVisMode hexVisMode = HexVisMode::HexInterp;

    protected:
        //! An overridable function to set the colour of hex hi
        virtual std::array<float, 3> setColour (unsigned int hi)
        {
            std::array<float, 3> clr = {0,0,0};
            // If vectorData has been set, then use it for the colours; otherwise,
            // convert this->dcolour using the current colour map.
            if (this->vectorData != nullptr && !this->vectorData->empty()) {
                // May need to cast vectorData Vector<T,3> to std::array<float, 3>
                if constexpr (std::is_same<std::decay_t<T>, float>::value == true) { // "if T is float"
                    clr = (*this->vectorData)[hi];
                } else {
                    // Need to cast:
                    for (size_t i = 0; i < 3; ++i) {
                        clr[i] = static_cast<float>((*this->vectorData)[hi][i]);
                    }
                }
            } else {
                clr = this->cm.convert (this->dcolour[hi]);
            }
            return clr;
        }

        //! The HexGrid to visualize
        const HexGrid* hg;

        //! A copy of the scalarData which can be transformed suitably to be the z value of the surface
        std::vector<float> dcopy;
        //! A copy of the scalarData, scaled to be a colour value
        std::vector<float> dcolour;
    };

    //! Extended HexGridVisual class for plotting with individual red, green and blue
    //! values (i.e., without a ColourMap).
    template <class T>
    class HexGridVisualManual : public morph::HexGridVisual<T>
    {
    public:
        //! Individual colour values for plotting
        std::vector<float> R, G, B;

        HexGridVisualManual(GLuint sp, GLuint tsp, const morph::HexGrid* _hg, const morph::Vector<float> _offset)
            : morph::HexGridVisual<T>(sp, tsp, _hg, _offset)
        {
            R.resize (this->hg->num(), 0.0f);
            G.resize (this->hg->num(), 0.0f);
            B.resize (this->hg->num(), 0.0f);
        };

#ifdef HGV_DEPRECATED
        HexGridVisualManual(GLuint sp, GLuint tsp,
                            const morph::HexGrid* _hg,
                            const morph::Vector<float> _offset,
                            const std::vector<T>* _data,
                            const morph::Scale<T, float>& zscale,
                            const morph::Scale<T, float>& cscale,
                            morph::ColourMapType _cmt)
            : morph::HexGridVisual<T>(sp, tsp, _hg, _offset, _data, zscale, cscale, _cmt)
        {
            R.resize (this->hg->num(), 0.0f);
            G.resize (this->hg->num(), 0.0f);
            B.resize (this->hg->num(), 0.0f);
        };
#endif

    protected:
        //! In this manual-colour-setting HexGridVisual we override this:
        std::array<float, 3> setColour (unsigned int hi) override
        {
            std::array<float, 3> clr = {R[hi], G[hi], B[hi]};
            return clr;
        }
    };

} // namespace morph
