/*!
 * \file GraphVisual
 *
 * \author Seb James
 * \date 2020
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

    /*
     * So you want to graph some data? You have an ordinal and data. Although these
     * could provide coordinates for graphing the data, it's possible that they may be
     * wide ranging. Much better to scale the data to be in the range [0,1].
     *
     * I plan to derive from GraphVisual specialized types that mean user doesn't have
     * to set all the parameters each time. E.g. LineGraphVisual, MarkerGraphVisual,
     * FatLineGraphVisual, etc.
     */
    template <typename Flt>
    class GraphVisual : public VisualDataModel<Flt>
    {
    public:
        //! Constructor which sets just the shader program and the model view offset
        GraphVisual(GLuint sp, const Vector<float> _offset)
        {
            this->shaderprog = sp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
        }

        //! Long constructor demonstrating what needs to be set before setup() is called.
        GraphVisual(GLuint sp,
                    const Vector<float> _offset,
                    std::vector<Flt>& _ordinals,
                    std::vector<Flt>& _data,
                    const Scale<Flt>& _ord_scale,
                    const Scale<Flt>& _data_scale,
                    ColourMapType _cmt,
                    const float _hue = 0.0f,
                    const float _sat = 1.0f)
        {
            this->shaderprog = sp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);

            this->colourScale = _data_scale;

            this->setdata (_ordinals, _data);

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);

            this->setup();
        }

        //! Fixme: Will quickly want to plot multiple datasets for an ordinal
        void setdata (std::vector<Flt>& _ordinals, std::vector<Flt>& _data)
        {
            if (_ordinals.size() != _data.size()) {
                throw std::runtime_error ("size mismatch");
            }

            size_t dsize = _data.size();

            if (this->dataCoords == (std::vector<morph::Vector<float>>*)0) {
                this->dataCoords = new std::vector<morph::Vector<float>>(dsize, {0,0,0});
            } else {
                this->dataCoords->resize (dsize);
            }

            // Copy the addresses of the raw incoming data and save in scalarData and ordinalData
            this->scalarData = &_data;
            this->ordinalData = &_ordinals;

            // Scale the incoming data?
            std::vector<Flt> sd (dsize, Flt{0});
            std::vector<Flt> od (dsize, Flt{0});
            this->zScale.transform (*this->scalarData, sd);
            this->ordscale.transform (*this->ordinalData, od);

            // Now sd and od can be used to construct dataCoords x/y. They are used to
            // set the position of each datum into dataCoords
            for (size_t i = 0; i < dsize; ++i) {
                (*this->dataCoords)[i][0] = static_cast<Flt>(od[i]); // crashes here, but usually after build, on mac
                (*this->dataCoords)[i][1] = static_cast<Flt>(sd[i]);
                (*this->dataCoords)[i][2] = Flt{0};
            }
        }

        //! A setup program for the client code. Client code can set up a GraphVisual
        //! object, set the data, Scales and colour map, and then call
        //! GraphVisual::setup()
        void setup()
        {
            this->initializeVertices();
            this->postVertexInit();
        }

        //! Compute stuff for a graph
        void initializeVertices()
        {
            size_t ncoords = this->dataCoords->size();

            // Find the minimum distance between points to get a radius? Or just allow
            // client code to set it?

            std::vector<Flt> dcopy;
            dcopy = *(this->scalarData);
            this->colourScale.do_autoscale = true;
            this->colourScale.transform (*this->scalarData, dcopy);

            // The indices index
            VBOint idx = 0;

            float rotation = morph::PI_F/4.0f;
            float thickness = 0.005f;
            morph::Vector<float> ux = {1,0,0};
            morph::Vector<float> uy = {0,1,0};
            morph::Vector<float> uz = {0,0,1};
            if (this->showmarkers == true) {
                for (size_t i = 0; i < ncoords; ++i) {

                    morph::Vector<float> pstart = (*this->dataCoords)[i];
                    morph::Vector<float> pend = pstart;
                    pstart[2] += thickness*Flt{0.5};
                    pend[2] -= thickness*Flt{0.5};
                    this->computeTube (idx, pstart, pend, ux, uy,
                                       this->markercolour, this->markercolour,
                                       this->markersize*Flt{0.5}, 4, rotation);
                }
            }
            if (this->showlines == true) {
                for (size_t i = 1; i < ncoords; ++i) {
                    // Draw tube from location -1 to location 0
                    this->computeLine (idx, (*this->dataCoords)[i-1], (*this->dataCoords)[i], uz,
                                       this->linecolour, this->linecolour,
                                       this->linewidth, thickness*Flt{0.7}, this->markersize);
                }
            }

        }

        //! Change marker size.
        void changeMarkersize (float ms)
        {
            this->markersize = ms;
            this->reinit();
        }

        //! Change line width
        void changeLinewidth (float lw)
        {
            this->linewidth = lw;
            this->reinit();
        }

        // A note on naming: I'm avoiding capitals in the parts of the GraphVisual API that are public.

        //! A scaling for the ordinals. I'll use zscale to scale the data values
        morph::Scale<Flt> ordscale;

        std::array<Flt, 3> markercolour = {0,0,0};
        std::array<Flt, 3> linecolour = {0,0,0};

        //! Graph features
        bool showmarkers = true;
        bool showlines = true;
        //! Change this to get larger or smaller spheres.
        Flt markersize = 0.05;
        Flt linewidth = 0.01;
        // Add linestyles too.

    protected:
        //! Data for the ordinals
        std::vector<Flt>* ordinalData;
    };

} // namespace morph
