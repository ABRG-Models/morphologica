/*!
 * \file
 *
 * \author Seb James
 * \date 2019
 */
#pragma once

#include <morph/tools.h>
#include <morph/VisualDataModel.h>
#include <morph/Scale.h>
#include <morph/vec.h>
#include <iostream>
#include <vector>
#include <array>

namespace morph {

    //! The template argument Flt is the type of the data which this ScatterVisual
    //! will visualize.
    template <typename Flt, int glver = morph::gl::version_4_1>
    class ScatterVisual : public VisualDataModel<Flt, glver>
    {
    public:
        ScatterVisual(const vec<float> _offset)
        {
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->zScale.setParams (1, 0);
            this->colourScale.do_autoscale = true;
        }

        //! Quick hack to add an additional point
        void add (morph::vec<float> coord, Flt value)
        {
            std::array<float, 3> clr = this->cm.convert (this->colourScale.transform_one (value));
            this->computeSphere (this->idx, coord, clr, this->radiusFixed, 16, 20);
            this->reinit_buffers();
        }
        //! Additional point with variable size
        void add (morph::vec<float> coord, Flt value, Flt size)
        {
            std::array<float, 3> clr = this->cm.convert (this->colourScale.transform_one (value));
            this->computeSphere (this->idx, coord, clr, size, 16, 20);
            this->reinit_buffers();
        }

        //! Compute spheres for a scatter plot
        void initializeVertices()
        {
            unsigned int ncoords = this->dataCoords == nullptr ? 0 : this->dataCoords->size();
            if (ncoords == 0) { return; }
            unsigned int ndata = this->scalarData == nullptr ? 0 : this->scalarData->size();
            // If we have vector data, then manipulate colour accordingly.
            unsigned int nvdata = this->vectorData == nullptr ? 0 : this->vectorData->size();

            if (ndata > 0 && ncoords != ndata) {
                std::cout << "ScatterVisual Error: ncoords ("<<ncoords<<") != ndata ("<<ndata<<"), return (no model)." << std::endl;
                return;
            }
            if (nvdata > 0 && ncoords != nvdata) {
                std::cout << "ScatterVisual Error: ncoords ("<<ncoords<<") != nvdata ("<<nvdata<<"), return (no model)." << std::endl;
                return;
            }

            // Find the minimum distance between points to get a radius? Or just allow
            // client code to set it?

            std::vector<Flt> dcopy;
            std::vector<Flt> vdcopy1;
            std::vector<Flt> vdcopy2;
            std::vector<Flt> vdcopy3;
            if (ndata && !nvdata) {
                dcopy = *(this->scalarData);
                this->colourScale.do_autoscale = true;
                this->colourScale.transform (*this->scalarData, dcopy);
            } else if (nvdata) {
                vdcopy1.resize(this->vectorData->size());
                vdcopy2.resize(this->vectorData->size());
                vdcopy3.resize(this->vectorData->size());

                std::vector<Flt> dcopy2, dcopy3;
                dcopy.resize(this->vectorData->size());
                dcopy2.resize(this->vectorData->size());
                dcopy3.resize(this->vectorData->size());

                for (unsigned int i = 0; i < this->vectorData->size(); ++i) {
                    dcopy[i] = (*this->vectorData)[i][0];
                    dcopy2[i] = (*this->vectorData)[i][1];
                    dcopy3[i] = (*this->vectorData)[i][2];
                }

                this->colourScale.do_autoscale = true;
                this->colourScale2.do_autoscale = true;
                this->colourScale3.do_autoscale = true;

                this->colourScale.transform (dcopy, vdcopy1);
                this->colourScale2.transform (dcopy2, vdcopy2);
                this->colourScale3.transform (dcopy3, vdcopy3);

            } // else no scaling required - spheres will be one colour

            for (unsigned int i = 0; i < ncoords; ++i) {
                // Scale colour (or use single colour)
                std::array<float, 3> clr = this->cm.getHueRGB();
                if (ndata && !nvdata) {
                    clr = this->cm.convert (dcopy[i]);
                } else if (nvdata) {
                    // Combine colour from two values. vdcopy1, vdcopy2? OR just do RGB for now?
                    // ColourMap in 'dual hue' (or triple hue) mode.
                    //std::cout << "Convert colour from vdcopy1[i]: " << vdcopy1[i] << ", vdcopy2[i]: " << vdcopy2[i] << std::endl;
                    clr = this->cm.convert (vdcopy1[i], vdcopy2[i]);
                }
                if (this->sizeFactor == Flt{0}) {
                    if constexpr (draw_spheres_as_geodesics) {
                        // Slower than regular computeSphere(). 2 iterations gives 320 faces
                        this->template computeSphereGeoFast<float, 2> (this->idx, (*this->dataCoords)[i], clr, this->radiusFixed);
                    } else {
                        // (16+2) * 20 gives 360 faces
                        this->computeSphere (this->idx, (*this->dataCoords)[i], clr, this->radiusFixed, 16, 20);
                    }
                } else {
                    if constexpr (draw_spheres_as_geodesics) {
                        this->template computeSphereGeoFast<float, 2> (this->idx, (*this->dataCoords)[i], clr, dcopy[i]*this->sizeFactor);
                    } else {
                        this->computeSphere (this->idx, (*this->dataCoords)[i], clr, dcopy[i]*this->sizeFactor, 16, 20);
                    }
                }

                if (this->labelIndices == true) {
                    // Draw an index label...
                    this->addLabel (std::to_string (i), (*this->dataCoords)[i] + labelOffset, morph::TextFeatures(labelSize) );
                }
            }
        }

        // The constexpr, unordered geodesic code is no slower than the regular
        // VisualModel::computeSphere(), but leave this off for now (if true, C++-20 is
        // required)
        static constexpr bool draw_spheres_as_geodesics = false;

        //! Set this->radiusFixed, then re-compute vertices.
        void setRadius (float fr)
        {
            this->radiusFixed = fr;
            this->reinit();
        }

        //! Change this to get larger or smaller spheres.
        Flt radiusFixed = Flt{0.05};
        Flt sizeFactor = Flt{0};

        // Hues for colour control with vectorData
        float hue1 = 0.1f;
        float hue2 = 0.5f;
        float hue3 = -1.0f;

        // Do we add index labels?
        bool labelIndices = false;

        morph::vec<float, 3> labelOffset = { 0.04f, 0.0f, 0.0f };
        float labelSize = 0.03f;
    };

} // namespace morph
