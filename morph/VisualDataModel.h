/*!
 * VisualModels which have data.
 */
#pragma once

#include <vector>
using std::vector;
#include "VisualModel.h"
using morph::VisualModel;
#include "ColourMap.h"
using morph::ColourMap;
#include "Scale.h"
using morph::Scale;

namespace morph {

    //! Class for VisualModels that visualize data of type T. T is probably float or
    //! double, but may be integer types, too.
    template <typename T>
    class VisualDataModel : public VisualModel
    {
    public:
        VisualDataModel()
            : morph::VisualModel::VisualModel() {
        }
        VisualDataModel (GLuint sp, const array<float, 3> _offset)
            : morph::VisualModel::VisualModel (sp, _offset) {
        }
        ~VisualDataModel() {
        }

        //! Methods to update the scalarData, vectorData, dataCoords and/or scaling and
        //! re-compute the vertices.
        //@{
        void updateData (const vector<T>* _data) {
            this->scalarData = _data;
            this->reinit();
        }

        void updateData (const vector<T>* _data,
                         const array<T, 2>& zscale) {
            this->scalarData = _data;
            this->zScale.setParams (zscale[0], zscale[1]);
            this->reinit();
        }
        void updateData (const vector<T>* _data,
                         const array<T, 2>& zscale,
                         const array<T, 2>& cscale) {
            this->scalarData = _data;
            this->zScale.setParams (zscale[0], zscale[1]);
            this->colourScale.setParams (cscale[0], cscale[1]);
            this->reinit();
        }
        void updateData (const vector<T>* _data,
                         const array<T, 4>& _scale) {
            this->scalarData = _data;
            this->zScale.setParams (_scale[0], _scale[1]);
            this->colourScale.setParams (_scale[2], _scale[3]);
            this->reinit();
        }

        virtual void updateData (vector<array<T, 3>>* _coords,
                                 const vector<T>* _data,
                                 const array<T, 2> zscale) {
            this->dataCoords = _coords;
            this->scalarData = _data;
            this->zScale.setParams (zscale[0], zscale[1]);
            this->reinit();
        }
        virtual void updateData (vector<array<T, 3>>* _coords,
                                 const vector<T>* _data,
                                 const array<T, 2> zscale,
                                 const array<T, 2> cscale) {
            this->dataCoords = _coords;
            this->scalarData = _data;
            this->zScale.setParams (zscale[0], zscale[1]);
            this->colourScale.setParams (cscale[0], cscale[1]);
            this->reinit();
        }
        virtual void updateData (vector<array<T, 3>>* _coords,
                                 const vector<T>* _data,
                                 const array<T, 4> _scale) {
            this->dataCoords = _coords;
            this->scalarData = _data;
            this->zScale.setParams (_scale[0], _scale[1]);
            this->colourScale.setParams (_scale[2], _scale[3]);
            this->reinit();
        }

        virtual void updateCoords (vector<array<T, 3>>* _coords) {
            this->dataCoords = _coords;
            this->reinit();
        }

        void updateData (const vector<array<T, 3>>* _vectors) {
            this->vectorData = _vectors;
            this->reinit();
        }
        void updateData (const vector<array<T, 3>>* _coords,
                         const vector<array<T, 3>>* _vectors) {
            this->dataCoords = _coords;
            this->vectorData = _vectors;
            this->reinit();
        }
        //@}

        void reinit (void) {
            // Fixme: Better not to clear, then repeatedly pushback here:
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->initializeVertices();
            // Now re-set up the VBOs
            int sz = this->indices.size() * sizeof(VBOint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, colLoc);
        }

        //! All data models use a a colour map. Change the type/hue of this colour map
        //! object to generate different types of map.
        ColourMap<T> cm;

        //! A Scaling function for the colour map. Perhaps a Scale class contains a
        //! colour map? If not, then this scale might well be autoscaled.
        Scale<T> colourScale;

        //! A scale to scale (or autoscale) scalarData. This might be used to set z
        //! locations of data coordinates based on scalarData. The scaling may
        Scale<T> zScale;

        //! A scaling function for the vectorData. Note it's a scalar scaling; scaling
        //! of the actual vectorData will have to be carried out manually.
        Scale<array<T,3>> vectorScale;
        //Scale<T> vectorScale;

        //! The data to visualize. Tdata may simply be float or double, or, if the
        //! visualization is of directional information, such as in a quiver plot,
        //! Tdata may be an array<float, 3> or array<double, 3>) etc. It's up to the
        const vector<T>* scalarData;

        //! A container for vector data to visualize.
        const vector<array<T,3>>* vectorData;

        //! The coordinates at which to visualize data, if appropriate (e.g. scatter
        //! graph, quiver plot). Note fixed type of float, which is suitable for
        //! OpenGL coordinates.
        vector<array<float, 3>>* dataCoords;
    };

} // namespace morph
