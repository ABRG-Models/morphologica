#pragma once

#include <morph/tools.h>
#include <morph/VisualDataModel.h>
#include <morph/MathAlgo.h>
#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/ColourMap.h>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>

namespace morph {

    /*!
     * The template argument Flt is the type of the data which this PointRowsMeshVisual
     * will visualize.
     *
     * Render a triangle made of 3 rods, with spheres at the vertices.
     *
     * \param sp The shader program
     *
     * \param _offset The offset within the morph::Visual scene at which the model will
     * be drawn (used when rendering, not when creating the model's vertices)
     */
    template <typename Flt, int glver = morph::gl::version_4_1>
    class TriFrameVisual : public VisualDataModel<Flt, glver>
    {
    public:
        TriFrameVisual(const vec<float, 3> _offset)
        {
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
        }

        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();
            this->idx = 0;

            unsigned int ncoords = this->dataCoords->size();
            unsigned int ndata = this->scalarData->size();

            std::vector<Flt> dcopy;
            if (ndata) {
                dcopy = *(this->scalarData);
                this->colourScale.do_autoscale = true;
                this->colourScale.transform (*this->scalarData, dcopy);
            } // else no scaling required - spheres will be one colour

            // Draw spheres
            for (unsigned int i = 0U; i < ncoords; ++i) {
                this->computeSphere ((*this->dataCoords)[i], this->cm.convert ((*this->scalarData)[i]), sradius);
            }
            // Draw tubes
            std::array<float, 3> clr = {0.3f,0.3f,0.3f};
            for (unsigned int i = 0U; i < ncoords; ++i) {
                morph::vec<float> v1 = (*this->dataCoords)[i];
                unsigned int e = (i < (ncoords-1) ? i+1 : 0);
                morph::vec<float> v2 = (*this->dataCoords)[e];
                this->computeTube (this->mv_offset+v1, this->mv_offset+v2, clr, clr, this->radius, this->tseg);
            }
        }

        //! tube radius
        float radius = 0.05f;
        //! sphere radius
        float sradius = 0.052f;
        //! sphere rings
        int srings = 10;
        //! sphere segments
        int sseg = 12;
        //! tube segments
        int tseg = 12;
        //! A colour map for the spheres
        morph::ColourMap<float> cm_sph;
    };

} // namespace morph
