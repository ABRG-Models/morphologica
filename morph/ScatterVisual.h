#pragma once

#include "GL3/gl3.h"
#include "tools.h"
#include "VisualDataModel.h"

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;
#include <array>
using std::array;

namespace morph {

    //! The template argument Flt is the type of the data which this ScatterVisual
    //! will visualize.
    template <class Flt>
    class ScatterVisual : public VisualDataModel<Flt>
    {
    public:
        ScatterVisual(GLuint sp,
                      vector<array<float,3>>* _coords,
                      const array<float, 3> _offset,
                      const vector<Flt>* _data,
                      const array<Flt, 2> _scale,
                      ColourMapType _cmt,
                      const float _hue = 0.0f) {
            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);
            this->colourScale.setParams (_scale[0], _scale[1]);
            this->dataCoords = _coords;
            this->scalarData = _data;

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);

            this->initializeVertices();
            this->postVertexInit();
        }

        ScatterVisual(GLuint sp,
                      vector<array<float,3>>* _coords,
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
            this->colourScale.setParams (_scale[0], _scale[1]);
            this->dataCoords = _coords;
            this->scalarData = _data;
            this->radiusFixed = fr;

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);

            this->initializeVertices();
            this->postVertexInit();
        }

        //! Compute spheres for a scatter plot
        void initializeVertices (void) {

            unsigned int ncoords = this->dataCoords->size();
            unsigned int ndata = this->scalarData->size();

            if (ndata > 0 && ncoords != ndata) {
                cout << "ncoords ("<<ncoords<<") != ndata ("<<ndata<<"), return." << endl;
                return;
            }

            // Find the minimum distance between points to get a radius? Or just allow
            // client code to set it?

            vector<Flt> dcopy;
            if (ndata) {
                dcopy = *(this->scalarData);
                this->colourScale.do_autoscale = true;
                this->colourScale.transform (*this->scalarData, dcopy);
            } // else no scaling required - spheres will be one colour

            // The indices index
            GLushort idx = 0;

            for (unsigned int i = 0; i < ncoords; ++i) {
                // Scale colour (or use single colour)
                array<float, 3> clr = this->cm.getHueRGB();
                if (ndata) {
                    clr = this->cm.convert (dcopy[i]);
                }
                this->computeSphere (idx, (*this->dataCoords)[i], clr, this->radiusFixed);
            }
        }

        //! Set this->radiusFixed, then re-compute vertices.
        void setRadius (float fr) {
            this->radiusFixed = fr;
            this->reinit();
        }

    private:

        //! Change this to get larger or smaller spheres.
        Flt radiusFixed = 0.05;
    };

} // namespace morph
