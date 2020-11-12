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
#include <morph/VisualTextModel.h>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <sstream>

namespace morph {

    //! What shape for the graph markers?
    enum class markerstyle
    {
        none,
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

    enum class tickstyle
    {
        ticksin,
        ticksout,
        numstyles
    };

    //! Different axis styles
    enum class axisstyle
    {
        L,          // just left and bottom axis bars
        box,        // left, right, top and bottom bars, ticks only on left and bottom bars
        boxfullticks, // left, right, top and bottom bars, with ticks all round
        cross,      // a cross of bars at the zero axes
        boxcross,   // A box AND the zero axes
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
    class GraphVisual : public VisualDataModel<Flt> // Might need 'VisualGraphDataModel'
    {
    public:
        //! Constructor which sets just the shader program and the model view offset
        GraphVisual(GLuint sp, GLuint tsp, const Vector<float> _offset)
        {
            this->shaderprog = sp;
            this->tshaderprog = tsp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);

            this->colourScale.do_autoscale = true;
            this->zScale.do_autoscale = true;
            this->abscissa_scale.do_autoscale = true;
        }

        //! Long constructor demonstrating what needs to be set before setup() is called.
        GraphVisual(GLuint sp, GLuint tsp,
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
            this->tshaderprog = tsp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);

            this->colourScale.do_autoscale = true;
            this->zScale.do_autoscale = true;
            this->abscissa_scale.do_autoscale = true;

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
                this->setsize (this->scalewidth, this->scaleheight);
            }
            std::vector<Flt> sd (dsize, Flt{0});
            std::vector<Flt> od (dsize, Flt{0});

            this->zScale.transform (*this->scalarData, sd);
            this->abscissa_scale.transform (*this->ordinalData, od);

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

        //! Set the graph size, in model units.
        void setsize (float width, float height)
        {
            std::cout << __FUNCTION__ << " called\n";
            if (this->zScale.autoscaled == true) {
                throw std::runtime_error ("Have already scaled the data, can't set the scale now.\n"
                                          "Hint: call GraphVisual::setsize() BEFORE GraphVisual::setdata() or ::setlimits()");
            }
            this->scalewidth = width;
            this->scaleheight = height;

            float _extra = this->dataaxisdist * this->scaleheight;
            this->zScale.range_min = _extra;
            this->zScale.range_max = this->scaleheight-_extra;

            _extra = this->dataaxisdist * this->scalewidth;
            this->abscissa_scale.range_min = _extra;
            this->abscissa_scale.range_max = this->scalewidth-_extra;

            this->thickness *= this->scalewidth;
        }

        // Axis ranges. The length of each axis could be determined from the data and
        // abscissas for a static graph, but for a dynamically updating graph, it's
        // going to be necessary to give a hint at how far the data/abscissas might need
        // to extend.
        void setlimits (Flt _xmin, Flt _xmax, Flt _ymin, Flt _ymax)
        {
            // First make sure that the range_min/max are correctly set
            this->setsize (this->scalewidth, this->scaleheight);
            // To make the axes larger, we change the scaling that we'll apply to the
            // data (the axes are always scalewidth * scaleheight in size).
            this->zScale.compute_autoscale (_ymin, _ymax);
            this->abscissa_scale.compute_autoscale (_xmin, _xmax);
        }

        //! Set the 'object thickness' attribute (maybe used just for 'object spacing')
        void setthickness (float th) { this->thickness = th; }

    protected:
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
            if (this->markerstyle != markerstyle::none) {
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

            this->addTickLabels();
            this->addAxisLabels();
        }

        //! Add the axis labels
        void addAxisLabels()
        {
            // x axis label (easy)
            morph::VisualTextModel* lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, 100);
            morph::TextGeometry geom = lbl->getTextGeometry (this->xlabel);
            morph::Vector<float> lblpos;
            if (this->axisstyle == axisstyle::cross) {
                std::cout << "cross axis labels\n";
                float _y0_mdl = this->zScale.transform_one (0);
                lblpos = {0.5f*(this->abscissa_scale.range_max - this->abscissa_scale.inverse_one(0))-geom.half_width(),
                          _y0_mdl-(this->axislabelgap+geom.height()),
                          0};
            } else {
                lblpos = {0.5f*(this->abscissa_scale.range_max-this->abscissa_scale.range_min)-geom.half_width(),
                          -(this->axislabelgap+geom.height()),
                          0};
            }
            lbl->setupText (this->xlabel, lblpos+this->mv_offset);
            this->texts.push_back (lbl);

            // y axis label (have to rotate)
        }

        //! Add the tick labels: 0, 1, 2 etc
        void addTickLabels()
        {
            float x_for_yticks = 0.0f;
            float y_for_xticks = 0.0f;
            if (this->axisstyle == axisstyle::cross) {
                // Then labels go next to the zero axes
                x_for_yticks = this->abscissa_scale.transform_one (0);
                y_for_xticks = this->zScale.transform_one (0);
            }

            for (unsigned int i = 0; i < this->xtick_posns.size(); ++i) {

                // Omit the 0 for 'cross' axes (or maybe shift its position)
                if (this->axisstyle == axisstyle::cross && this->xticks[i] == 0) { continue; }

                std::stringstream ss;
                ss << this->xticks[i];
                // Issue: I need the width of the text ss.str() before I can create the
                // VisualTextModel, so need a static method like this:
                morph::VisualTextModel* lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, 100);
                morph::TextGeometry geom = lbl->getTextGeometry (ss.str());
                morph::Vector<float> lblpos = {this->xtick_posns[i]-geom.half_width(), y_for_xticks-(this->ticklabelgap+geom.height()), 0};
                lbl->setupText (ss.str(), lblpos+this->mv_offset);
                this->texts.push_back (lbl);
            }
            for (unsigned int i = 0; i < this->ytick_posns.size(); ++i) {

                // Omit the 0 for 'cross' axes (or maybe shift its position)
                if (this->axisstyle == axisstyle::cross && this->yticks[i] == 0) { continue; }

                std::stringstream ss;
                ss << this->yticks[i];
                morph::VisualTextModel* lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, 100);
                morph::TextGeometry geom = lbl->getTextGeometry (ss.str());
                morph::Vector<float> lblpos = {x_for_yticks-this->ticklabelgap-geom.width(), this->ytick_posns[i]-geom.half_height(), 0};
                lbl->setupText (ss.str(), lblpos+this->mv_offset);
                this->texts.push_back (lbl);
            }
        }

        void drawCrossAxes (VBOint& idx)
        {
            // Vert zero is not at model(0,0), have to get model coords of data(0,0)
            float _x0_mdl = this->abscissa_scale.transform_one (0);
            float _y0_mdl = this->zScale.transform_one (0);
            this->computeLine (idx,
                               {_x0_mdl, -(this->axislinewidth*0.5f),                  -this->thickness},
                               {_x0_mdl, this->scaleheight+(this->axislinewidth*0.5f), -this->thickness},
                               uz, this->axiscolour, this->axislinewidth*0.7f, this->thickness);
            // Horz zero
            this->computeLine (idx,
                               {0,                _y0_mdl, -this->thickness},
                               {this->scalewidth, _y0_mdl, -this->thickness},
                               uz, this->axiscolour, this->axislinewidth*0.7f, this->thickness);

            for (auto xt : this->xtick_posns) {
                // Want to place lines in screen units. So transform the data units
                this->computeLine (idx,
                                   {(float)xt, _y0_mdl, -this->thickness},
                                   {(float)xt, _y0_mdl - this->ticklength,   -this->thickness}, uz,
                                   this->axiscolour, this->axislinewidth*0.5f, this->thickness);
            }
            for (auto yt : this->ytick_posns) {
                this->computeLine (idx,
                                   {_x0_mdl,      (float)yt, -this->thickness},
                                   {_x0_mdl - this->ticklength, (float)yt, -this->thickness}, uz,
                                   this->axiscolour, this->axislinewidth*0.5f, this->thickness);
            }
        }

        //! Draw the axes for the graph
        void drawAxes (VBOint& idx)
        {
            // First, ensure that this->xtick_posns/xticks and this->ytick_posns/yticks are populated
            this->computeTickPositions();

            if (this->axisstyle == axisstyle::cross) {
                return this->drawCrossAxes (idx);
            }

            if (this->axisstyle == axisstyle::box
                || this->axisstyle == axisstyle::boxfullticks
                || this->axisstyle == axisstyle::boxcross
                || this->axisstyle == axisstyle::L) {

                // y axis
                this->computeLine (idx,
                                   {0, -(this->axislinewidth*0.5f),                  -this->thickness},
                                   {0, this->scaleheight + this->axislinewidth*0.5f, -this->thickness},
                                   uz, this->axiscolour, this->axislinewidth, this->thickness);
                // x axis
                this->computeLine (idx,
                                   {0,                0, -this->thickness},
                                   {this->scalewidth, 0, -this->thickness},
                                   uz, this->axiscolour, this->axislinewidth, this->thickness);

                // Draw left and bottom ticks
                float tl = -this->ticklength;
                if (this->tickstyle == tickstyle::ticksin) { tl = this->ticklength; }

                for (auto xt : this->xtick_posns) {
                    // Want to place lines in screen units. So transform the data units
                    this->computeLine (idx,
                                       {(float)xt, 0.0f, -this->thickness},
                                       {(float)xt, tl,   -this->thickness}, uz,
                                       this->axiscolour, this->axislinewidth*0.5f, this->thickness);
                }
                for (auto yt : this->ytick_posns) {
                    this->computeLine (idx,
                                       {0.0f, (float)yt, -this->thickness},
                                       {tl,   (float)yt, -this->thickness}, uz,
                                       this->axiscolour, this->axislinewidth*0.5f, this->thickness);
                }

            }

            if (this->axisstyle == axisstyle::box
                || this->axisstyle == axisstyle::boxfullticks
                || this->axisstyle == axisstyle::boxcross) {
                // right axis
                this->computeLine (idx,
                                   {this->scalewidth, -this->axislinewidth*0.5f,                    -this->thickness},
                                   {this->scalewidth, this->scaleheight+(this->axislinewidth*0.5f), -this->thickness},
                                   uz, this->axiscolour, this->axislinewidth, this->thickness);
                // top axis
                this->computeLine (idx,
                                   {0,                this->scaleheight, -this->thickness},
                                   {this->scalewidth, this->scaleheight, -this->thickness},
                                   uz, this->axiscolour, this->axislinewidth, this->thickness);

                // Draw top and right ticks if necessary
                if (this->axisstyle == axisstyle::boxfullticks) {
                    // Tick positions
                    float tl = this->ticklength;
                    if (this->tickstyle == tickstyle::ticksin) {
                        tl = -this->ticklength;
                    }

                    for (auto xt : this->xtick_posns) {
                        // Want to place lines in screen units. So transform the data units
                        this->computeLine (idx,
                                           {(float)xt, this->scalewidth, -this->thickness},
                                           {(float)xt, this->scalewidth + tl,   -this->thickness}, uz,
                                           this->axiscolour, this->axislinewidth*0.5f, this->thickness);
                    }
                    for (auto yt : this->ytick_posns) {
                        this->computeLine (idx,
                                           {this->scaleheight, (float)yt, -this->thickness},
                                           {this->scaleheight + tl,   (float)yt, -this->thickness}, uz,
                                           this->axiscolour, this->axislinewidth*0.5f, this->thickness);
                    }
                }

                if (this->axisstyle == axisstyle::boxcross) { this->drawCrossAxes (idx); }
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

        // Given the data, compute the ticks (or use the ones that client code gave us)
        void computeTickPositions()
        {
            if (this->manualticks == true) {
                std::cout << "Writeme: Implement a manual tick-setting scheme\n";
            } else {
                // Compute locations for ticks...
                Flt _xmin = this->abscissa_scale.inverse_one (this->abscissa_scale.range_min);
                Flt _xmax = this->abscissa_scale.inverse_one (this->abscissa_scale.range_max);
                Flt _ymin = this->zScale.inverse_one (this->zScale.range_min);
                Flt _ymax = this->zScale.inverse_one (this->zScale.range_max);

                std::cout << "x ticks between " << _xmin << " and " << _xmax << " in data units\n";
                std::cout << "y ticks between " << _ymin << " and " << _ymax << " in data units\n";

                float realmin = this->abscissa_scale.inverse_one (0);
                float realmax = this->abscissa_scale.inverse_one (this->scalewidth);
                this->xticks = this->maketicks (_xmin, _xmax, realmin, realmax);
                realmin = this->zScale.inverse_one (0);
                realmax = this->zScale.inverse_one (this->scaleheight);
                this->yticks = this->maketicks (_ymin, _ymax, realmin, realmax);

                this->xtick_posns.resize (this->xticks.size());
                this->abscissa_scale.transform (xticks, xtick_posns);

                this->ytick_posns.resize (this->yticks.size());
                this->zScale.transform (yticks, ytick_posns);
            }
        }

        /*!
         * Auto-computes the tick marker locations (in data space) for the data range
         * rmin to rmax. realmin nd realmax gives the data range actually displayed on
         * the graph - it's the data range, plus any padding introduced by
         * GraphVisual::dataaxisdist
         */
        std::deque<Flt> maketicks (Flt rmin, Flt rmax, float realmin, float realmax)
        {
            std::deque<Flt> ticks;

            std::cout << "maketicks called for range: " << rmin << " to " << rmax << "\n";
            Flt range = rmax - rmin;
            // How big should the range be? log the range, find the floor, raise it to get candidate
            Flt trytick = std::pow (Flt{10.0}, std::floor(std::log10 (range)));
            Flt numticks = floor(range/trytick);
            if (numticks > 10) {
                while (numticks > 10) {
                    trytick = trytick * 2;
                    numticks = floor(range/trytick);
                }
            } else {
                while (numticks < 3) {
                    trytick = trytick * 0.5;
                    numticks = floor(range/trytick);
                }
            }
            std::cout << "Try (data) ticks of size " << trytick << ", which makes for " << numticks << " ticks." << std::endl;

            // Realmax and realmin come from the full range of abscissa_scale/zScale

            Flt atick = trytick;
            while (atick <= realmax) {
                std::cout << "Push back tick at " << atick << std::endl;
                ticks.push_back (atick);
                atick += trytick;
            }
            atick = trytick - trytick;
            while (atick >= realmin) {
                std::cout << "Push back tick at " << atick << std::endl;
                ticks.push_back (atick);
                atick -= trytick;
            }

            return ticks;
        }

    public:
        //! A scaling for the abscissa. I'll use zScale to scale the data values
        morph::Scale<Flt> abscissa_scale;

        // marker features
        std::array<float, 3> markercolour = {0,0,1};
        //! marker size in model units
        float markersize = 0.03f;
        //! The markerstyle. triangle, square, diamond, downtriangle, hexagon, circle, etc
        morph::markerstyle markerstyle = markerstyle::square;
        //! A gap between the data point and the line between data points
        float markergap = 0.03f;

        //! Show lines between data points? This may become a morph::linestyle thing.
        bool showlines = true;
        //! The colour of the lines between data points
        std::array<float, 3> linecolour = {0,0,0};
        //! Width of lines between data points
        float linewidth = 0.007f;

        // axis features
        std::array<float, 3> axiscolour = {0,0,0};
        //! The line width of the main axis bars
        float axislinewidth = 0.006f;
        //! How long should the ticks be?
        float ticklength = 0.02f;
        //! Ticks in or ticks out? Or something else?
        morph::tickstyle tickstyle = tickstyle::ticksout;
        //! What sort of axes to draw: box, cross or leftbottom
        morph::axisstyle axisstyle = axisstyle::box;
        //! Show gridlines where the tick lines are?
        bool showgrid = false;
        //! Should ticks be manually set?
        bool manualticks = false;
        //! The xtick values that should be displayed
        std::deque<Flt> xticks;
        //! The positions, along the x axis (in model space) for the xticks
        std::deque<Flt> xtick_posns;
        //! The ytick values that should be displayed
        std::deque<Flt> yticks;
        //! The positions, along the y axis (in model space) for the yticks
        std::deque<Flt> ytick_posns;
        // Default font
        morph::VisualFont font = morph::VisualFont::Vera;
        //! The font size is the width of an m in the chosen font, in model units
        float fontsize = 0.05;
        // might need tickfontsize and axisfontsize
        //! Gap to x axis tick labels
        float ticklabelgap = 0.05;
        //! Gap to axis label
        float axislabelgap = 0.15;
        //! The x axis label
        std::string xlabel = "x";
        //! The y axis label
        std::string ylabel = "y";

    protected:
        //! How thick are the markers, axes etc? Sort of 'paper thickness'
        float thickness = 0.001f;
        //! scalewidth scales the amount of in-model distance that the graph will be wide
        float scalewidth = 1.0f;
        //! scalewidth height scales the amount of in-model distance that the graph will be high
        float scaleheight = 1.0f;
        //! What proportion of the graph width/height should be allowed as a space
        //! between the min/max and the axes? Can be 0.0f;
        float dataaxisdist = 0.04f;
        //! The axes for orientation of the graph visual, which is 2D within the 3D environment.
        morph::Vector<float> ux = {1,0,0};
        morph::Vector<float> uy = {0,1,0};
        morph::Vector<float> uz = {0,0,1};

        //! Data for the ordinals
        std::vector<Flt>* ordinalData;
    };

} // namespace morph
