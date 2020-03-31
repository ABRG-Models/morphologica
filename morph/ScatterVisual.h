#ifndef _SCATTERVISUAL_H_
#define _SCATTERVISUAL_H_

#include "GL3/gl3.h"

#include "tools.h"

#include "VisualModel.h"

#include "MathAlgo.h"

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;
#include <array>
using std::array;

namespace morph {

    //! The template argument Flt is the type of the data which this ScatterVisual will visualize.
    template <class Flt>
    class ScatterVisual : public VisualModel
    {
    public:
        ScatterVisual(GLuint sp,
                      const vector<array<Flt,3>>* _coords,
                      const array<float, 3> _offset,
                      const vector<Flt>* _data,
                      const array<Flt, 2> _scale,
                      ColourMapType _cmt,
                      const float _hue = 0.0f) {
            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);
            this->scale = _scale;
            this->coords = _coords;
            this->data = _data;

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);

            this->initializeVertices();
            this->postVertexInit();
        }

        ScatterVisual(GLuint sp,
                      const vector<array<Flt,3>>* _coords,
                      const array<float, 3> _offset,
                      const vector<Flt>* _data,
                      const float fr,
                      const array<Flt, 2> _scale,
                      ColourMapType _cmt,
                      const float _hue = 0.0f) {
            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);
            this->scale = _scale;
            this->coords = _coords;
            this->data = _data;
            this->radiusFixed = fr;

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);

            this->initializeVertices();
            this->postVertexInit();
        }

        //! Do the computations to initialize the vertices that will represent the Scatter.
        void initializeVertices (void) {

            unsigned int ncoords = this->coords->size();
            unsigned int ndata = this->data->size();

            if (ndata > 0 && ncoords != ndata) {
                cout << "ncoords ("<<ncoords<<") != ndata ("<<ndata<<"), return." << endl;
                return;
            }

            // Find the minimum distance between points to get a radius? Or just allow client code to set it?

            vector<Flt> dcopy;
            if (ndata) {
                dcopy = *(this->data);
                if (this->scale[0] == 0.0f && this->scale[1] == 0.0f) {
                    // Special 0,0 scale means auto scale data
                    dcopy = MathAlgo<Flt>::autoscale (dcopy);
                    this->scale[0] = 1.0f;
                }
            } // no scaling required - spheres will be one colour

            // The indices index
            GLushort idx = 0;

            for (unsigned int i = 0; i < ncoords; ++i) {
                // Scale colour (or use single colour)
                array<float, 3> clr = this->cm.getHueRGB();
                if (ndata) {
                    Flt datum = dcopy[i] * this->scale[0] + this->scale[1];
                    datum = datum > static_cast<Flt>(1.0) ? static_cast<Flt>(1.0) : datum;
                    datum = datum < static_cast<Flt>(0.0) ? static_cast<Flt>(0.0) : datum;
                    // And turn it into a colour:
                    clr = this->cm.convert(datum);
                }
                this->computeSphere (idx, (*this->coords)[i], clr, this->radiusFixed);
            }
        }

        //! Update the data and/or coords and re-compute the vertices.
        //@{
        void updateData (const vector<Flt>* _data, const array<Flt, 2> _scale) {
            this->scale = _scale;
            this->data = _data;
            this->reinit();
        }
        void updateData (const vector<array<Flt, 3>>* _coords,
                         const vector<Flt>* _data, const array<Flt, 2> _scale) {
            this->coords = _coords;
            this->scale = _scale;
            this->data = _data;
            this->reinit();
        }
        void updateCoords (const vector<array<Flt, 3>>* _coords) {
            this->coords = _coords;
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

        //! The linear scaling for the colour is y1 = m1 x + c1 (m1 = scale[0] and c1 = scale[1]) If
        //! all entries of scale are static_cast<Flt>(0), then auto-scale.
        array<Flt, 2> scale;

        //! The relevant colour map. Change the type/hue of this colour map object to generate
        //! different types of map.
        ColourMap<Flt> cm;

        //! Set this->radiusFixed, then re-compute vertices.
        void setRadius (float fr) {
            this->radiusFixed = fr;
            this->reinit();
        }

    private:

        //! Change this to get larger or smaller spheres.
        Flt radiusFixed = 0.05;

        //! The coordinates of the points to visualize. A sphere at each coordinate. Sphere radius
        //! computed as 10% of the minimum distance between coordinates.
        const vector<array<Flt,3>>* coords;

        //! The data to visualize as colour (modulated by the linear scaling provided in
        //! this->scale)
        const vector<Flt>* data;
    };

} // namespace morph

#endif // _SCATTERVISUAL_H_
