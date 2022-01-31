/*!
 * \file GraphVisual
 *
 * \author Seb James
 * \date 2020
 */
#pragma once

#ifndef USE_GLEW
#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#endif
#include <morph/tools.h>
#include <morph/VisualDataModel.h>
#include <morph/Scale.h>
#include <morph/Vector.h>
#include <morph/VisualTextModel.h>
#include <morph/Quaternion.h>
#include <morph/ColourMap.h>
#include <morph/colour.h>
#include <morph/histo.h>
#include <iostream>
#include <vector>
#include <deque>
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
        pentagon, // A polygon has a flat top edge, the 'uppolygon' has a vertex pointing up
        uppentagon,
        hexagon,
        uphexagon,
        heptagon,
        upheptagon,
        octagon,
        upoctagon,
        circle,
        bar, // Special. For a bar graph.
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
        L,          // just left and bottom axis bars (3D: exactly 3 x/y/z axis bars)
        box,        // left, right, top and bottom bars, ticks only on left and bottom bars
        boxfullticks, // left, right, top and bottom bars, with ticks all round
        panels,     // For 3D: like a 2D 'box' making a floor, and 2 side 'walls'
        cross,      // a cross of bars at the zero axes
        boxcross,   // A box AND the zero axes
        twinax,     // A box which has two y axes, the first on the left and the second on the right
        numstyles
    };

    //! When generating a graph, should we generate marker-only graphs, line-only graphs
    //! or marker+line graphs? Each DatasetStyle contains a stylepolicy.
    enum class stylepolicy
    {
        markers,        // coloured markers, with differing shapes
        lines,          // coloured lines
        both,           // coloured markers, black lines
        allcolour,      // coloured markers and lines
        bar,            // bar graph. marker colour is the bar colour, linecolour is the outline colour (if used)
        numstyles
    };

    //! When autoscaling data, do we purely autoscale from the data, using the data's
    //! min and max values, or do we autoscale from a fixed value (such as 0) to the
    //! data's max, or do we scale the graph to two fixed values, set by the user?
    enum class scalingpolicy
    {
        autoscale,      // full autoscaling to data min and max
        manual_min,     // Use the GraphVisual's abs/datamin_y attribute when scaling, and data's max
        manual_max,     // Unlikely to be used, but use data's min and abs/datamax_y
        manual,         // Use abs/datamin_y and abs/datamax_y for scaling - i.e. full manual scaling
        numstyles
    };

    //! Which side of the (twin) axes should the dataset relate to?
    enum class axisside
    {
        left,
        right,
        numstyles
    };

    //! The attributes for graphing a single dataset
    struct DatasetStyle
    {
        DatasetStyle(){}
        DatasetStyle(stylepolicy p)
        {
            this->policy = p;
            if (p == stylepolicy::markers) {
                this->showlines = false;
            } else if (p == stylepolicy::lines) {
                this->markerstyle = markerstyle::none;
                this->markergap = 0.0f;
            } else if (p == stylepolicy::bar) {
                this->markerstyle = markerstyle::bar;
            }
        }
        //! Policy of style
        stylepolicy policy = stylepolicy::both;
        //! The colour of the marker
        std::array<float, 3> markercolour = morph::colour::royalblue;
        //! marker size in model units. Used as bar width for bar graphs
        float markersize = 0.03f;
        //! The markerstyle. triangle, square, diamond, downtriangle, hexagon, circle, etc
        morph::markerstyle markerstyle = markerstyle::square;
        //! A gap between the data point and the line between data points
        float markergap = 0.03f;

        //! Show lines between data points? This may become a morph::linestyle thing.
        bool showlines = true;
        //! The colour of the lines between data points
        std::array<float, 3> linecolour = morph::colour::black;
        //! Width of lines between data points
        float linewidth = 0.007f;
        //! Label for the dataset's legend
        std::string datalabel = "";
        //! Which y axis of a twinax graph should these data relate to?
        morph::axisside axisside = morph::axisside::left;

        //! A setter to set both colours to the same value
        void setcolour (const std::array<float, 3>& c)
        {
            this->linecolour = c;
            this->markercolour = c;
        }
    };

    /*!
     * A VisualModel for showing a 2D graph.
     */
    template <typename Flt>
    class GraphVisual : public VisualModel
    {
    public:
        //! Constructor which sets just the shader program and the model view offset
        GraphVisual(GLuint sp, GLuint tsp, const Vector<float> _offset)
        {
            this->shaderprog = sp;
            this->tshaderprog = tsp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->ord1_scale.do_autoscale = true;
            this->ord2_scale.do_autoscale = true;
            this->abscissa_scale.do_autoscale = true;
            // Graphs don't rotate by default. If you want yours to, set this false in your client code.
            this->twodimensional = true;
        }

        ~GraphVisual() { for (auto& gdc : this->graphDataCoords) { delete gdc; } }

        //! Set true for any optional debugging
        static constexpr bool gv_debug = false;

        //! Append a single datum onto the relevant graph. Build on existing data in
        //! graphDataCoords. Finish up with a call to completeAppend(). didx is the data
        //! index and counts up from 0.
        void append (const Flt& _abscissa, const Flt& _ordinate, const size_t didx)
        {
            this->pendingAppended = true;
            // Transfor the data into temporary containers sd and ad
            Flt o = Flt{0};
            if (this->datastyles[didx].axisside == morph::axisside::left) {
                o = this->ord1_scale.transform_one (_ordinate);
            } else {
                o = this->ord2_scale.transform_one (_ordinate);
            }
            Flt a = this->abscissa_scale.transform_one (_abscissa);
            //std::cout << "transformed coords: " << a << ", " << o << std::endl;
            // Now sd and ad can be used to construct dataCoords x/y. They are used to
            // set the position of each datum into dataCoords
            size_t oldsz = this->graphDataCoords[didx]->size();
            (this->graphDataCoords[didx])->resize (oldsz+1);
            (*this->graphDataCoords[didx])[oldsz][0] = a;
            (*this->graphDataCoords[didx])[oldsz][1] = o;
            (*this->graphDataCoords[didx])[oldsz][2] = Flt{0};
        }

        //! Before calling the base class's render method, check if we have any pending data
        void render()
        {
            if (this->pendingAppended == true) {
                // After adding to graphDataCoords, we have to create the new OpenGL
                // vertices (CPU side) and update the OpenGL buffers.
                this->drawAppendedData();
                this->reinit_buffers();
                this->pendingAppended = false;
            }
            // Now do the usual drawing stuff from VisualModel:
            VisualModel::render();
        }

        //! Clear all the data for the graph, but leave the containers in place.
        void clear()
        {
            size_t dsize = this->graphDataCoords.size();
            for (size_t i = 0; i < dsize; ++i) {
                this->graphDataCoords[i]->clear();
            }
            this->reinit();
        }

        //! Update the data for the graph, recomputing the vertices when done.
        void update (const std::vector<Flt>& _abscissae,
                     const std::vector<Flt>& _data, const size_t data_idx)
        {
            size_t dsize = _data.size();

            if (_abscissae.size() != dsize) {
                throw std::runtime_error ("updatedata: size mismatch");
            }

            if (data_idx >= this->graphDataCoords.size()) {
                std::cout << "Can't add data at graphDataCoords index " << data_idx << std::endl;
                return;
            }

            // Ensure the vector at data_idx has enough capacity for the updated data
            this->graphDataCoords[data_idx]->resize (dsize);

            // May need a re-autoscaling option somewhere in here.

            // Transfor the data into temporary containers sd and ad
            std::vector<Flt> sd (dsize, Flt{0});
            std::vector<Flt> ad (dsize, Flt{0});
            this->ord1_scale.transform (_data, sd);
            this->abscissa_scale.transform (_abscissae, ad);

            // Now sd and ad can be used to construct dataCoords x/y. They are used to
            // set the position of each datum into dataCoords
            for (size_t i = 0; i < dsize; ++i) {
                (*this->graphDataCoords[data_idx])[i][0] = static_cast<Flt>(ad[i]);
                (*this->graphDataCoords[data_idx])[i][1] = static_cast<Flt>(sd[i]);
                (*this->graphDataCoords[data_idx])[i][2] = Flt{0};
            }

            this->reinit();
        }

        //! Set marker and colours in ds, according the 'style policy'
        void setstyle (morph::DatasetStyle& ds, std::array<float, 3> col, morph::markerstyle ms)
        {
            if (ds.policy != stylepolicy::lines) {
                // Is not lines only, so must be markers, or markers+lines
                ds.markerstyle = ms;
                ds.markercolour = col;
            } else {
                // Must be stylepolicy::lines
                ds.linecolour = col;
            }
            if (ds.policy == stylepolicy::allcolour) {
                ds.linecolour = col;
            }
        }

        //! Obtain the curated dataset colours, by index. Static public function to
        //! allow other Visuals to colour things in the same order as a graph.
        static std::array<float, 3> datacolour (size_t data_index)
        {
            std::array<float, 3> rtn = morph::colour::gray50;
            switch (data_index) {
            case 0: { rtn = morph::colour::royalblue; break; }
            case 1: { rtn = morph::colour::crimson; break; }
            case 2: { rtn = morph::colour::goldenrod2; break; }
            case 3: { rtn = morph::colour::green2; break; }

            case 4: { rtn = morph::colour::blue2; break; }
            case 5: { rtn = morph::colour::blueviolet; break; }
            case 6: { rtn = morph::colour::khaki1; break; }
            case 7: { rtn = morph::colour::sapgreen; break; }

            case 8: { rtn = morph::colour::mediumturquoise; break; }
            case 9: { rtn = morph::colour::seagreen1; break; }
            case 10: { rtn = morph::colour::darkgoldenrod4; break; }
            case 11: { rtn = morph::colour::olivedrab2; break; }

            case 12: { rtn = morph::colour::lightsteelblue3; break; }
            case 13: { rtn = morph::colour::purple; break; }
            case 14: { rtn = morph::colour::deeppink1; break; }
            case 15: { rtn = morph::colour::red2; break; }

            case 16: { rtn = morph::colour::royalblue1; break; }
            case 17: { rtn = morph::colour::mediumorchid1; break; }
            case 18: { rtn = morph::colour::lightskyblue1; break; }
            case 19: { rtn = morph::colour::firebrick1; break; }

            case 20: { rtn = morph::colour::royalblue2; break; }
            case 21: { rtn = morph::colour::mediumorchid2; break; }
            case 22: { rtn = morph::colour::lightskyblue2; break; }
            case 23: { rtn = morph::colour::firebrick2; break; }

            case 24: { rtn = morph::colour::royalblue3; break; }
            case 25: { rtn = morph::colour::mediumorchid3; break; }
            case 26: { rtn = morph::colour::lightskyblue3; break; }
            case 27: { rtn = morph::colour::firebrick3; break; }

            case 28: { rtn = morph::colour::royalblue4; break; }
            case 29: { rtn = morph::colour::mediumorchid4; break; }
            case 30: { rtn = morph::colour::lightskyblue4; break; }
            case 31: { rtn = morph::colour::firebrick4; break; }

            case 32: { rtn = morph::colour::brown1; break; }
            case 33: { rtn = morph::colour::darkolivegreen1; break; }
            case 34: { rtn = morph::colour::chocolate1; break; }
            case 35: { rtn = morph::colour::chartreuse; break; }

            case 36: { rtn = morph::colour::brown2; break; }
            case 37: { rtn = morph::colour::darkolivegreen2; break; }
            case 38: { rtn = morph::colour::chocolate2; break; }
            case 39: { rtn = morph::colour::chartreuse2; break; }

            case 40: { rtn = morph::colour::brown3; break; }
            case 41: { rtn = morph::colour::darkolivegreen3; break; }
            case 42: { rtn = morph::colour::chocolate3; break; }
            case 43: { rtn = morph::colour::chartreuse3; break; }

            case 44: { rtn = morph::colour::brown4; break; }
            case 45: { rtn = morph::colour::darkolivegreen4; break; }
            case 46: { rtn = morph::colour::chocolate4; break; }
            case 47: { rtn = morph::colour::chartreuse4; break; }

            default: { break; }
            }
            return rtn;
        }

        //! Prepare an as-yet empty dataset.
        void prepdata (const std::string name = "", const morph::axisside axisside = morph::axisside::left)
        {
            std::vector<Flt> emptyabsc;
            std::vector<Flt> emptyord;
            this->setdata (emptyabsc, emptyord, name, axisside);
        }

        //! Set a dataset into the graph using default styles, incrementing colour and
        //! marker shape as more datasets are included in the graph.
        void setdata (const std::vector<Flt>& _abscissae, const std::vector<Flt>& _data,
                      const std::string name = "", const morph::axisside axisside = morph::axisside::left)
        {
            DatasetStyle ds(this->policy);
            ds.axisside = axisside;
            if (!name.empty()) { ds.datalabel = name; }

            size_t data_index = this->graphDataCoords.size();
            switch (data_index) {
            case 0:
            {
                this->setstyle (ds, GraphVisual<Flt>::datacolour(data_index), morph::markerstyle::square);
                break;
            }
            case 1:
            {
                this->setstyle (ds, GraphVisual<Flt>::datacolour(data_index), morph::markerstyle::triangle);
                break;
            }
            case 2:
            {
                this->setstyle (ds, GraphVisual<Flt>::datacolour(data_index), morph::markerstyle::circle);
                break;
            }
            case 3:
            {
                this->setstyle (ds, GraphVisual<Flt>::datacolour(data_index), morph::markerstyle::diamond);
                break;
            }
            default:
            {
                // Everything else gets this:
                this->setstyle (ds, GraphVisual<Flt>::datacolour(data_index), morph::markerstyle::hexagon);
                break;
            }
            }
            this->setdata (_abscissae, _data, ds);
        }

        //! Set a dataset into the graph. Provide abscissa and ordinate and a dataset
        //! style. The locations of the markers for each dataset are computed and stored
        //! in this->graohDataCoords, one vector for each dataset.
        void setdata (const std::vector<Flt>& _abscissae,
                      const std::vector<Flt>& _data, const DatasetStyle& ds)
        {
            if (_abscissae.size() != _data.size()) { throw std::runtime_error ("size mismatch"); }

            size_t dsize = _data.size();
            size_t didx = this->graphDataCoords.size();

            // Allocate memory for the new data coords, add the data style info and the
            // starting index for dataCoords
#ifdef __ICC__
            morph::Vector<float> dummyzero = {{0.0f, 0.0f, 0.0f}};
            this->graphDataCoords.push_back (new std::vector<morph::Vector<float>>(dsize, dummyzero));
#else
            this->graphDataCoords.push_back (new std::vector<morph::Vector<float>>(dsize, {0,0,0}));
#endif
            this->datastyles.push_back (ds);

            // Compute the ord1_scale and asbcissa_scale for the first added dataset only
            if (ds.axisside == morph::axisside::left) {
                if (this->ord1_scale.autoscaled == false) { this->compute_scaling (_abscissae, _data, ds.axisside); }
            } else {
                if (this->ord2_scale.autoscaled == false) { this->compute_scaling (_abscissae, _data, ds.axisside); }
            }

            if (dsize > 0) {
                // Transform the data into temporary containers sd and ad
                std::vector<Flt> sd (dsize, Flt{0});
                std::vector<Flt> ad (dsize, Flt{0});
                if (ds.axisside == morph::axisside::left) {
                    this->ord1_scale.transform (_data, sd);
                } else {
                    this->ord2_scale.transform (_data, sd);
                }
                this->abscissa_scale.transform (_abscissae, ad);

                // Now sd and ad can be used to construct dataCoords x/y. They are used to
                // set the position of each datum into dataCoords
                for (size_t i = 0; i < dsize; ++i) {
                    (*this->graphDataCoords[didx])[i][0] = static_cast<Flt>(ad[i]);
                    (*this->graphDataCoords[didx])[i][1] = static_cast<Flt>(sd[i]);
                    (*this->graphDataCoords[didx])[i][2] = Flt{0};
                }
            }
        }

        //! Special setdata for a morph::histo object
        void setdata (const morph::histo<Flt>& h, const std::string name = "")
        {
            DatasetStyle ds(this->policy);
            if (!name.empty()) { ds.datalabel = name; }

            // Because this overload of setdata sets bargraph data, I want it to force the graph to be stylepolicy::bar
            ds.policy = morph::stylepolicy::bar;
            ds.markerstyle = morph::markerstyle::bar;
            // How to choose? User sets afterwards?
            ds.showlines = true;
            ds.markersize = (this->width - this->width*2*this->dataaxisdist) * (h.binwidth / h.range);
            ds.linewidth = ds.markersize/10.0;

            size_t data_index = this->graphDataCoords.size();
            ds.markercolour = GraphVisual<Flt>::datacolour(data_index);
            ds.linecolour = morph::colour::black; // For now.

            // Because this is bar graph data, make sure to compute the ord1_scale now from
            // 0 -> max and NOT from min -> max.
            this->scalingpolicy_y = morph::scalingpolicy::manual_min;
            this->datamin_y = Flt{0};
            this->setdata (h.bins, h.proportions, ds);
        }

        //! Set graph from histogram with pre-configured datasetstyle
        void setdata (const morph::histo<Flt>& h, const DatasetStyle& ds)
        {
            // Because this is bar graph data, make sure to compute the ord1_scale now from
            // 0 -> max and NOT from min -> max.
            this->scalingpolicy_y = morph::scalingpolicy::manual_min;
            this->datamin_y = Flt{0};
            this->setdata (h.bins, h.proportions, ds);
        }

    protected:
        //! Compute the scaling of ord1_scale and abscissa_scale according to the scalingpolicies
        void compute_scaling (const std::vector<Flt>& _abscissae, const std::vector<Flt>& _data, const morph::axisside axisside)
        {
            std::pair<Flt, Flt> data_maxmin = morph::MathAlgo::maxmin (_data);
            std::pair<Flt, Flt> absc_maxmin = morph::MathAlgo::maxmin (_abscissae);
            if (axisside == morph::axisside::left) {
                this->setsize (this->width, this->height);
            }

            // x axis - the abscissa
            switch (this->scalingpolicy_x) {
            case morph::scalingpolicy::manual:
            {
                this->abscissa_scale.compute_autoscale (this->datamin_x, this->datamax_x);
                break;
            }
            case morph::scalingpolicy::manual_min:
            {
                this->abscissa_scale.compute_autoscale (this->datamin_x, absc_maxmin.first);
                break;
            }
            case morph::scalingpolicy::manual_max:
            {
                this->abscissa_scale.compute_autoscale (absc_maxmin.second, this->datamax_x);
                break;
            }
            case morph::scalingpolicy::autoscale:
            default:
            {
                this->abscissa_scale.compute_autoscale (absc_maxmin.second, absc_maxmin.first);
                break;
            }
            }

            // y axis - the ordinate
            switch (this->scalingpolicy_y) {
            case morph::scalingpolicy::manual:
            {
                if (axisside == morph::axisside::left) {
                    this->ord1_scale.compute_autoscale (this->datamin_y, this->datamax_y);
                } else {
                    this->ord2_scale.compute_autoscale (this->datamin_y2, this->datamax_y2);
                }
                break;
            }
            case morph::scalingpolicy::manual_min:
            {
                if (axisside == morph::axisside::left) {
                    this->ord1_scale.compute_autoscale (this->datamin_y, data_maxmin.first);
                } else {
                    this->ord2_scale.compute_autoscale (this->datamin_y2, data_maxmin.first);
                }
                break;
            }
            case morph::scalingpolicy::manual_max:
            {
                if (axisside == morph::axisside::left) {
                    this->ord1_scale.compute_autoscale (data_maxmin.second, this->datamax_y);
                } else {
                    this->ord2_scale.compute_autoscale (data_maxmin.second, this->datamax_y2);
                }
                break;
            }
            case morph::scalingpolicy::autoscale:
            default:
            {
                if (axisside == morph::axisside::left) {
                    this->ord1_scale.compute_autoscale (data_maxmin.second, data_maxmin.first);
                } else {
                    this->ord2_scale.compute_autoscale (data_maxmin.second, data_maxmin.first);
                }
                break;
            }
            }
        }

    public:

        //! Setter for the dataaxisdist attribute
        void setdataaxisdist (float proportion)
        {
            if (this->ord1_scale.autoscaled == true) {
                throw std::runtime_error ("Have already scaled the data, can't set the dataaxisdist now.\n"
                                          "Hint: call GraphVisual::setdataaxisdist() BEFORE GraphVisual::setdata() or ::setlimits()");
            }
            this->dataaxisdist = proportion;
        }

        //! Set the graph size, in model units.
        void setsize (float _width, float _height)
        {
            if (this->ord1_scale.autoscaled == true) {
                throw std::runtime_error ("Have already scaled the data, can't set the scale now.\n"
                                          "Hint: call GraphVisual::setsize() BEFORE GraphVisual::setdata() or ::setlimits()");
            }
            this->width = _width;
            this->height = _height;

            float _extra = this->dataaxisdist * this->height;
            this->ord1_scale.range_min = _extra;
            this->ord1_scale.range_max = this->height - _extra;
            // Same for ord2_scale:
            this->ord2_scale.range_min = _extra;
            this->ord2_scale.range_max = this->height - _extra;

            _extra = this->dataaxisdist * this->width;
            this->abscissa_scale.range_min = _extra;
            this->abscissa_scale.range_max = this->width - _extra;

            this->thickness *= this->width;
        }

        //! Set manual limits for the x axis (abscissa)
        void setlimits_x (Flt _xmin, Flt _xmax)
        {
            this->scalingpolicy_x = morph::scalingpolicy::manual;
            this->datamin_x = _xmin;
            this->datamax_x = _xmax;
            this->setsize (this->width, this->height);
            this->abscissa_scale.compute_autoscale (this->datamin_x, this->datamax_x);
        }

        //! Set manual limits for the y axis (ordinate)
        void setlimits_y (Flt _ymin, Flt _ymax)
        {
            this->scalingpolicy_y = morph::scalingpolicy::manual;
            this->datamin_y = _ymin;
            this->datamax_y = _ymax;
            this->setsize (this->width, this->height);
            this->ord1_scale.compute_autoscale (this->datamin_y, this->datamax_y);
        }

        //! Set manual limits for the second y axis (ordinate)
        void setlimits_y2 (Flt _ymin, Flt _ymax)
        {
            this->scalingpolicy_y = morph::scalingpolicy::manual; // scalingpolicy_y common to both left and right axes?
            this->datamin_y2 = _ymin;
            this->datamax_y2 = _ymax;
            this->setsize (this->width, this->height);
            this->ord2_scale.compute_autoscale (this->datamin_y2, this->datamax_y2);
        }

        // Axis ranges. The length of each axis could be determined from the data and
        // abscissas for a static graph, but for a dynamically updating graph, it's
        // going to be necessary to give a hint at how far the data/abscissas might need
        // to extend.
        void setlimits (Flt _xmin, Flt _xmax, Flt _ymin, Flt _ymax)
        {
            // Set limits with 4 args gives fully manual scaling
            this->scalingpolicy_x = morph::scalingpolicy::manual;
            this->datamin_x = _xmin;
            this->datamax_x = _xmax;
            this->scalingpolicy_y = morph::scalingpolicy::manual;
            this->datamin_y = _ymin;
            this->datamax_y = _ymax;

            // First make sure that the range_min/max are correctly set
            this->setsize (this->width, this->height);
            // To make the axes larger, we change the scaling that we'll apply to the
            // data (the axes are always width * height in size).
            this->ord1_scale.compute_autoscale (this->datamin_y, this->datamax_y);
            this->abscissa_scale.compute_autoscale (this->datamin_x, this->datamax_x);
        }

        //! setlimits overload that sets BOTH left and right axes limits
        void setlimits (Flt _xmin, Flt _xmax, Flt _ymin, Flt _ymax, Flt _ymin2, Flt _ymax2)
        {
            // Set limits with 4 args gives fully manual scaling
            this->scalingpolicy_x = morph::scalingpolicy::manual;
            this->datamin_x = _xmin;
            this->datamax_x = _xmax;
            this->scalingpolicy_y = morph::scalingpolicy::manual;
            this->datamin_y = _ymin;
            this->datamax_y = _ymax;
            this->datamin_y2 = _ymin2;
            this->datamax_y2 = _ymax2;

            // First make sure that the range_min/max are correctly set
            this->setsize (this->width, this->height);
            // To make the axes larger, we change the scaling that we'll apply to the
            // data (the axes are always width * height in size).
            this->ord1_scale.compute_autoscale (this->datamin_y, this->datamax_y);
            this->ord2_scale.compute_autoscale (this->datamin_y2, this->datamax_y2);
            this->abscissa_scale.compute_autoscale (this->datamin_x, this->datamax_x);
        }

        //! Set the 'object thickness' attribute (maybe used just for 'object spacing')
        void setthickness (float th) { this->thickness = th; }

        //! Tell this GraphVisual that it's going to be rendered on a dark background. Updates axis colour.
        void setdarkbg()
        {
            this->darkbg = true;
            this->axiscolour = {0.8f, 0.8f, 0.8f};
        }

        //! Graph-specific number formatting for tick labels.
        static std::string graphNumberFormat (Flt num)
        {
            std::stringstream ss;
            ss << num;
            std::string s = ss.str();

            if (num > Flt{-1} && num < Flt{1} && num != Flt{0}) {
                // It's a 0.something number. Get rid of any 0 preceding a '.'
                std::string::size_type p = s.find ('.');
                if (p != std::string::npos && p>0) {
                    if (s[--p] == '0') { s.erase(p, 1); }
                }
            }

            return s;
        }

        /*!
         * Auto-computes the tick marker locations (in data space) for the data range
         * rmin to rmax. realmin and realmax gives the data range actually displayed on
         * the graph - it's the data range, plus any padding introduced by
         * GraphVisual::dataaxisdist
         */
        static std::deque<Flt> maketicks (Flt rmin, Flt rmax, float realmin, float realmax,
                                          const Flt max_num_ticks = 10, const Flt min_num_ticks = 3)
        {
            std::deque<Flt> ticks;

            Flt range = rmax - rmin;
            // How big should the range be? log the range, find the floor, raise it to get candidate
            Flt trytick = std::pow (Flt{10.0}, std::floor(std::log10 (range)));
            Flt numticks = floor(range/trytick);
            if (numticks > max_num_ticks) {
                while (numticks > max_num_ticks) {
                    trytick = trytick * 2;
                    numticks = floor(range/trytick);
                }
            } else {
                while (numticks < min_num_ticks) {
                    trytick = trytick * 0.5;
                    numticks = floor(range/trytick);
                }
            }
            if constexpr (gv_debug) {
                std::cout << "Try (data) ticks of size " << trytick << ", which makes for " << numticks << " ticks.\n";
            }
            // Realmax and realmin come from the full range of abscissa_scale/ord1_scale
            Flt atick = trytick;
            while (atick <= realmax) {
                ticks.push_back (atick);
                atick += trytick;
            }
            atick = trytick - trytick;
            while (atick >= realmin) {
                ticks.push_back (atick);
                atick -= trytick;
            }

            return ticks;
        }

    protected:

        //! The OpenGL indices index
        VBOint idx = 0;

        //! Stores the length of each entry in graphDataCoords - i.e how many data
        //! points are in each graph curve
        std::vector<size_t> coords_lengths;

        //! Is there pending appended data that needs to be converted into OpenGL shapes?
        bool pendingAppended = false;

        //! Compute stuff for a graph
        void initializeVertices()
        {
            // The indices index
            this->idx = 0;
            this->drawAxes();
            this->drawData();
            if (this->legend == true) { this->drawLegend(); }
            this->drawTickLabels(); // from which we can store the tick label widths
            this->drawAxisLabels();
        }

        static constexpr bool three_d_lines = false;
        //! dsi: data set iterator
        void drawDataCommon (size_t dsi, size_t coords_start, size_t coords_end, bool appending = false)
        {
            // Draw data markers
            if (this->datastyles[dsi].markerstyle != markerstyle::none) {
                if (this->datastyles[dsi].markerstyle == markerstyle::bar) {
                    for (size_t i = coords_start; i < coords_end; ++i) {
                        this->bar ((*this->graphDataCoords[dsi])[i], this->datastyles[dsi]);
                    }
                } else {
                    for (size_t i = coords_start; i < coords_end; ++i) {
                        this->marker ((*this->graphDataCoords[dsi])[i], this->datastyles[dsi]);
                    }
                }
            }
            if (this->datastyles[dsi].markerstyle == markerstyle::bar && this->datastyles[dsi].showlines == true) {
                // No need to do anything, lines will have been drawn by GraphVisual::bar()
            } else if (this->datastyles[dsi].showlines == true) {

                // If appending markers to a dataset, need to add the line preceding the first marker
                if (appending == true) { if (coords_start != 0) { coords_start -= 1; } }

                for (size_t i = coords_start+1; i < coords_end; ++i) {
                    // Draw tube from location -1 to location 0.
                    if constexpr (three_d_lines) {
                        this->computeLine (this->idx, (*this->graphDataCoords[dsi])[i-1], (*this->graphDataCoords[dsi])[i], uz,
                                           this->datastyles[dsi].linecolour, this->datastyles[dsi].linecolour,
                                           this->datastyles[dsi].linewidth, this->thickness*Flt{0.7}, this->datastyles[dsi].markergap);
                    } else {
                        if (this->datastyles[dsi].markergap > 0.0f) {
                            this->computeFlatLine (this->idx, (*this->graphDataCoords[dsi])[i-1], (*this->graphDataCoords[dsi])[i], uz,
                                                   this->datastyles[dsi].linecolour,
                                                   this->datastyles[dsi].linewidth, this->datastyles[dsi].markergap);
                        } else if (appending == true) {
                            this->computeFlatLineRnd (this->idx,
                                                      (*this->graphDataCoords[dsi])[i-1], // start
                                                      (*this->graphDataCoords[dsi])[i],   // end
                                                      uz,
                                                      this->datastyles[dsi].linecolour,
                                                      this->datastyles[dsi].linewidth, 0.0f, true, false);
                        } else {
                            // No gaps, so draw a perfect set of joined up lines
                            if (i == 1+coords_start) {
                                // First line
                                this->computeFlatLineN (this->idx,
                                                        (*this->graphDataCoords[dsi])[i-1], // start
                                                        (*this->graphDataCoords[dsi])[i],   // end
                                                        (*this->graphDataCoords[dsi])[i+1], // next
                                                        uz,
                                                        this->datastyles[dsi].linecolour,
                                                        this->datastyles[dsi].linewidth);
                            } else if (i == (coords_end-1)) {
                                // last line
                                this->computeFlatLineP (this->idx, (*this->graphDataCoords[dsi])[i-1], (*this->graphDataCoords[dsi])[i],
                                                        (*this->graphDataCoords[dsi])[i-2],
                                                        uz,
                                                        this->datastyles[dsi].linecolour,
                                                        this->datastyles[dsi].linewidth);
                            } else {
                                this->computeFlatLine (this->idx, (*this->graphDataCoords[dsi])[i-1], (*this->graphDataCoords[dsi])[i],
                                                       (*this->graphDataCoords[dsi])[i-2], (*this->graphDataCoords[dsi])[i+1],
                                                       uz,
                                                       this->datastyles[dsi].linecolour,
                                                       this->datastyles[dsi].linewidth);
                            }
                        }
                    }
                }
            }
        }

        // Defines a boolean 'true' that can be provided as arg to drawDataCommon()
        static constexpr bool appending_data = true;

        //! Draw markers and lines for data points that are being appended to a graph
        void drawAppendedData()
        {
            for (size_t dsi = 0; dsi < this->graphDataCoords.size(); ++dsi) {
                // Start is old end:
                size_t coords_start = this->coords_lengths[dsi];
                size_t coords_end = this->graphDataCoords[dsi]->size();
                this->coords_lengths[dsi] = coords_end;
                this->drawDataCommon (dsi, coords_start, coords_end, appending_data);
            }
        }

        //! Draw all markers and lines for datasets in the graph (as stored in graphDataCoords)
        void drawData()
        {
            size_t coords_start = 0;
            this->coords_lengths.resize (this->graphDataCoords.size());
            for (size_t dsi = 0; dsi < this->graphDataCoords.size(); ++dsi) {
                size_t coords_end = this->graphDataCoords[dsi]->size();
                // Record coords length for future appending:
                this->coords_lengths[dsi] = coords_end;
                this->drawDataCommon (dsi, coords_start, coords_end);
            }
        }

        //! Draw the graph legend, above the graph, rather than inside it (so much simpler!)
        void drawLegend()
        {
            size_t gd_size = this->graphDataCoords.size();

            // Text offset from marker to text
            morph::Vector<float> toffset = {this->fontsize, 0.0f, 0.0f};

            // To determine the legend layout, will need all the text geometries
            std::vector<morph::TextGeometry> geom;
            std::vector<morph::VisualTextModel*> legtexts;
            float text_advance = 0.0f;
            for (size_t dsi = 0; dsi < gd_size; ++dsi) {
                // Legend text. If all is well, this will be pushed onto the texts
                // attribute and deleted when the model is deconstructed.
                legtexts.push_back (new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, this->fontres));
                geom.push_back (legtexts.back()->getTextGeometry (this->datastyles[dsi].datalabel));
                if (geom.back().total_advance > text_advance) {
                    text_advance = geom.back().total_advance;
                }
            }
            //std::cout << "Legend text advance is: " << text_advance << std::endl;

            // If there are no legend texts to show, then clean up and return
            if (text_advance == 0.0f && !legtexts.empty()) {
                // delete memory pointed to in legtexts
                for (auto& lt : legtexts) { delete lt; }
                return;
            }

            // Adjust the text offset by the last entry in geom
            if (!geom.empty()) { toffset[1] -= geom.back().height()/2.0f; }

            // What's our legend grid arrangement going to be? Each column will advance by the text_advance, some space and the size of the marker
            float col_advance = 2 * toffset[0] + text_advance;
            if (!datastyles.empty()) { col_advance += this->datastyles[0].markersize; }
            //std::cout << "Legend col advance is: " << col_advance << std::endl;
            int max_cols = static_cast<int>((1.0f - this->dataaxisdist) / col_advance);
            //std::cout << "max_cols: " << max_cols << std::endl;
            if (max_cols < 1) { max_cols = 1; }
            //std::cout << "max_cols after check it's 1 or more: " << max_cols << std::endl;
            int num_cols = static_cast<int>(gd_size) <= max_cols ? static_cast<int>(gd_size) : max_cols;
            //std::cout << "gd_size is " << gd_size << " num_cols is " << num_cols << std::endl;
            int num_rows = (int)gd_size;
            if (num_cols != 0) {
                num_rows = ((int)gd_size / num_cols);
                num_rows += (int)gd_size % num_cols ? 1 : 0;
            }
            //std::cout << "num_rows = " << num_rows << std::endl;

            // Label position
            morph::Vector<float> lpos = {this->dataaxisdist, 0.0f, 0.0f};
            for (int dsi = 0; dsi < (int)gd_size; ++dsi) {

                if (num_cols == 0) { throw std::runtime_error ("GraphVisual::drawLegend: Why is num_cols 0?"); }
                int col = dsi % num_cols;
                int row = (num_rows-1) - (dsi / num_cols);
                //std::cout << "Dataset " << dsi << " will be on row " << row << " and col " << col << std::endl;

                lpos[0] = this->dataaxisdist + ((float)col * col_advance);
                lpos[1] = this->height + 1.5f*this->fontsize + (float)(row)*2.0f*this->fontsize;
                // Legend line/marker
                if (this->datastyles[dsi].showlines == true && this->datastyles[dsi].markerstyle != markerstyle::bar) {
                    // draw short line at lpos (rounded ends)
                    morph::Vector<float, 3> abit = { 0.5f * toffset[0], 0.0f, 0.0f };
                    this->computeFlatLineRnd (this->idx, lpos - abit, lpos + abit,
                                              this->uz,
                                              this->datastyles[dsi].linecolour,
                                              this->datastyles[dsi].linewidth);

                }
                if (this->datastyles[dsi].markerstyle != markerstyle::none) {
                    if (this->datastyles[dsi].markerstyle == markerstyle::bar) {
                        // For bar graph, show a small square (or rect?) with the colour
                        this->bar_symbol (lpos, this->datastyles[dsi]);
                    } else {
                        this->marker (lpos, this->datastyles[dsi]);
                    }
                }
                legtexts[dsi]->setupText (this->datastyles[dsi].datalabel, lpos+toffset+this->mv_offset, this->axiscolour);
                this->texts.push_back (legtexts[dsi]);
            }
        }

        //! Add the axis labels
        void drawAxisLabels()
        {
            // x axis label (easy)
            morph::VisualTextModel* lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, this->fontres);
            morph::TextGeometry geom = lbl->getTextGeometry (this->xlabel);
            morph::Vector<float> lblpos;
            if (this->axisstyle == axisstyle::cross) {
                float _y0_mdl = this->ord1_scale.transform_one (0);
                lblpos = {{0.9f * this->width,
                           _y0_mdl-(this->axislabelgap+geom.height()+this->ticklabelgap+this->xtick_label_height), 0 }};
            } else {
                lblpos = {{0.5f * this->width - geom.half_width(),
                           -(this->axislabelgap+this->ticklabelgap+geom.height()+this->xtick_label_height), 0}};
            }
            lbl->setupText (this->xlabel, lblpos+this->mv_offset, this->axiscolour);
            this->texts.push_back (lbl);

            // y axis label (have to rotate)
            lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, this->fontres);
            geom = lbl->getTextGeometry (this->ylabel);

            // Rotate label if it's long, but assume NOT rotated first:
            float leftshift = geom.width();
            float downshift = geom.height();
            if (geom.width() > 2*this->fontsize) { // rotate so shift by text height
                // Rotated, so left shift due to text is 0
                leftshift = 0;
                downshift = geom.half_width();
            }

            if (this->axisstyle == axisstyle::cross) {
                float _x0_mdl = this->abscissa_scale.transform_one (0);
                lblpos = {{ _x0_mdl-(this->ticklabelgap+this->ytick_label_width+leftshift+this->axislabelgap),
                            0.9f * this->height, 0 }};
            } else {
                lblpos = {{ -(this->ticklabelgap+this->ytick_label_width+leftshift+this->axislabelgap),
                            0.5f*this->height - downshift, 0 }};
            }

            if (geom.width() > 2*this->fontsize) {
                morph::Quaternion<float> leftrot;
                leftrot.initFromAxisAngle (this->uz, -90.0f);
                lbl->setupText (this->ylabel, leftrot, lblpos+this->mv_offset, this->axiscolour);
            } else {
                lbl->setupText (this->ylabel, lblpos+this->mv_offset, this->axiscolour);
            }
            this->texts.push_back (lbl);

            if (this->axisstyle == axisstyle::twinax) {
                // y2 axis label (have to rotate)
                lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, this->fontres);
                geom = lbl->getTextGeometry (this->ylabel2);

                // Rotate label if it's long and then leftshift? No need if unrotated.
                float leftshift = 0.0f;
                float downshift = geom.height();
                if (geom.width() > 2*this->fontsize) { // rotate so shift by text height
                    leftshift = geom.height();
                    downshift = geom.half_width();
                }

                lblpos = {{ this->width+(this->ticklabelgap+this->ytick_label_width2+this->axislabelgap+leftshift),
                            0.5f*this->height - downshift, 0 }};

                if (geom.width() > 2*this->fontsize) {
                    morph::Quaternion<float> leftrot;
                    leftrot.initFromAxisAngle (this->uz, -90.0f);
                    lbl->setupText (this->ylabel2, leftrot, lblpos+this->mv_offset, this->axiscolour);
                } else {
                    lbl->setupText (this->ylabel2, lblpos+this->mv_offset, this->axiscolour);
                }
                this->texts.push_back (lbl);
            }
        }

        //! Add the tick labels: 0, 1, 2 etc
        void drawTickLabels()
        {
            // Reset these members
            this->xtick_label_height = 0.0f;
            this->ytick_label_width = 0.0f;
            this->ytick_label_width2 = 0.0f;

            float x_for_yticks = 0.0f;
            float y_for_xticks = 0.0f;
            if (this->axisstyle == axisstyle::cross) {
                // Then labels go next to the zero axes
                x_for_yticks = this->abscissa_scale.transform_one (0);
                y_for_xticks = this->ord1_scale.transform_one (0);
            }

            for (unsigned int i = 0; i < this->xtick_posns.size(); ++i) {

                // Omit the 0 for 'cross' axes (or maybe shift its position)
                if (this->axisstyle == axisstyle::cross && this->xticks[i] == 0) { continue; }

                // Expunge any '0' from 0.123 so that it's .123 and so on.
                std::string s = this->graphNumberFormat (this->xticks[i]);

                // Issue: I need the width of the text ss.str() before I can create the
                // VisualTextModel, so need a static method like this:
                morph::VisualTextModel* lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, this->fontres);
                morph::TextGeometry geom = lbl->getTextGeometry (s);
                this->xtick_label_height = geom.height() > this->xtick_label_height ? geom.height() : this->xtick_label_height;
                morph::Vector<float> lblpos = {(float)this->xtick_posns[i]-geom.half_width(), y_for_xticks-(this->ticklabelgap+geom.height()), 0};
                lbl->setupText (s, lblpos+this->mv_offset, this->axiscolour);
                this->texts.push_back (lbl);
            }
            for (unsigned int i = 0; i < this->ytick_posns.size(); ++i) {

                // Omit the 0 for 'cross' axes (or maybe shift its position)
                if (this->axisstyle == axisstyle::cross && this->yticks[i] == 0) { continue; }

                std::string s = this->graphNumberFormat (this->yticks[i]);
                morph::VisualTextModel* lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, this->fontres);
                morph::TextGeometry geom = lbl->getTextGeometry (s);
                this->ytick_label_width = geom.width() > this->ytick_label_width ? geom.width() : this->ytick_label_width;
                morph::Vector<float> lblpos = {x_for_yticks-this->ticklabelgap-geom.width(), (float)this->ytick_posns[i]-geom.half_height(), 0};
                std::array<float, 3> clr = this->axiscolour;
                if (this->axisstyle == axisstyle::twinax && this->datastyles.size() > 0) {
                    clr = this->datastyles[0].policy == stylepolicy::lines ? this->datastyles[0].linecolour : this->datastyles[0].markercolour;
                }
                lbl->setupText (s, lblpos+this->mv_offset, clr);
                this->texts.push_back (lbl);
            }
            if (this->axisstyle == axisstyle::twinax) {
                x_for_yticks = this->width;
                this->ytick_label_width2 = 0.0f;
                for (unsigned int i = 0; i < this->ytick_posns2.size(); ++i) {
                    std::string s = this->graphNumberFormat (this->yticks2[i]);
                    morph::VisualTextModel* lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, this->fontres);
                    morph::TextGeometry geom = lbl->getTextGeometry (s);
                    this->ytick_label_width2 = geom.width() > this->ytick_label_width2 ? geom.width() : this->ytick_label_width2;
                    morph::Vector<float> lblpos = {x_for_yticks+this->ticklabelgap, (float)this->ytick_posns2[i]-geom.half_height(), 0};
                    std::array<float, 3> clr = this->axiscolour;
                    if (this->datastyles.size() > 1) {
                        clr = this->datastyles[1].policy == stylepolicy::lines ? this->datastyles[1].linecolour : this->datastyles[1].markercolour;

                    }
                    lbl->setupText (s, lblpos+this->mv_offset, clr);
                    this->texts.push_back (lbl);
                }
            }
        }

        void drawCrossAxes()
        {
            // Vert zero is not at model(0,0), have to get model coords of data(0,0)
            float _x0_mdl = this->abscissa_scale.transform_one (0);
            float _y0_mdl = this->ord1_scale.transform_one (0);
            this->computeFlatLine (this->idx,
                                   {_x0_mdl, -(this->axislinewidth*0.5f),             -this->thickness},
                                   {_x0_mdl, this->height+(this->axislinewidth*0.5f), -this->thickness},
                                   uz, this->axiscolour, this->axislinewidth*0.7f);
            // Horz zero
            this->computeFlatLine (this->idx,
                                   {0,           _y0_mdl, -this->thickness},
                                   {this->width, _y0_mdl, -this->thickness},
                                   uz, this->axiscolour, this->axislinewidth*0.7f);

            for (auto xt : this->xtick_posns) {
                // Want to place lines in screen units. So transform the data units
                this->computeFlatLine (this->idx,
                                       {(float)xt, _y0_mdl,                      -this->thickness},
                                       {(float)xt, _y0_mdl - this->ticklength,   -this->thickness}, uz,
                                       this->axiscolour, this->axislinewidth*0.5f);
            }
            for (auto yt : this->ytick_posns) {
                this->computeFlatLine (this->idx,
                                       {_x0_mdl,                    (float)yt, -this->thickness},
                                       {_x0_mdl - this->ticklength, (float)yt, -this->thickness}, uz,
                                       this->axiscolour, this->axislinewidth*0.5f);
            }
        }

        //! Draw the axes for the graph
        void drawAxes()
        {
            // First, ensure that this->xtick_posns/xticks and this->ytick_posns/yticks are populated
            this->computeTickPositions();

            if (this->axisstyle == axisstyle::cross) { return this->drawCrossAxes(); }

            if (this->axisstyle == axisstyle::box
                || this->axisstyle == axisstyle::twinax
                || this->axisstyle == axisstyle::boxfullticks
                || this->axisstyle == axisstyle::boxcross
                || this->axisstyle == axisstyle::L) {

                // y axis
                this->computeFlatLine (this->idx,
                                       {0, -(this->axislinewidth*0.5f),             -this->thickness},
                                       {0, this->height + this->axislinewidth*0.5f, -this->thickness},
                                       uz, this->axiscolour, this->axislinewidth);
                // x axis
                this->computeFlatLine (this->idx,
                                       {0,           0, -this->thickness},
                                       {this->width, 0, -this->thickness},
                                       uz, this->axiscolour, this->axislinewidth);

                // Draw left and bottom ticks
                float tl = -this->ticklength;
                if (this->tickstyle == tickstyle::ticksin) { tl = this->ticklength; }

                for (auto xt : this->xtick_posns) {
                    // Want to place lines in screen units. So transform the data units
                    this->computeFlatLine (this->idx,
                                           {(float)xt, 0.0f, -this->thickness},
                                           {(float)xt, tl,   -this->thickness}, uz,
                                           this->axiscolour, this->axislinewidth*0.5f);
                }
                for (auto yt : this->ytick_posns) {
                    this->computeFlatLine (this->idx,
                                           {0.0f, (float)yt, -this->thickness},
                                           {tl,   (float)yt, -this->thickness}, uz,
                                           this->axiscolour, this->axislinewidth*0.5f);
                }

            }

            if (this->axisstyle == axisstyle::box
                || this->axisstyle == axisstyle::twinax
                || this->axisstyle == axisstyle::boxfullticks
                || this->axisstyle == axisstyle::boxcross) {
                // right axis
                this->computeFlatLine (this->idx,
                                       {this->width, -this->axislinewidth*0.5f,               -this->thickness},
                                       {this->width, this->height+(this->axislinewidth*0.5f), -this->thickness},
                                       uz, this->axiscolour, this->axislinewidth);
                // top axis
                this->computeFlatLine (this->idx,
                                       {0,           this->height, -this->thickness},
                                       {this->width, this->height, -this->thickness},
                                       uz, this->axiscolour, this->axislinewidth);

                float tl = this->ticklength;
                if (this->tickstyle == tickstyle::ticksin) {
                    tl = -this->ticklength;
                }
                // Draw top and right ticks if necessary
                if (this->axisstyle == axisstyle::boxfullticks) {
                    // Tick positions
                    for (auto xt : this->xtick_posns) {
                        // Want to place lines in screen units. So transform the data units
                        this->computeFlatLine (this->idx,
                                               {(float)xt, this->height,      -this->thickness},
                                               {(float)xt, this->height + tl, -this->thickness}, uz,
                                               this->axiscolour, this->axislinewidth*0.5f);
                    }
                    for (auto yt : this->ytick_posns) {
                        this->computeFlatLine (this->idx,
                                               {this->width,      (float)yt, -this->thickness},
                                               {this->width + tl, (float)yt, -this->thickness}, uz,
                                               this->axiscolour, this->axislinewidth*0.5f);
                    }
                } else if (this->axisstyle == axisstyle::twinax) {
                    // Draw ticks for y2
                    for (auto yt : this->ytick_posns2) {
                        this->computeFlatLine (this->idx,
                                               {this->width,      (float)yt, -this->thickness},
                                               {this->width + tl, (float)yt, -this->thickness}, uz,
                                               this->axiscolour, this->axislinewidth*0.5f);
                    }
                }

                if (this->axisstyle == axisstyle::boxcross) { this->drawCrossAxes(); }
            }
        }

        //! Generate vertices for a bar of a bar graph, with p1 and p2 defining the top
        //! left and right corners of the bar. bottom left and right assumed to be on x
        //! axis.
        void bar (morph::Vector<float>& p, const morph::DatasetStyle& style)
        {
            // To update the z position of the data, must also add z thickness to p[2]
            p[2] += thickness;

            morph::Vector<float> p1 = p;
            p1[0] -= style.markersize/2.0f;
            morph::Vector<float> p2 = p;
            p2[0] += style.markersize/2.0f;

            // Zero is at (height*dataaxisdist)
            morph::Vector<float> p1b = p1;
            p1b[1] = this->height * this->dataaxisdist;
            morph::Vector<float> p2b = p2;
            p2b[1] = this->height * this->dataaxisdist;

            this->computeFlatQuad (this->idx, p1b, p1, p2, p2b, style.markercolour);

            if (style.showlines == true) {
                p1b[2] += this->thickness/2.0f;
                p1[2] += this->thickness/2.0f;
                p2[2] += this->thickness/2.0f;
                p2b[2] += this->thickness/2.0f;
                this->computeFlatLineRnd (this->idx, p1b, p1,  this->uz, style.linecolour, style.linewidth, 0.0f, false, true);
                this->computeFlatLineRnd (this->idx, p1,  p2,  this->uz, style.linecolour, style.linewidth, 0.0f, true, true);
                this->computeFlatLineRnd (this->idx, p2,  p2b, this->uz, style.linecolour, style.linewidth, 0.0f, true, false);
            }
        }

        //! Special code to draw a marker representing a bargraph bar for the legend
        void bar_symbol (morph::Vector<float>& p, const morph::DatasetStyle& style)
        {
            p[2] += this->thickness;

            morph::Vector<float> p1 = p;
            p1[0] -= 0.035f; // Note fixed size for legend
            morph::Vector<float> p2 = p;
            p2[0] += 0.035f;

            // Zero is at (height*dataaxisdist)
            morph::Vector<float> p1b = p1;
            p1b[1] -= 0.03f;
            morph::Vector<float> p2b = p2;
            p2b[1] -= 0.03f;

            float outline_width = 0.005f; // also fixed

            this->computeFlatQuad (this->idx, p1b, p1, p2, p2b, style.markercolour);

            if (style.showlines == true) {
                p1b[2] += this->thickness;
                p1[2] += this->thickness;
                p2[2] += this->thickness;
                p2b[2] += this->thickness;
                this->computeFlatLineRnd (this->idx, p1b, p1,  this->uz, style.linecolour, outline_width, 0.0f, true, true);
                this->computeFlatLineRnd (this->idx, p1,  p2,  this->uz, style.linecolour, outline_width, 0.0f, true, true);
                this->computeFlatLineRnd (this->idx, p2,  p2b, this->uz, style.linecolour, outline_width, 0.0f, true, true);
                this->computeFlatLineRnd (this->idx, p2b, p1b, this->uz, style.linecolour, outline_width, 0.0f, true, true);
            }
        }

        //! Generate vertices for a marker of the given style at location p
        void marker (morph::Vector<float>& p, const morph::DatasetStyle& style)
        {
            switch (style.markerstyle) {
            case morph::markerstyle::triangle:
            case morph::markerstyle::uptriangle:
            {
                this->polygonMarker (p, 3, style);
                break;
            }
            case morph::markerstyle::downtriangle:
            {
                this->polygonFlattop (p, 3, style);
                break;
            }
            case morph::markerstyle::square:
            {
                this->polygonFlattop (p, 4, style);
                break;
            }
            case morph::markerstyle::diamond:
            {
                this->polygonMarker (p, 4, style);
                break;
            }
            case morph::markerstyle::pentagon:
            {
                this->polygonFlattop (p, 5, style);
                break;
            }
            case morph::markerstyle::uppentagon:
            {
                this->polygonMarker (p, 5, style);
                break;
            }
            case morph::markerstyle::hexagon:
            {
                this->polygonFlattop (p, 6, style);
                break;
            }
            case morph::markerstyle::uphexagon:
            {
                this->polygonMarker (p, 6, style);
                break;
            }
            case morph::markerstyle::heptagon:
            {
                this->polygonFlattop (p, 7, style);
                break;
            }
            case morph::markerstyle::upheptagon:
            {
                this->polygonMarker (p, 7, style);
                break;
            }
            case morph::markerstyle::octagon:
            {
                this->polygonFlattop (p, 8, style);
                break;
            }
            case morph::markerstyle::upoctagon:
            {
                this->polygonMarker (p, 8, style);
                break;
            }
            case morph::markerstyle::circle:
            default:
            {
                this->polygonMarker (p, 20, style);
                break;
            }
            }
        }

        static constexpr bool three_d_pucks = false;
        // Create an n sided polygon with first vertex 'pointing up'
        void polygonMarker  (morph::Vector<float> p, int n, const morph::DatasetStyle& style)
        {
            if constexpr (three_d_pucks) {
                morph::Vector<float> pend = p;
                p[2] += this->thickness*Flt{0.5};
                pend[2] -= this->thickness*Flt{0.5};
                this->computeTube (this->idx, p, pend, ux, uy,
                                   style.markercolour, style.markercolour,
                                   style.markersize*Flt{0.5}, n);
            } else {
                p[2] += this->thickness;
                this->computeFlatPoly (this->idx, p, ux, uy,
                                       style.markercolour,
                                       style.markersize*Flt{0.5}, n);
            }
        }

        // Create an n sided polygon with a flat edge 'pointing up'
        void polygonFlattop (morph::Vector<float> p, int n, const morph::DatasetStyle& style)
        {
            if constexpr (three_d_pucks) {
                morph::Vector<float> pend = p;
                p[2] += this->thickness*Flt{0.5};
                pend[2] -= this->thickness*Flt{0.5};
                this->computeTube (this->idx, p, pend, ux, uy,
                                   style.markercolour, style.markercolour,
                                   style.markersize*Flt{0.5}, n, morph::PI_F/(float)n);
            } else {
                p[2] += this->thickness;
                this->computeFlatPoly (this->idx, p, ux, uy,
                                       style.markercolour,
                                       style.markersize*Flt{0.5}, n, morph::PI_F/(float)n);
            }
        }

        // Given the data, compute the ticks (or use the ones that client code gave us)
        void computeTickPositions()
        {
            if (this->manualticks == true) {
                std::cout << "Writeme: Implement a manual tick-setting scheme\n";
            } else {
                if (!(this->abscissa_scale.ready() && this->ord1_scale.ready())) {
                    throw std::runtime_error ("abscissa and ordinate Scales not set. Is there data?");
                }
                // Compute locations for ticks...
                Flt _xmin = this->abscissa_scale.inverse_one (this->abscissa_scale.range_min);
                Flt _xmax = this->abscissa_scale.inverse_one (this->abscissa_scale.range_max);
                Flt _ymin = this->ord1_scale.inverse_one (this->ord1_scale.range_min);
                Flt _ymax = this->ord1_scale.inverse_one (this->ord1_scale.range_max);
                Flt _ymin2 = Flt{0};
                Flt _ymax2 = Flt{1};
                if (this->ord2_scale.ready()) {
                    _ymin2 = this->ord2_scale.inverse_one (this->ord2_scale.range_min);
                    _ymax2 = this->ord2_scale.inverse_one (this->ord2_scale.range_max);
                }
                if constexpr (gv_debug) {
                    std::cout << "x ticks between " << _xmin << " and " << _xmax << " in data units\n";
                    std::cout << "y ticks between " << _ymin << " and " << _ymax << " in data units\n";
                }
                float realmin = this->abscissa_scale.inverse_one (0);
                float realmax = this->abscissa_scale.inverse_one (this->width);
                this->xticks = this->maketicks (_xmin, _xmax, realmin, realmax);
                realmin = this->ord1_scale.inverse_one (0);
                realmax = this->ord1_scale.inverse_one (this->height);
                this->yticks = this->maketicks (_ymin, _ymax, realmin, realmax);

                if (this->ord2_scale.ready()) {
                    realmin = this->ord2_scale.inverse_one (0);
                    realmax = this->ord2_scale.inverse_one (this->height);
                    this->yticks2 = this->maketicks (_ymin2, _ymax2, realmin, realmax);
                }

                this->xtick_posns.resize (this->xticks.size());
                this->abscissa_scale.transform (xticks, xtick_posns);

                this->ytick_posns.resize (this->yticks.size());
                this->ord1_scale.transform (yticks, ytick_posns);

                if (this->ord2_scale.ready()) {
                    this->ytick_posns2.resize (this->yticks2.size());
                    this->ord2_scale.transform (yticks2, ytick_posns2);
                }
            }
        }

    public:
        //! Graph data coordinates. A vector of vectors of pointers to data, with one
        //! pointer for each graph in the model.
        std::vector<std::vector<Vector<float>>*> graphDataCoords;
        //! A scaling for the abscissa.
        morph::Scale<Flt> abscissa_scale;
        //! A scaling for the first (left hand) ordinate
        morph::Scale<Flt> ord1_scale;
        //! A scaling for the second (right hand) ordinate, if it's a twin axis graph
        morph::Scale<Flt> ord2_scale;
        //! What's the scaling policy for the abscissa?
        morph::scalingpolicy scalingpolicy_x = morph::scalingpolicy::autoscale;
        //! If required, the abscissa's minimum/max data values
        Flt datamin_x = Flt{0};
        Flt datamax_x = Flt{1};
        //! What's the scaling policy for the ordinate?
        morph::scalingpolicy scalingpolicy_y = morph::scalingpolicy::autoscale;
        //! If required, the ordinate's minimum/max data values
        Flt datamin_y = Flt{0};
        Flt datamax_y = Flt{1};
        //! If required, the second ordinate's minimum/max data values (twinax)
        Flt datamin_y2 = Flt{0};
        Flt datamax_y2 = Flt{1};
        //! A vector of styles for the datasets to be displayed on this graph
        std::vector<DatasetStyle> datastyles;
        //! A default policy for showing datasets - lines, markers or both
        morph::stylepolicy policy = stylepolicy::both;
        //! axis features, starting with the colour for the axis box/lines. Text also
        //! takes this colour.
        std::array<float, 3> axiscolour = {0,0,0};
        //! Set axis and text colours for a dark or black background
        bool darkbg = false;
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
        //! The ytick values that should be displayed on the right hand axis for a twinax graph
        std::deque<Flt> yticks2;
        //! The positions, along the right hand y axis (in model space) for the yticks2
        std::deque<Flt> ytick_posns2;
        // Default font
        morph::VisualFont font = morph::VisualFont::DVSans;
        //! Font resolution - determines how textures for glyphs are generated. If your
        //! labels will be small, this should be smaller. If labels are large, then it
        //! should be increased.
        int fontres = 24;
        //! The font size is the width of an m in the chosen font, in model units
        float fontsize = 0.05;
        //! A separate fontsize for the axis labels, incase these should be different from the tick labels
        float axislabelfontsize = fontsize;
        // might need tickfontsize and axisfontsize
        //! EITHER Gap from the y axis to the right hand of the y axis tick label text
        //! quads OR from the x axis to the top of the x axis tick label text quads
        float ticklabelgap = 0.05;
        //! The gap from the left side of the y tick labels to the right side of the
        //! axis label (or similar for the x axis label)
        float axislabelgap = 0.05;
        //! The x axis label
        std::string xlabel = "x";
        //! The y axis label
        std::string ylabel = "y";
        //! Second y axis label
        std::string ylabel2 = "y2";
        //! Whether or not to show a legend
        bool legend = true;

    protected:
        //! This is used to set a spacing between elements in the graph (markers and
        //! lines) so that some objects (like a marker) is viewed 'on top' of another
        //! (such as a line). If it's too small, and the graph is far away in the scene,
        //! then precision errors can cause colour mixing.
        float thickness = 0.002f;
        //! width is how wide the graph axes will be, in 3D model coordinates
        float width = 1.0f;
        //! height is how high the graph axes will be, in 3D model coordinates
        float height = 1.0f;
        //! What proportion of the graph width/height should be allowed as a space
        //! between the min/max and the axes? Can be 0.0f;
        float dataaxisdist = 0.04f;
        //! The axes for orientation of the graph visual, which is 2D within the 3D environment.
        morph::Vector<float> ux = {1,0,0};
        morph::Vector<float> uy = {0,1,0};
        morph::Vector<float> uz = {0,0,1};

        //! Temporary storage for the max height of the xtick labels
        float xtick_label_height = 0.0f;
        //! Temporary storage for the max width of the ytick labels
        float ytick_label_width = 0.0f;
        float ytick_label_width2 = 0.0f;
    };

} // namespace morph
