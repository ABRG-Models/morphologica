/*!
 * VisualModels which have data.
 */
#pragma once

#include <vector>
#include "morph/Vector.h"
#include "morph/VisualModel.h"
#include "morph/ColourMap.h"
#include "morph/Scale.h"

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
        VisualDataModel (GLuint sp, const Vector<float> _offset)
            : morph::VisualModel::VisualModel (sp, _offset) {
        }
        ~VisualDataModel() {
        }

        //! Reset the autoscaled flags so that the next time data is transformed by
        //! the Scale objects they will autoscale again (assuming they have
        //! do_autoscale set true).
        void clearAutoscale() {
            this->zScale.autoscaled = false;
            this->colourScale.autoscaled = false;
            this->vectorScale.autoscaled = false;
        }

        void clearAutoscaleZ() {
            this->zScale.autoscaled = false;
        }
        void clearAutoscaleColour() {
            this->colourScale.autoscaled = false;
        }
        void clearAutoscaleVector() {
            this->vectorScale.autoscaled = false;
        }

        void setZScale (const Scale<T>& zscale) {
            this->zScale = zscale;
            this->reinit();
        }
        void setCScale (const Scale<T>& cscale) {
            this->colourScale = cscale;
            this->reinit();
        }
        void setVectorScale (const Scale<Vector<T>>& vscale) {
            this->vectorScale = vscale;
            this->reinit();
        }

        //! Methods to update the scalarData, vectorData, dataCoords and/or scaling and
        //! re-compute the vertices.
        //@{
        void updateData (const std::vector<T>* _data) {
            this->scalarData = _data;
            this->reinit();
        }
        void updateData (const std::vector<T>* _data, const Scale<T>& zscale) {
            this->scalarData = _data;
            this->zScale = zscale;
            this->reinit();
        }
        void updateData (const std::vector<T>* _data, const Scale<T>& zscale, const Scale<T>& cscale) {
            this->scalarData = _data;
            this->zScale = zscale;
            this->colourScale = cscale;
            this->reinit();
        }
        virtual void updateData (std::vector<Vector<T>>* _coords, const std::vector<T>* _data,
                                 const Scale<T>& zscale) {
            this->dataCoords = _coords;
            this->scalarData = _data;
            this->zScale = zscale;
            this->reinit();
        }
        virtual void updateData (std::vector<Vector<T>>* _coords, const std::vector<T>* _data,
                                 const Scale<T>& zscale, const Scale<T>& cscale) {
            this->dataCoords = _coords;
            this->scalarData = _data;
            this->zScale = zscale;
            this->colourScale = cscale;
            this->reinit();
        }
        virtual void updateCoords (std::vector<Vector<T>>* _coords) {
            this->dataCoords = _coords;
            this->reinit();
        }
        void updateData (const std::vector<Vector<T>>* _vectors) {
            this->vectorData = _vectors;
            this->reinit();
        }
        void updateData (std::vector<Vector<T>>* _coords, const std::vector<Vector<T>>* _vectors) {
            this->dataCoords = _coords;
            this->vectorData = _vectors;
            this->reinit();
        }
        //@}

        void reinit() {
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

        //! A scaling function for the vectorData. This will scale the lengths of the
        //! vectorData.
        Scale<Vector<T>> vectorScale;

        //! The data to visualize. T may simply be float or double, or, if the
        //! visualization is of directional information, such as in a quiver plot,
        const std::vector<T>* scalarData;

        //! A container for vector data to visualize.
        const std::vector<Vector<T>>* vectorData;

        //! The coordinates at which to visualize data, if appropriate (e.g. scatter
        //! graph, quiver plot). Note fixed type of float, which is suitable for
        //! OpenGL coordinates.
        std::vector<Vector<float>>* dataCoords;
    };

} // namespace morph
