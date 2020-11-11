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

    //! What shape for the graph markers?
    enum class markerstyle
    {
        triangle,
        uptriangle,
        downtriangle,
        square,
        diamond,
        pentagon,
        hexagon,
        heptagon,
        octagon,
        circle,
        numstyles
    };

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

            this->colourScale.do_autoscale = true;
            this->zScale.do_autoscale = true;
            this->ordscale.do_autoscale = true;
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

            this->colourScale.do_autoscale = true;
            this->zScale.do_autoscale = true;
            this->ordscale.do_autoscale = true;

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
            if (this->zScale.autoscaled == false) {
                this->setgraphsize (this->scalewidth, this->scaleheight);
            }
            std::vector<Flt> sd (dsize, Flt{0});
            std::vector<Flt> od (dsize, Flt{0});

            this->zScale.transform (*this->scalarData, sd);
            this->ordscale.transform (*this->ordinalData, od);

            // Now sd and od can be used to construct dataCoords x/y. They are used to
            // set the position of each datum into dataCoords
            for (size_t i = 0; i < dsize; ++i) {
                (*this->dataCoords)[i][0] = static_cast<Flt>(od[i]);
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

            std::vector<Flt> dcopy;
            dcopy = *(this->scalarData);
            this->colourScale.do_autoscale = true;
            this->colourScale.transform (*this->scalarData, dcopy);

            // The indices index
            VBOint idx = 0;

            this->drawAxes (idx);

            // Draw data
            // for (auto i : data) {...
            if (this->showmarkers == true) {
                for (size_t i = 0; i < ncoords; ++i) {
                    this->marker (idx, (*this->dataCoords)[i], this->markerstyle);
                }
            }
            if (this->showlines == true) {
                for (size_t i = 1; i < ncoords; ++i) {
                    // Draw tube from location -1 to location 0.
                    this->computeLine (idx, (*this->dataCoords)[i-1], (*this->dataCoords)[i], uz,
                                       this->linecolour, this->linecolour,
                                       this->linewidth, this->thickness*Flt{0.7}, this->markergap);
                }
            }
        }

        //! Draw the axes for the graph
        void drawAxes (VBOint& idx)
        {
            // y axis
            this->computeLine (idx, {0, 0, -this->thickness},
                               {0, this->scaleheight, -this->thickness}, uz,
                               this->axescolour, this->axeswidth, this->thickness);
            // x axis
            this->computeLine (idx, {0, 0, -this->thickness},
                               {this->scalewidth, 0, -this->thickness}, uz,
                               this->axescolour, this->axeswidth, this->thickness);

            if (this->axesfull == true) {
                // right axis
                this->computeLine (idx, {this->scalewidth, 0, -this->thickness},
                                   {this->scalewidth, this->scaleheight, -this->thickness}, uz,
                                   this->axescolour, this->axeswidth, this->thickness);
                // top axis
                this->computeLine (idx, {0, this->scaleheight, -this->thickness},
                                   {this->scalewidth, this->scaleheight, -this->thickness}, uz,
                                   this->axescolour, this->axeswidth, this->thickness);
            }
        }

        //! Generate vertices for a marker of the given style at location p
        void marker (VBOint& idx, morph::Vector<float>& p, morph::markerstyle mstyle)
        {
            switch (mstyle) {
            case morph::markerstyle::triangle:
            case morph::markerstyle::uptriangle:
            {
                this->polygonMarker (idx, p, 3);
                break;
            }
            case morph::markerstyle::downtriangle:
            {
                this->polygonFlattop (idx, p, 3);
                break;
            }
            case morph::markerstyle::square:
            {
                this->polygonFlattop (idx, p, 4);
                break;
            }
            case morph::markerstyle::diamond:
            {
                this->polygonMarker (idx, p, 4);
                break;
            }
            case morph::markerstyle::pentagon:
            {
                this->polygonMarker (idx, p, 5);
                break;
            }
            case morph::markerstyle::hexagon:
            {
                this->polygonMarker (idx, p, 6);
                break;
            }
            case morph::markerstyle::heptagon:
            {
                this->polygonMarker (idx, p, 7);
                break;
            }
            case morph::markerstyle::octagon:
            {
                this->polygonMarker (idx, p, 8);
                break;
            }
            case morph::markerstyle::circle:
            default:
            {
                this->polygonMarker (idx, p, 20);
                break;
            }
            }
        }

    protected:
        // Create an n sided polygon with first vertex 'pointing up'
        void polygonMarker  (VBOint& idx, morph::Vector<float> p, int n)
        {
            morph::Vector<float> pend = p;
            p[2] += this->thickness*Flt{0.5};
            pend[2] -= this->thickness*Flt{0.5};
            this->computeTube (idx, p, pend, ux, uy,
                               this->markercolour, this->markercolour,
                               this->markersize*Flt{0.5}, n);
        }

        // Create an n sided polygon with a flat edge 'pointing up'
        void polygonFlattop (VBOint& idx, morph::Vector<float> p, int n)
        {
            morph::Vector<float> pend = p;
            p[2] += this->thickness*Flt{0.5};
            pend[2] -= this->thickness*Flt{0.5};
            this->computeTube (idx, p, pend, ux, uy,
                               this->markercolour, this->markercolour,
                               this->markersize*Flt{0.5}, n, morph::PI_F/(float)n);
        }

    public:
        //! A scaling for the ordinals. I'll use zCcale to scale the data values
        morph::Scale<Flt> ordscale;

        // marker features
        bool showmarkers = true;
        std::array<float, 3> markercolour = {0,0,1};
        float markersize = 0.03f;
        morph::markerstyle markerstyle = markerstyle::square;
        float markergap = 0.03f;

        // line features
        bool showlines = true;
        std::array<float, 3> linecolour = {0,0,0};
        float linewidth = 0.01f;

        // axis features
        std::array<float, 3> axescolour = {0,0,0};
        float axeswidth = 0.01f;
        //! full axes: left, bottom, top and right. Not full: left, bottom only.
        bool axesfull = true;
        //! Show gridlines where the tick lines are?
        bool showgrid = false;

        //! Set the graph size, in model units.
        void setgraphsize (float width, float height)
        {
            std::cout << __FUNCTION__ << " called\n";
            if (this->zScale.autoscaled == true) {
                throw std::runtime_error ("Have already scaled the data, can't set the scale now.\n"
                                          "Hint: call GraphVisual::setgraphsize() BEFORE GraphVisual::setdata() or ::setaxes()");
            }
            this->scalewidth = width;
            this->scaleheight = height;

            float _extra = 0.05 * this->scaleheight;
            this->zScale.range_min = _extra;
            this->zScale.range_max = this->scaleheight-_extra;

            _extra = 0.05 * this->scalewidth;
            this->ordscale.range_min = _extra;
            this->ordscale.range_max = this->scalewidth-_extra;

            this->thickness *= this->scalewidth;
        }

        // Axis ranges. The length of each axis could be determined from the data and
        // ordinates for a static graph, but for a dynamically updating graph, it's
        // going to be necessary to give a hint at how far the data/ordinates might need
        // to extend.
        void setaxes (Flt _xmin, Flt _xmax, Flt _ymin, Flt _ymax)
        {
            // First make sure that the range_min/max are correctly set
            this->setgraphsize (this->scalewidth, this->scaleheight);
            // To make the axes larger, we change the scaling that we'll apply to the
            // data (the axes are always scalewidth * scaleheight in size).
            this->zScale.compute_autoscale (_ymin, _ymax);
            this->ordscale.compute_autoscale (_xmin, _xmax);
        }

    protected:
        //! How thick are the markers, axes etc? Sort of 'paper thickness'
        float thickness = 0.005f;
        //! scalewidth scales the amount of in-model distance that the graph will be wide
        float scalewidth = 1.0f;
        //! scalewidth height scales the amount of in-model distance that the graph will be high
        float scaleheight = 1.0f;

        //! The axes for orientation of the graph visual, which is 2D within the 3D environment.
        morph::Vector<float> ux = {1,0,0};
        morph::Vector<float> uy = {0,1,0};
        morph::Vector<float> uz = {0,0,1};

        //! Data for the ordinals
        std::vector<Flt>* ordinalData;
    };

} // namespace morph
