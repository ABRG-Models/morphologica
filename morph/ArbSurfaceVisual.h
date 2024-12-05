/*!
 * Visualize an arbitrary surface defined by values at points in 3D space (similar to
 * ScatterVisual). Compute a 2.5D Delaunay triangulization around the data points to create the
 * 'panels' to colourize. An assumption is that the z value of the data coordinates can be 'set
 * aside' and then a 2D Delaunay triangulation applied to the (x,y) coordinates.
 *
 * \author Seb James
 * \date 2024
 */
#pragma once

#include <morph/tools.h>
#include <morph/VisualDataModel.h>
#include <morph/scale.h>
#include <morph/vec.h>
#include <iostream>
#include <vector>
#include <array>

#include <morph/delaunator.hpp>

namespace morph {

    //! The template argument F is the type of the data which this ArbSurfaceVisual
    //! will visualize.
    template <typename F, int glver = morph::gl::version_4_1>
    class ArbSurfaceVisual : public VisualDataModel<F, glver>
    {
    public:
        ArbSurfaceVisual (const vec<float> _offset)
        {
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->zScale.setParams (1, 0);
            this->colourScale.do_autoscale = true;
        }

        void setupScaling (size_t n)
        {
            if (this->scalarData != nullptr) {
                // Check scalar data has same size as Grid
                if (this->scalarData->size() != n) {
                    throw std::runtime_error ("Error: scalarData size does not match n");
                }

                this->dcopy.resize (n);
                this->zScale.transform (*(this->scalarData), dcopy);
                this->dcolour.resize (n);
                this->colourScale.transform (*(this->scalarData), dcolour);

            } else if (this->vectorData != nullptr) {

                // Check vector data
                if (this->vectorData->size() != n) {
                    throw std::runtime_error ("Error: size does not match vectorData size");
                }

                this->dcopy.resize (n);
                this->dcolour.resize (n);
                this->dcolour2.resize (n);
                this->dcolour3.resize (n);
                std::vector<float> veclens(dcopy);
                for (unsigned int i = 0; i < n; ++i) {
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

        //! An overridable function to set the colour of rect ri
        std::array<float, 3> setColour (size_t ri)
        {
            std::array<float, 3> clr = { 0.0f, 0.0f, 0.0f };
            if (this->cm.numDatums() == 3) {
                if constexpr (std::is_integral<std::decay_t<F>>::value) {
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

        //! Compute a triangle from 3 arbitrary corners
        void computeTriangle (vec<float> c1, vec<float> c2, vec<float> c3, std::array<float, 3> colr)
        {
            // v is the face normal
            vec<float> u1 = c1-c2;
            vec<float> u2 = c2-c3;
            vec<float> v = u1.cross(u2);
            v.renormalize();
            // Push corner vertices
            this->vertex_push (c1, this->vertexPositions);
            this->vertex_push (c2, this->vertexPositions);
            this->vertex_push (c3, this->vertexPositions);
            // Colours/normals
            for (unsigned int i = 0; i < 3U; ++i) {
                this->vertex_push (colr, this->vertexColors);
                this->vertex_push (v, this->vertexNormals);
            }
            this->indices.push_back (this->idx++);
            this->indices.push_back (this->idx++);
            this->indices.push_back (this->idx++);
        }

        //! Compute the triangulization
        void initializeVertices()
        {
            unsigned int ncoords = this->dataCoords == nullptr ? 0 : this->dataCoords->size();
            if (ncoords == 0) { return; }
            unsigned int ndata = this->scalarData == nullptr ? 0 : this->scalarData->size();
            // If we have vector data, then manipulate colour accordingly.
            unsigned int nvdata = this->vectorData == nullptr ? 0 : this->vectorData->size();

            if (ndata > 0 && ncoords != ndata) {
                std::cout << "ArbSurfaceVisual Error: ncoords ("<<ncoords<<") != ndata ("<<ndata<<"), return (no model)." << std::endl;
                return;
            }
            if (nvdata > 0 && ncoords != nvdata) {
                std::cout << "ArbSurfaceVisual Error: ncoords ("<<ncoords<<") != nvdata ("<<nvdata<<"), return (no model)." << std::endl;
                return;
            }

            this->setupScaling (this->scalarData->size());

            // Make delaunay
            // coords should be [ x0, y0, x1, y1, x2, y2 etc ]
            std::vector<double> coords2d (ncoords * 2);
            for (unsigned int i = 0; i < ncoords; ++i) {
                coords2d[i*2] = static_cast<double>((*this->dataCoords)[i][0]);
                coords2d[i*2+1] = static_cast<double>((*this->dataCoords)[i][1]);
            }
            delaunator::Delaunator d(coords2d);

            std::cout << "There are " << d.triangles.size()/3 << " triangles from Delaunator d\n";

            for (std::size_t i = 0; i < d.triangles.size(); i+=3) {
                // What's corresponding data index?
                std::size_t di = i/3;

                // We have: (*this->dataCoords)[di] (*this->scalarData)[di] and (*this->vectorData)[di]
                // And points for the triangulation

                // x, y coords of the triangles.
                morph::vec<double> t0 = { d.coords[2 * d.triangles[i]],     d.coords[2 * d.triangles[i] + 1],     (*this->dataCoords)[di][2] };
                morph::vec<double> t1 = { d.coords[2 * d.triangles[i + 1]], d.coords[2 * d.triangles[i + 1] + 1], (*this->dataCoords)[di][2] };
                morph::vec<double> t2 = { d.coords[2 * d.triangles[i + 2]], d.coords[2 * d.triangles[i + 2] + 1], (*this->dataCoords)[di][2] };


                this->computeTriangle (t0.as_float(), t1.as_float(), t2.as_float(), this->setColour(di));

                if (this->labelIndices == true) {
                    // Draw an index label...
                    this->addLabel (std::to_string (i), (*this->dataCoords)[i] + labelOffset, morph::TextFeatures(labelSize) );
                }
            }

            for (unsigned int i = 0; i < ncoords; ++i) {
                this->computeSphere ((*this->dataCoords)[i], morph::colour::crimson, 0.1f);
            }
        }

        //! A copy of the scalarData which can be transformed suitably to be the z value of the surface
        std::vector<float> dcopy;
        //! A copy of the scalarData (or first field of vectorData), scaled to be a colour value
        std::vector<float> dcolour;
        std::vector<float> dcolour2;
        std::vector<float> dcolour3;

        // Do we add index labels?
        bool labelIndices = false;
        morph::vec<float, 3> labelOffset = { 0.04f, 0.0f, 0.0f };
        float labelSize = 0.03f;
    };

} // namespace morph
