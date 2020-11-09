/*!
 * \file
 *
 * \author Seb James
 * \date 2019
 */
#pragma once

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#include <morph/tools.h>
#include <morph/VisualDataModel.h>
#include <morph/Scale.h>
#include <morph/Vector.h>
#include <iostream>
#include <vector>
#include <array>

namespace morph {

    //! The template argument Flt is the type of the data which this ScatterVisual
    //! will visualize.
    template <typename Flt>
    class ScatterVisual : public VisualDataModel<Flt>
    {
    public:
        ScatterVisual(GLuint sp,
                      std::vector<Vector<float,3>>* _coords,
                      const Vector<float, 3> _offset,
                      const std::vector<Flt>* _data,
                      const Scale<Flt>& _scale,
                      ColourMapType _cmt,
                      const float _hue = 0.0f) {
            // Set up...
            this->shaderprog = sp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->colourScale = _scale;
            this->dataCoords = _coords;
            this->scalarData = _data;

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);

            this->initializeVertices();
            this->postVertexInit();
        }

        ScatterVisual(GLuint sp,
                      std::vector<Vector<float,3>>* _coords,
                      const Vector<float, 3> _offset,
                      const std::vector<Flt>* _data,
                      const float fr,
                      const Scale<Flt>& _scale,
                      ColourMapType _cmt,
                      const float _hue = 0.0f) {
            // Set up...
            this->shaderprog = sp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->colourScale = _scale;
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
                std::cout << "ScatterVisual Error: ncoords ("<<ncoords<<") != ndata ("<<ndata<<"), return (no model)." << std::endl;
                return;
            }

            // Find the minimum distance between points to get a radius? Or just allow
            // client code to set it?

            std::vector<Flt> dcopy;
            if (ndata) {
                dcopy = *(this->scalarData);
                this->colourScale.do_autoscale = true;
                this->colourScale.transform (*this->scalarData, dcopy);
            } // else no scaling required - spheres will be one colour

            // The indices index
            VBOint idx = 0;

            for (unsigned int i = 0; i < ncoords; ++i) {
                // Scale colour (or use single colour)
                std::array<float, 3> clr = this->cm.getHueRGB();
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
