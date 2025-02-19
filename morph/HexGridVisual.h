#pragma once

#include <morph/gl/version.h>
#include <morph/tools.h>
#include <morph/scale.h>
#include <morph/VisualDataModel.h>
#include <morph/ColourMap.h>
#include <morph/HexGrid.h>
#include <morph/MathAlgo.h>
#include <morph/vec.h>
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
    template <class T, int glver = morph::gl::version_4_1>
    class HexGridVisual final : public VisualDataModel<T,glver>
    {
    public:
        //! Simplest constructor. Use this in all new code!
        HexGridVisual(const HexGrid* _hg, const vec<float> _offset)
        {
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->zScale.setParams (1, 0);
            this->colourScale.do_autoscale = true;
            this->colourScale2.do_autoscale = true;
            this->colourScale3.do_autoscale = true;
            this->hg = _hg;
        }

        //! Hexes to mark out. There are hex iterators, that I can do if (markedHexes.count(hi)) {}
        std::set<unsigned int> markedHexes;

        //! Mark a hex at location r,g,b=0 - it should be outlined with a ring, or
        //! something, so that it is visible.
        void markHex (unsigned int hi) { this->markedHexes.insert(hi); }

        //! The length of the data structure that will be visualized. May be length of
        //! this->scalarData or of this->vectorData.
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

        // Common function for setting up the z and colour scaling
        void setupScaling()
        {
            this->dcopy.resize (this->datasize, 0);
            this->dcolour.resize (this->datasize);

            if (this->scalarData != nullptr) {
                // What do these scaling operations do to any NaNs in scalarData? They should remain
                // NaN. Then in dcopy, might want to make them 0.
                this->zScale.transform (*(this->scalarData), this->dcopy);
                dcopy.replace_nan_with (this->zScale.transform_one(0.0f));
                this->colourScale.transform (*(this->scalarData), this->dcolour);

            } else if (this->vectorData != nullptr) {

                this->dcolour2.resize (this->datasize);
                this->dcolour3.resize (this->datasize);
                std::vector<float> veclens(this->dcopy);
                for (unsigned int i = 0; i < this->datasize; ++i) {
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

        //! Zoom factor
        float zoom = 1.0f;

        //! Show a set of hexes at the zero?
        bool zerogrid = false;

        //! Show boundary as 'marked' hexes?
        bool showboundary = false;

        //! Show centre hex as a 'marked' hex?
        bool showcentre = false;

        //! Set true to show the overlap geometry workings
        bool showoverlap = false;

        //! Set false to omit the hexes (to show just the geometry of showoverlap==true)
        bool showhexes = true;

        void initializeVertices() { this->initializeVertices (false); }
        //! Do the computations to initialize the vertices that will represent the
        //! HexGrid.
        void initializeVertices(const bool update)
        {
            this->idx = 0;
            this->set_datasize();
            if (this->datasize == 0) { return; }

            switch (this->hexVisMode) {
            case HexVisMode::Triangles:
            {
                this->initializeVerticesTris (update);
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

        // This locally defined reinit function knows that we don't want to clear vertexPositions/vertexNormals
        void reinit_on_update()
        {
            if (this->setContext != nullptr) { this->setContext (this->parentVis); }
            // No need to set idx to 0 on an update, or clear/empty vertex/indices containers
            this->initializeVertices (true); // true for 'update' not 'initial build'
            this->reinit_buffers(); // could potentially be 'reinit_position_color_buffers_only()'
        }

        // Override the updateData method
        void updateData (const std::vector<T>* _data)
        {
            this->scalarData = _data;
            switch (this->hexVisMode) {
            case HexVisMode::Triangles:
            {
                this->reinit_on_update(); // instead of VisualDataModel<T,glver>::reinit().
                break;
            }
            default:
            {
                VisualDataModel<T,glver>::reinit();
                break;
            }
            }
        }

        // Initialize vertex buffer objects and vertex array object.

        /*!
         * Initialize as triangled. Gives a smooth surface with much less compute than
         * initializeVerticesHexesInterpolated.
         *
         * If update is true, then we are updating an existing HexGridVisual, and that
         * means we don't need to re-generate the indices OR change the normals.
         */
        void initializeVerticesTris (const bool update)
        {
            unsigned int nhex = this->hg->num();

            this->setupScaling();

            std::array<float, 3> blkclr = {0,0,0};

            if (update == false) {
                this->vertexPositions.resize (3u * nhex);
                this->vertexNormals.resize (3u * nhex);
                this->vertexColors.resize (3u * nhex);
                this->indices.reserve (6u * nhex);
            }

            for (unsigned int hi = 0; hi < nhex; ++hi) {
                std::array<float, 3> clr = this->setColour (hi);
                // If dataCoords has been populated, use these for hex positions, allowing for
                // mapping of the 2D HexGrid onto a 3D manifold.
                if (this->dataCoords == nullptr) {
                    if (update == false) {
                        this->vertexPositions[hi * 3] = this->zoom * this->hg->d_x[hi];
                        this->vertexPositions[hi * 3 + 1] = this->zoom * this->hg->d_y[hi];
                    }
                    this->vertexPositions[hi * 3 + 2] = this->zoom * dcopy[hi];

                } else { // Otherwise use the positions directly in the HexGrid:
                    if (update == false) {
                        this->vertexPositions[hi * 3] = (*this->dataCoords)[hi][0];
                        this->vertexPositions[hi * 3 + 1] = (*this->dataCoords)[hi][1];
                    }
                    this->vertexPositions[hi * 3 + 2] = (*this->dataCoords)[hi][2];
                }
                if (this->markedHexes.count(hi)) {
                    this->vertexColors[hi * 3] = blkclr[0];
                    this->vertexColors[hi * 3 + 1] = blkclr[1];
                    this->vertexColors[hi * 3 + 2] = blkclr[2];
                } else {
                    this->vertexColors[hi * 3] = clr[0];
                    this->vertexColors[hi * 3 + 1] = clr[1];
                    this->vertexColors[hi * 3 + 2] = clr[2];
                }
                if (update == false) {
                    this->vertexNormals[hi * 3] = 0.0f;
                    this->vertexNormals[hi * 3 + 1] = 0.0f;
                    this->vertexNormals[hi * 3 + 2] = 1.0f;
                }
            }

            // Build indices based on neighbour relations in the HexGrid
            // Only needs to happen *on init*. On update, this will not change :)
            if (update == false) {
                std::size_t ind_sz = 0;
                for (unsigned int hi = 0; hi < nhex; ++hi) {
                    if (HAS_NNE(hi) && HAS_NE(hi)) {
                        //std::cout << "1st triangle " << hi << "->" << NNE(hi) << "->" << NE(hi) << std::endl;
                        this->indices.resize (ind_sz + 3);
                        this->indices[ind_sz++] = hi;
                        this->indices[ind_sz++] = NNE(hi);
                        this->indices[ind_sz++] = NE(hi);
                    }

                    if (HAS_NW(hi) && HAS_NSW(hi)) {
                        //std::cout << "2nd triangle " << hi << "->" << NW(hi) << "->" << NSW(hi) << std::endl;
                        this->indices.resize (ind_sz + 3);
                        this->indices[ind_sz++] = hi;
                        this->indices[ind_sz++] = NW(hi);
                        this->indices[ind_sz++] = NSW(hi);
                    }
                }
                this->idx = nhex;
            }
        }

        //! Initialize as hexes, with z position of each of the 6
        //! outer edges of the hexes interpolated, but a single colour
        //! for each hex. Gives a smooth surface.
        void initializeVerticesHexesInterpolated()
        {
            if (this->showhexes == true) {
                this->computeHexes();
            }

            // Optionally show some hexes to verify the hex overlap area computation (see HexGrid::shiftdata)
            if (this->showoverlap == true) {
                this->computeOverlapIndices();
            }

            // Optionally show a Flat surface for the zero plane
            if (this->zerogrid == true) {
                this->computeZerogridIndices();
            }
            // End trial grid
        }

        // Compute vertices for the patchwork quilt of hexes
        void computeHexes()
        {
            // Here's a complication. In a transformed grid, we can't rely on these. Should be able
            // to *compute* them though.
            float sr = this->hg->getSR();
            float vne = this->hg->getVtoNE();
            float lr = this->hg->getLR();

            unsigned int nhex = this->hg->num();

            this->setupScaling();

            // x and y coords on the HexGrid. May be replaced if dataCoords has been set.
            float _x = 0.0f;
            float _y = 0.0f;
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
            morph::vec<float> vtx_0, vtx_1, vtx_2, vtx_tmp;

            morph::vec<float> coordC = { 0.0f, 0.0f, 0.0f };
            morph::vec<float> coordNE = coordC;
            morph::vec<float> coordNNE = coordC;
            morph::vec<float> coordNNW = coordC;
            morph::vec<float> coordNW = coordC;
            morph::vec<float> coordNSW = coordC;
            morph::vec<float> coordNSE = coordC;

            for (unsigned int hi = 0; hi < nhex; ++hi) {

                if (this->dataCoords == nullptr) {
                    _x = this->hg->d_x[hi];
                    _y = this->hg->d_y[hi];
                    // Use the linear scaled copy of the data, dcopy.
                    datumC   = dcopy[hi]; // '_z'
                    datumNE  = HAS_NE(hi)  ? dcopy[NE(hi)]  : datumC; // datum Neighbour East
                    datumNNE = HAS_NNE(hi) ? dcopy[NNE(hi)] : datumC; // datum Neighbour North East
                    datumNNW = HAS_NNW(hi) ? dcopy[NNW(hi)] : datumC; // etc
                    datumNW  = HAS_NW(hi)  ? dcopy[NW(hi)]  : datumC;
                    datumNSW = HAS_NSW(hi) ? dcopy[NSW(hi)] : datumC;
                    datumNSE = HAS_NSE(hi) ? dcopy[NSE(hi)] : datumC;
                } else {
                    // Get coordinates from dataCoords
                    _x = (*this->dataCoords)[hi][0];
                    _y = (*this->dataCoords)[hi][1];
                    datumC = (*this->dataCoords)[hi][2];
                    coordC = (*this->dataCoords)[hi];

                    coordNE  = HAS_NE(hi)  ? (*this->dataCoords)[NE(hi)]  : (*this->dataCoords)[hi]; // datum Neighbour East
                    coordNNE = HAS_NNE(hi) ? (*this->dataCoords)[NNE(hi)] : (*this->dataCoords)[hi]; // datum Neighbour North East
                    coordNNW = HAS_NNW(hi) ? (*this->dataCoords)[NNW(hi)] : (*this->dataCoords)[hi]; // etc
                    coordNW  = HAS_NW(hi)  ? (*this->dataCoords)[NW(hi)]  : (*this->dataCoords)[hi];
                    coordNSW = HAS_NSW(hi) ? (*this->dataCoords)[NSW(hi)] : (*this->dataCoords)[hi];
                    coordNSE = HAS_NSE(hi) ? (*this->dataCoords)[NSE(hi)] : (*this->dataCoords)[hi];

                    datumNE = coordNE[2];
                    datumNNE = coordNNE[2];
                    datumNNW = coordNNW[2];
                    datumNW = coordNW[2];
                    datumNSW = coordNSW[2];
                    datumNSE = coordNSE[2];
                }

                // Use a single colour for each hex, even though hex z positions are
                // interpolated. Do the _colour_ scaling:
                std::array<float, 3> clr = this->setColour (hi);
                if (this->showboundary && (this->hg->vhexen[hi])->boundaryHex() == true) {
                    this->markHex (hi);
                }
                if (this->showcentre && _x == 0.0f && _y == 0.0f) {
                    this->markHex (hi);
                }
                std::array<float, 3> blkclr = {0,0,0};

                // First push the 7 positions of the triangle vertices, starting with the centre

                // Use the centre position as the first location for finding the normal vector
                vtx_0 = this->dataCoords == nullptr ? morph::vec<float>{ _x, _y, datumC } : coordC;
                this->vertex_push (this->zoom * vtx_0, this->vertexPositions);

                // NE vertex
                if (this->dataCoords == nullptr) {
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
                    vtx_1 = { (_x+sr), (_y+vne), datum };
                } else {
                    // Similar logic, but for the coordinate, not just the data value
                    if (HAS_NNE(hi) && HAS_NE(hi)) {
                        // Compute mean of coordC and NE and E hexes
                        vtx_1 = third * (coordC + coordNNE + coordNE);
                    } else if (HAS_NNE(hi) || HAS_NE(hi)) {
                        if (HAS_NNE(hi)) {
                            vtx_1 = half * (coordC + coordNNE);
                        } else {
                            vtx_1 = half * (coordC + coordNE);
                        }
                    } else {
                        vtx_1 = coordC;
                    }
                }
                this->vertex_push (this->zoom * vtx_1, this->vertexPositions);


                // SE vertex
                if (this->dataCoords == nullptr) {
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
                    vtx_2 = { (_x+sr), (_y-vne), datum };
                } else {
                    if (HAS_NE(hi) && HAS_NSE(hi)) {
                        vtx_2 = third * (coordC + coordNE + coordNSE);
                    } else if (HAS_NE(hi) || HAS_NSE(hi)) {
                        if (HAS_NE(hi)) {
                            vtx_2 = half * (coordC + coordNE);
                        } else {
                            vtx_2 = half * (coordC + coordNSE);
                        }
                    } else {
                        vtx_2 = coordC;
                    }
                }
                this->vertex_push (this->zoom * vtx_2, this->vertexPositions);


                // S
                if (this->dataCoords == nullptr) {
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
                    vtx_tmp = { _x, (_y-lr), datum };
                } else {
                    if (HAS_NSE(hi) && HAS_NSW(hi)) {
                        vtx_tmp = third * (coordC + coordNSE + coordNSW);
                    } else if (HAS_NSE(hi) || HAS_NSW(hi)) {
                        if (HAS_NSE(hi)) {
                            vtx_tmp = half * (coordC + coordNSE);
                        } else {
                            vtx_tmp = half * (coordC + coordNSW);
                        }
                    } else {
                        vtx_tmp = coordC;
                    }
                }
                this->vertex_push (this->zoom * vtx_tmp, this->vertexPositions);

                // SW
                if (this->dataCoords == nullptr) {
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
                    vtx_tmp = { (_x-sr), (_y-vne), datum };
                } else {
                    if (HAS_NW(hi) && HAS_NSW(hi)) {
                        vtx_tmp = third * (coordC + coordNW + coordNSW);
                    } else if (HAS_NW(hi) || HAS_NSW(hi)) {
                        if (HAS_NW(hi)) {
                            vtx_tmp = half * (coordC + coordNW);
                        } else {
                            vtx_tmp = half * (coordC + coordNSW);
                        }
                    } else {
                        vtx_tmp = coordC;
                    }
                }
                this->vertex_push (this->zoom * vtx_tmp, this->vertexPositions);

                // NW
                if (this->dataCoords == nullptr) {
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
                    vtx_tmp = { (_x-sr), (_y+vne), datum };
                } else {
                    if (HAS_NNW(hi) && HAS_NW(hi)) {
                        vtx_tmp = third * (coordC + coordNNW + coordNW);
                    } else if (HAS_NNW(hi) || HAS_NW(hi)) {
                        if (HAS_NNW(hi)) {
                            vtx_tmp = half * (coordC + coordNNW);
                        } else {
                            vtx_tmp = half * (coordC + coordNW);
                        }
                    } else {
                        vtx_tmp = coordC;
                    }
                }
                this->vertex_push (this->zoom * vtx_tmp, this->vertexPositions);

                // N
                if (this->dataCoords == nullptr) {
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
                    vtx_tmp = { _x, (_y+lr), datum };
                } else {
                    if (HAS_NNW(hi) && HAS_NNE(hi)) {
                        vtx_tmp = third * (coordC + coordNNW + coordNNE);
                    } else if (HAS_NNW(hi) || HAS_NNE(hi)) {
                        if (HAS_NNW(hi)) {
                            vtx_tmp = half * (coordC + coordNNW);
                        } else {
                            vtx_tmp = half * (coordC + coordNNE);
                        }
                    } else {
                        vtx_tmp = coordC;
                    }
                }
                this->vertex_push (this->zoom * vtx_tmp, this->vertexPositions);

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

                // Usually seven vertices with the same colour, but if the hex is
                // marked, then three of the vertices are given the colour black,
                // marking the hex out visually.
                if (std::isnan(dcolour[hi])) {
                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (blkclr, this->vertexColors);
                    this->vertex_push (blkclr, this->vertexColors);
                    this->vertex_push (blkclr, this->vertexColors);
                    this->vertex_push (blkclr, this->vertexColors);
                    this->vertex_push (blkclr, this->vertexColors);
                    this->vertex_push (blkclr, this->vertexColors);
                } else {
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
                }

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

        // Show a Flat surface for the zero plane. Currently, this is expensively
        // plotting out all the hexes because that was easy. it could be simply a big
        // rectangle of two triangles.
        void computeZerogridIndices()
        {
            float sr = this->hg->getSR();
            float vne = this->hg->getVtoNE();
            float lr = this->hg->getLR();
            unsigned int nhex = this->hg->num();

            morph::vec<float> vtx_0, vtx_1, vtx_2;
            for (unsigned int hi = 0; hi < nhex; ++hi) {

                // z position is always 0
                float datum = 0.0f;
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

        // Compute indices to visualise hexes that have been shifted wrt one
        // another. Used to verify the HexGrid::shiftdata function.
        void computeOverlapIndices()
        {
            // Show points and lines for hex overlap/shift
            std::array<float, 3> clr = { 0.3, 0.5, 0.1 };
            std::array<float, 3> blk = { 0, 0, 0 };
            morph::vec<float, 3> uz = {0.0f, 0.0f, 1.0f};

            float sw = this->hg->getd()/80.0f; // sphere radius (~width)
            float lw = this->hg->getd()/40.0f; // line width
            float lh = this->hg->getd()/60.0f; // line height

            // Vertices and lines of base hexagon
            this->computeSphere (this->hg->sw_loc.plus_one_dim(), clr, sw, 14, 12);
            this->computeSphere (this->hg->nw_loc.plus_one_dim(), clr, sw, 14, 12);
            this->computeLine (this->hg->sw_loc.plus_one_dim(),
                               this->hg->nw_loc.plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere (this->hg->n_loc.plus_one_dim(), clr, sw, 14, 12);
            this->computeLine (this->hg->nw_loc.plus_one_dim(),
                               this->hg->n_loc.plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere (this->hg->ne_loc.plus_one_dim(), clr, sw, 14, 12);
            this->computeLine (this->hg->n_loc.plus_one_dim(),
                               this->hg->ne_loc.plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere (this->hg->se_loc.plus_one_dim(), clr, sw, 14, 12);
            this->computeLine (this->hg->ne_loc.plus_one_dim(),
                               this->hg->se_loc.plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere (this->hg->s_loc.plus_one_dim(), clr, sw, 14, 12);
            this->computeLine (this->hg->se_loc.plus_one_dim(),
                               this->hg->s_loc.plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere (this->hg->s_loc.plus_one_dim(), clr, sw, 14, 12);
            this->computeLine (this->hg->s_loc.plus_one_dim(),
                               this->hg->sw_loc.plus_one_dim(),
                               uz, clr, clr, lw, lh);
            if (!this->hg->q1.has_nan() && !this->hg->q6.has_nan()) {
                this->computeLine (this->hg->q1.plus_one_dim(),
                                   this->hg->q6.plus_one_dim(),
                                   uz, clr, clr, lw, lh);
            }
            if (!this->hg->p6.has_nan() && !this->hg->q6.has_nan()) {
                this->computeLine (this->hg->p6.plus_one_dim(),
                                   this->hg->q6.plus_one_dim(),
                                   uz, clr, clr, lw, lh);
            }
            if (!this->hg->p6.has_nan() && !this->hg->q5.has_nan()) {
                this->computeLine (this->hg->p6.plus_one_dim(),
                                   this->hg->q5.plus_one_dim(),
                                   uz, clr, clr, lw, lh);
            }
            if (!this->hg->q6.has_nan() && !this->hg->p8.has_nan()) {
                this->computeLine (this->hg->q6.plus_one_dim(),
                                   this->hg->p8.plus_one_dim(),
                                   uz, clr, clr, lw, lh);
            }
            if (!this->hg->q8.has_nan() && !this->hg->p8.has_nan()) {
                this->computeLine (this->hg->q8.plus_one_dim(),
                                   this->hg->p8.plus_one_dim(),
                                   uz, clr, clr, lw, lh);
            }


            // Vertices and lines of 0 hexagon
            clr = { 0.1, 0.1, 0.8 };
            this->computeSphere ((this->hg->sw_0).plus_one_dim(), clr, sw, 14, 12);
            this->computeSphere ((this->hg->nw_0).plus_one_dim(), clr, sw, 14, 12);
            this->computeLine ((this->hg->sw_0).plus_one_dim(),
                               (this->hg->nw_0).plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere ((this->hg->n_0).plus_one_dim(), clr, sw, 14, 12);
            this->computeLine ((this->hg->nw_0).plus_one_dim(),
                               (this->hg->n_0).plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere ((this->hg->ne_0).plus_one_dim(), clr, sw, 14, 12);
            this->computeLine ((this->hg->n_0).plus_one_dim(),
                               (this->hg->ne_0).plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere ((this->hg->se_0).plus_one_dim(), clr, sw, 14, 12);
            this->computeLine ((this->hg->ne_0).plus_one_dim(),
                               (this->hg->se_0).plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere ((this->hg->s_0).plus_one_dim(), clr, sw, 14, 12);
            this->computeLine ((this->hg->se_0).plus_one_dim(),
                               (this->hg->s_0).plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere ((this->hg->s_0).plus_one_dim(), clr, sw, 14, 12);
            this->computeLine ((this->hg->s_0).plus_one_dim(),
                               (this->hg->sw_0).plus_one_dim(),
                               uz, clr, clr, lw, lh);

            // Vertices and lines of shifted hexagon
            clr = { 0.9, 0.1, 0.1 };
            this->computeSphere ((this->hg->sw_sft).plus_one_dim(), clr, sw, 14, 12);
            this->computeSphere ((this->hg->nw_sft).plus_one_dim(), clr, sw, 14, 12);
            this->computeLine ((this->hg->sw_sft).plus_one_dim(),
                               (this->hg->nw_sft).plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere ((this->hg->n_sft).plus_one_dim(), clr, sw, 14, 12);
            this->computeLine ((this->hg->nw_sft).plus_one_dim(),
                               (this->hg->n_sft).plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere ((this->hg->ne_sft).plus_one_dim(), clr, sw, 14, 12);
            this->computeLine ((this->hg->n_sft).plus_one_dim(),
                               (this->hg->ne_sft).plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere ((this->hg->se_sft).plus_one_dim(), clr, sw, 14, 12);
            this->computeLine ((this->hg->ne_sft).plus_one_dim(),
                               (this->hg->se_sft).plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere ((this->hg->s_sft).plus_one_dim(), clr, sw, 14, 12);
            this->computeLine ((this->hg->se_sft).plus_one_dim(),
                               (this->hg->s_sft).plus_one_dim(),
                               uz, clr, clr, lw, lh);
            this->computeSphere ((this->hg->s_sft).plus_one_dim(), clr, sw, 14, 12);
            this->computeLine ((this->hg->s_sft).plus_one_dim(),
                               (this->hg->sw_sft).plus_one_dim(),
                               uz, clr, clr, lw, lh);

            // Drawn lines for finding i1
            clr = blk;
            if (!this->hg->p1.has_nan() && !this->hg->q1.has_nan()
                && !this->hg->p2.has_nan() && !this->hg->q2.has_nan()) {
                this->computeLine (this->hg->p1.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   this->hg->q1.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, blk, blk, lw/2.0f, lh);

                this->computeLine (this->hg->p2.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   this->hg->q2.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, blk, blk, lw/2.0f, lh);
            }

            // finding i5
            if (!this->hg->p3.has_nan() && !this->hg->q3.has_nan()
                && !this->hg->p4.has_nan() && !this->hg->q4.has_nan()) {
                this->computeLine (this->hg->p3.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   this->hg->q3.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, blk, blk, lw/2.0f, lh);

                this->computeLine (this->hg->p4.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   this->hg->q4.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, blk, blk, lw/2.0f, lh);
            }

            // intersection points
            sw = this->hg->getd()/40.0f;
            if (!this->hg->i1.has_nan()) {
                clr = {1,0,0};
                this->computeSphere (this->hg->i1.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("i1", (this->hg->i1).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
                clr = {0,0,0};
            }
            if (!this->hg->i2.has_nan()) {
                this->computeSphere (this->hg->i2.plus_one_dim(), clr, sw, 14, 12);
            }
            if (!this->hg->i3.has_nan()) {
                this->computeSphere (this->hg->i3.plus_one_dim(), clr, sw, 14, 12);
            }
            if (!this->hg->i4.has_nan()) {
                this->computeSphere (this->hg->i4.plus_one_dim(), clr, sw, 14, 12);
            }
            if (!this->hg->i5.has_nan()) {
                this->computeSphere (this->hg->i5.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("i5", (this->hg->i5).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }

            // p/q points used to compute additional pgrams
            if (!this->hg->q2.has_nan()) {
                clr = {0,0,1};
                this->computeSphere (this->hg->q2.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("q2", (this->hg->q2).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }
            if (!this->hg->q1.has_nan()) {
                clr = {0,1,0};
                this->computeSphere (this->hg->q1.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("q1", (this->hg->q1).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));

            }
            if (!this->hg->q3.has_nan()) {
                clr = {0,0,1};
                this->computeSphere (this->hg->q3.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("q3", (this->hg->q3).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }
            if (!this->hg->q4.has_nan()) {
                clr = {0,1,0};
                this->computeSphere (this->hg->q4.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("q4", (this->hg->q4).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }
            if (!this->hg->q5.has_nan()) {
                clr = {0,1,0};
                this->computeSphere (this->hg->q5.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("q5", (this->hg->q5).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }
            if (!this->hg->q6.has_nan()) {
                clr = {0,1,0};
                this->computeSphere (this->hg->q6.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("q6", (this->hg->q6).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }
            if (!this->hg->q7.has_nan()) {
                clr = {0,1,0};
                this->computeSphere (this->hg->q7.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("q7", (this->hg->q7).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }
            if (!this->hg->q8.has_nan()) {
                clr = {0,1,0};
                this->computeSphere (this->hg->q8.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("q8", (this->hg->q8).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }
#if 1 // 60/300 units vectors
            if (!this->hg->i1.has_nan() && !this->hg->unit_60.has_nan()) {
                clr = {1,0,0};
                this->computeLine (this->hg->i1.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   (this->hg->i1+this->hg->unit_60).plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, clr, clr, lw/2.0f, lh);
            }
            if (!this->hg->i5.has_nan() && !this->hg->unit_300.has_nan()) {
                clr = {0,0,0};
                this->computeLine (this->hg->i5.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   (this->hg->i5+this->hg->unit_300).plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, clr, clr, lw/2.0f, lh);
            }
            if (!this->hg->i1.has_nan() && !this->hg->unit_120.has_nan()) {
                clr = {1,0,0};
                this->computeLine (this->hg->i1.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   (this->hg->i1+this->hg->unit_120).plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, clr, clr, lw/2.0f, lh);
            }
#endif
            if (!this->hg->p1.has_nan()) {
                clr = {0,1,0};
                this->computeSphere (this->hg->p1.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("p1", (this->hg->p1).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }
            if (!this->hg->p2.has_nan()) {
                clr = {0,0,1};
                this->computeSphere (this->hg->p2.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("p2", (this->hg->p2).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }
            if (!this->hg->p3.has_nan()) {
                clr = {0,0,1};
                this->computeSphere (this->hg->p3.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("p3", (this->hg->p3).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }
            if (!this->hg->p4.has_nan()) {
                clr = {0,1,0};
                this->computeSphere (this->hg->p4.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("p4", (this->hg->p4).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }
            if (!this->hg->p5.has_nan()) {
                clr = {0,1,0};
                this->computeSphere (this->hg->p5.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("p5", (this->hg->p5).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }
            if (!this->hg->p6.has_nan()) {
                clr = {0,1,0};
                this->computeSphere (this->hg->p6.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("p6", (this->hg->p6).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }
            if (!this->hg->p8.has_nan()) {
                clr = {0,1,0};
                this->computeSphere (this->hg->p8.plus_one_dim(), clr, sw, 14, 12);
                this->addLabel ("p8", (this->hg->p8).plus_one_dim()+vec<float>({sw,0,0.02}) * this->hg->getd(),
                                morph::TextFeatures(0.1f * this->hg->getd(), 48, clr));
            }

            // Draw grey triangles/rects for the relevant areas
            clr = {0.5f, 0.5f, 0.5f};
            // t1
            if (!this->hg->a1_tl.has_nan() && !this->hg->i1.has_nan() && !this->hg->i2.has_nan()) {
                this->computeLine (this->hg->a1_tl.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   this->hg->i1.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, clr, clr, lw/2.0f, lh);
                this->computeLine (this->hg->i1.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   this->hg->i2.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, clr, clr, lw/2.0f, lh);
                this->computeLine (this->hg->i2.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   this->hg->a1_tl.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, clr, clr, lw/2.0f, lh);
            }
            // t2
            if (!this->hg->a1_bl.has_nan() && !this->hg->i3.has_nan() && !this->hg->i4.has_nan()) {
                this->computeLine (this->hg->a1_bl.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   this->hg->i3.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, clr, clr, lw/2.0f, lh);
                this->computeLine (this->hg->i3.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   this->hg->i4.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, clr, clr, lw/2.0f, lh);
                this->computeLine (this->hg->i4.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   this->hg->a1_bl.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, clr, clr, lw/2.0f, lh);
            }

            // Sides of a1
            if (!this->hg->a1_bl.has_nan() && !this->hg->i2.has_nan() && !this->hg->i3.has_nan()) {
                this->computeLine (this->hg->a1_bl.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   this->hg->a1_bl.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, clr, clr, lw/2.0f, lh);
                this->computeLine (this->hg->i2.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   this->hg->i3.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, clr, clr, lw/2.0f, lh);
            }

            // Side of the central rectangle, from i5 and up
            if (!this->hg->i5.has_nan() && !this->hg->i6.has_nan()) {
                this->computeLine (this->hg->i5.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   this->hg->i6.plus_one_dim()+vec<float>({0,0,0.02}) * this->hg->getd(),
                                   uz, clr, clr, lw/2.0f, lh);
            }

            // Parallel and rectangle vertices. Do vert cylinders
            if (!this->hg->pll1_top.has_nan()) {
                clr = morph::colour::magenta2;
                this->computeTube (this->hg->pll1_top.plus_one_dim()+vec<float>({0,0,0.1}) * this->hg->getd(),
                                   this->hg->pll1_top.plus_one_dim()+vec<float>({0,0,-0.1}) * this->hg->getd(),
                                   clr, clr, lw/4.0f);
            }
            if (!this->hg->pll1_br.has_nan()) {
                clr = morph::colour::deeppink2;
                this->computeTube (this->hg->pll1_br.plus_one_dim()+vec<float>({0,0,0.1}) * this->hg->getd(),
                                   this->hg->pll1_br.plus_one_dim()+vec<float>({0,0,-0.1}) * this->hg->getd(),
                                   clr, clr, lw/4.0f);
            }
            if (!this->hg->pll2_bot.has_nan()) {
                clr = morph::colour::dodgerblue2;
                this->computeTube (this->hg->pll2_bot.plus_one_dim()+vec<float>({0,0,0.1}) * this->hg->getd(),
                                   this->hg->pll2_bot.plus_one_dim()+vec<float>({0,0,-0.1}) * this->hg->getd(),
                                   clr, clr, lw/4.0f);
            }
            if (!this->hg->pll2_tr.has_nan()) {
                clr = morph::colour::darkgreen;
                this->computeTube (this->hg->pll2_tr.plus_one_dim()+vec<float>({0,0,0.1}) * this->hg->getd(),
                                   this->hg->pll2_tr.plus_one_dim()+vec<float>({0,0,-0.1}) * this->hg->getd(),
                                   clr, clr, lw/4.0f);
            }
            if (!this->hg->a1_tl.has_nan()) {
                clr = morph::colour::yellow;
                this->computeTube (this->hg->a1_tl.plus_one_dim()+vec<float>({0,0,0.1}) * this->hg->getd(),
                                   this->hg->a1_tl.plus_one_dim()+vec<float>({0,0,-0.1}) * this->hg->getd(),
                                   clr, clr, lw/4.0f);
            }
            if (!this->hg->a1_bl.has_nan()) {
                clr = morph::colour::green;
                this->computeTube (this->hg->a1_bl.plus_one_dim()+vec<float>({0,0,0.1}) * this->hg->getd(),
                                   this->hg->a1_bl.plus_one_dim()+vec<float>({0,0,-0.1}) * this->hg->getd(),
                                   clr, clr, lw/4.0f);
            }
       }

        //! How to render the hexes. Triangles are faster, HexInterp allows you to see
        //! the scale of the hexes in your sim
        HexVisMode hexVisMode = HexVisMode::HexInterp;

    protected:
        //! An overridable function to set the colour of hex hi
        virtual std::array<float, 3> setColour (unsigned int hi)
        {
            std::array<float, 3> clr = { 0.0f, 0.0f, 0.0f };
            if (this->cm.numDatums() == 3) {
                //if constexpr (std::is_same<std::decay_t<T>, unsigned char>::value == true) {
                if constexpr (std::is_integral<std::decay_t<T>>::value) {
                    // Differs from above as we divide by 255 to get value in range 0-1
                    clr = this->cm.convert (this->dcolour[hi]/255.0f, this->dcolour2[hi]/255.0f, this->dcolour3[hi]/255.0f);
                } else {
                    clr = this->cm.convert (this->dcolour[hi], this->dcolour2[hi], this->dcolour3[hi]);
                }
            } else if (this->cm.numDatums() == 2) {
                // Use vectorData
                clr = this->cm.convert (this->dcolour[hi], this->dcolour2[hi]);
            } else {
                clr = this->cm.convert (this->dcolour[hi]);
            }
            return clr;
        }

        //! The HexGrid to visualize. This is not expected to change (update methods may
        //! assume the HexGrid has remained unaltered)
        const HexGrid* hg;

        //! A copy of the scalarData which can be transformed suitably to be the z value of the surface
        morph::vvec<float> dcopy;
        //! A copy of the scalarData, scaled to be a colour value
        std::vector<float> dcolour;
        std::vector<float> dcolour2;
        std::vector<float> dcolour3;
    };

} // namespace morph
