/*!
 * \file GraphVisual
 *
 * \author Seb James
 * \date 2020
 */
#pragma once

#include <iostream>
#include <array>
#include <vector>
#include <deque>
#include <cmath>
#include <sstream>
#include <memory>
#include <cstdint>
#include <morph/mathconst.h>
#include <morph/tools.h>
#include <morph/scale.h>
#include <morph/range.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/quaternion.h>
#include <morph/histo.h>
#include <morph/colour.h>
#include <morph/gl/version.h>
#include <morph/VisualModel.h>
#include <morph/graphing.h>
#include <morph/graphstyles.h>
#include <morph/ColourMap.h>
#include <morph/Grid.h>
#include <morph/DatasetStyle.h>
#include <morph/VisualTextModel.h>

namespace morph {

    /*!
     * A VisualModel for showing a 2D graph.
     */
    template <typename Flt, int glver = morph::gl::version_4_1>
    class GraphVisual : public VisualModel<glver>
    {
    public:
        //! Constructor which sets just the shader programs and the model view offset
        GraphVisual(const vec<float> _offset)
        {
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->ord1_scale.do_autoscale = true;
            this->ord2_scale.do_autoscale = true;
            this->abscissa_scale.do_autoscale = true;
            // Graphs don't rotate by default. If you want yours to, set this false in your client code.
            this->twodimensional = true;
        }

        //! Set true for any optional debugging
        static constexpr bool gv_debug = false;

        //! Append a single datum onto the relevant graph. Build on existing data in
        //! graphDataCoords. Finish up with a call to completeAppend(). didx is the data
        //! index and counts up from 0. Have to save _abscissa and _ordinate in a local
        //! copy of the data to be able to rescale.
        void append (const Flt& _abscissa, const Flt& _ordinate, const unsigned int didx)
        {
            this->pendingAppended = true;
            // Transfor the data into temporary containers sd and ad
            Flt o = Flt{0};
            if (this->datastyles[didx].axisside == morph::axisside::left) {
                this->ord1.push_back (_ordinate);
                this->absc1.push_back (_abscissa);
                try {
                    o = this->ord1_scale.transform_one (_ordinate);
                } catch (const std::exception& e) {
                    std::cerr << "Error scaling ordinate 1 datum: " << e.what() << "\n";
                    throw e;
                }
            } else {
                this->ord2.push_back (_ordinate);
                this->absc2.push_back (_abscissa);
                try {
                    o = this->ord2_scale.transform_one (_ordinate);
                } catch (const std::exception& e) {
                    std::cerr << "Error scaling ordinate 2 datum: " << e.what() << "\n";
                    throw e;
                }
            }

            Flt a = Flt{0};
            try {
                a = this->abscissa_scale.transform_one (_abscissa);
            } catch (const std::exception& e) {
                std::cerr << "Error scaling abscissa datum: " << e.what() << "\n";
                throw e;
            }

            // Now sd and ad can be used to construct dataCoords x/y. They are used to
            // set the position of each datum into dataCoords
            if (graphDataCoords.size() < didx + 1) {
                // Need to add an additional graphDataCoords to receive data. This can occur after
                // appending the first data point of a first dataset and then appending the first
                // data point of a second dataset to an otherwise empty graph.
                this->graphDataCoords.push_back (std::make_unique<std::vector<morph::vec<float>>>(0u, morph::vec<float>{0,0,0}));
                // As well as creating a new, empty graphDataCoords, we have to add the right datastyle
                if (this->datastyles[didx].axisside == morph::axisside::left) {
                    this->datastyles.push_back (this->ds_ord1);
                } else {
                    this->datastyles.push_back (this->ds_ord2);
                }
            }

            unsigned int oldsz = this->graphDataCoords[didx]->size();
            (this->graphDataCoords[didx])->resize (oldsz+1);
            this->graphDataCoords[didx].get()->at(oldsz) = morph::vec<float>{ static_cast<float>(a), static_cast<float>(o), float{0} };
            int redraw_plot = 0;
            morph::range<Flt> xrange = this->datarange_x;
            morph::range<Flt> yrange = this->datarange_y;
            morph::range<Flt> y2range = this->datarange_y2;
            // check x axis
            if (this->auto_rescale_x) { redraw_plot += xrange.update (_abscissa) ? 1 : 0; }

            // check y axis
            if (this->auto_rescale_y) {
                if (this->datastyles[didx].axisside == morph::axisside::left) {
                    redraw_plot += yrange.update (this->ord1.back()) ? 1 : 0;
                } else {
                    redraw_plot += y2range.update (this->ord2.back()) ? 1 : 0;
                }
            }

            // update graph if necessary
            if (redraw_plot > 0) {
                this->clear_graph_data();

                // setdata or this function will re-add these
                this->graphDataCoords.clear();
                this->datastyles.clear();

                this->pendingAppended = true; // as the graph will be re-drawn
                if (didx == 0) { this->abscissa_scale.reset(); }
                if (this->datastyles[didx].axisside == morph::axisside::left) {
                    this->ord1_scale.reset();
                } else {
                    this->ord2_scale.reset();
                }
                this->setlimits (xrange, yrange, y2range);

                if (!this->ord1.empty()) {
                    // vvec, vvec, datasetstyle
                    this->setdata (this->absc1, this->ord1, this->ds_ord1);
                }
                if (!this->ord2.empty()) {
                    this->setdata (this->absc2, this->ord2, this->ds_ord2);
                }
            }

            VisualModel<glver>::clear(); // Get rid of the vertices.
            this->initializeVertices(); // Re-build
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
            VisualModel<glver>::render();
        }

        //! Clear all the coordinate data for the graph, but leave the containers in place.
        void clear_graph_data()
        {
            unsigned int dsize = this->graphDataCoords.size();
            for (unsigned int i = 0; i < dsize; ++i) {
                this->graphDataCoords[i]->clear();
            }
            this->reinit();
        }

        //! Update the data for the graph, recomputing the vertices when done.
        template <typename Ctnr1, typename Ctnr2>
        std::enable_if_t<morph::is_copyable_container<Ctnr1>::value
                         && morph::is_copyable_container<Ctnr2>::value, void>
        update (const Ctnr1& _abscissae, const Ctnr2& _data, const unsigned int data_idx)
        {
            unsigned int dsize = _data.size();
            morph::range<Flt> datarange;

            if (_abscissae.size() != dsize) {
                throw std::runtime_error ("GraphVisual::update: size mismatch");
            }

            if (data_idx >= this->graphDataCoords.size()) {
                std::cout << "Can't add data at graphDataCoords index " << data_idx << std::endl;
                return;
            }

            // Ensure the vector at data_idx has enough capacity for the updated data
            this->graphDataCoords[data_idx]->resize (dsize);

            // Are we auto-rescaling the x axis?
            if (this->auto_rescale_x) {
                this->abscissa_scale.reset();
                datarange = this->datarange_x;
                for (auto x_val : _abscissae) { datarange.update (x_val); }
                this->setlimits_x (datarange, true);
                this->abscissa_scale.compute_scaling (this->datarange_x);
            }

            // Transform the data into temporary containers sd and ad. Note call of
            // abscissa_scale.transform comes AFTER the auto_rescale_x logic
            std::vector<Flt> ad (dsize, Flt{0});
            this->abscissa_scale.transform (_abscissae, ad);

            std::vector<Flt> sd (dsize, Flt{0});
            if (this->datastyles[data_idx].axisside == morph::axisside::left) {
                // check min and max of the y axis
                if (this->auto_rescale_y && this->auto_rescale_fit) {
                    this->ord1_scale.reset();
                    // Find the data range in _data and setlimits_y accordingly
                    datarange.search_init();
                    for (auto y_val : _data) { datarange.update (y_val); }
                    this->setlimits_y (datarange, true);
                    this->ord1_scale.compute_scaling (this->datarange_y);

                } else if (this->auto_rescale_y) {
                    this->ord1_scale.reset();
                    // Starting with datarange_y, update datarange.
                    datarange = this->datarange_y;
                    for (auto y_val : _data) { datarange.update (y_val); }
                    this->setlimits_y (datarange, true);
                    this->ord1_scale.compute_scaling (this->datarange_y);
                }
                // scale data with the axis
                this->ord1_scale.transform (_data, sd);
            } else {
                // Similar to the above, for the y2 axis
                if (this->auto_rescale_y && this->auto_rescale_fit) {
                    this->ord2_scale.reset();
                    datarange.search_init();
                    for (auto y_val : _data) { datarange.update (y_val); }
                    this->setlimits_y2 (datarange, true);
                    this->ord2_scale.compute_scaling (this->datarange_y2);

                } else if (this->auto_rescale_y) {
                    this->ord2_scale.reset();
                    datarange = this->datarange_y2;
                    for (auto y_val : _data) { datarange.update (y_val); }
                    this->setlimits_y2 (datarange, true);
                    this->ord2_scale.compute_scaling (this->datarange_y2);
                }
                // scale data with the axis
                this->ord2_scale.transform (_data, sd);
            }

            // Now sd and ad can be used to construct dataCoords x/y. They are used to
            // set the position of each datum into dataCoords
            for (unsigned int i = 0; i < dsize; ++i) {
                this->graphDataCoords[data_idx].get()->at(i) = morph::vec<float>{ static_cast<float>(ad[i]), static_cast<float>(sd[i]), float{0} };
            }

            this->clearTexts(); // VisualModel::clearTexts()
            this->reinit();
        }

        //! update() overload that accepts vvec of coords
        void update (const morph::vvec<morph::vec<Flt, 2>>& _coords, const unsigned int data_idx)
        {
            std::vector<Flt> absc (_coords.size(), Flt{0});
            std::vector<Flt> ord (_coords.size(), Flt{0});
            for (unsigned int i = 0; i < _coords.size(); ++i) {
                absc[i] = _coords[i][0];
                ord[i] = _coords[i][1];
            }
            this->update (absc, ord, data_idx);
        }

        //! update() overload that allows you also to set the data label
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        void update (const Container<T, Allocator>& _abscissae,
                     const Container<T, Allocator>& _data, std::string datalabel, const unsigned int data_idx)
        {
            if (data_idx >= this->datastyles.size()) {
                std::cout << "Can't add change data label at graphDataCoords index " << data_idx << std::endl;
                return;
            }
            this->datastyles[data_idx].datalabel = datalabel;
            this->update (_abscissae, _data, data_idx);
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

        //! Prepare an as-yet empty dataset.
        void prepdata (const std::string name = "", const morph::axisside axisside = morph::axisside::left)
        {
            std::vector<Flt> emptyabsc;
            std::vector<Flt> emptyord;
            this->setdata (emptyabsc, emptyord, name, axisside);
        }

        //! Prepare an as-yet empty dataset with a specified DatasetStyle.
        void prepdata (const DatasetStyle& ds)
        {
            std::vector<Flt> emptyabsc;
            std::vector<Flt> emptyord;
            this->setdata (emptyabsc, emptyord, ds);
        }

        //! Set a dataset into the graph using default styles, incrementing colour and
        //! marker shape as more datasets are included in the graph.
        template <typename Ctnr1, typename Ctnr2>
        std::enable_if_t<morph::is_copyable_container<Ctnr1>::value
                         && morph::is_copyable_container<Ctnr2>::value, void>
        setdata (const Ctnr1& _abscissae, const Ctnr2& _data,
                 const std::string name = "", const morph::axisside axisside = morph::axisside::left)
        {
            DatasetStyle ds(this->policy);
            ds.axisside = axisside;
            if (!name.empty()) { ds.datalabel = name; }
            unsigned int data_index = static_cast<unsigned int>(this->graphDataCoords.size());
            this->setstyle (ds, DatasetStyle::datacolour(data_index), DatasetStyle::datamarkerstyle (data_index));
            this->setdata (_abscissae, _data, ds);
        }

        //! setdata overload that accepts vvec of coords (as morph::vec<Flt, 2>)
        void setdata (const morph::vvec<morph::vec<Flt, 2>>& _coords,
                      const std::string name = "", const morph::axisside axisside = morph::axisside::left)
        {
            // Split coords into two vectors then call setdata() overload
            std::vector<Flt> absc (_coords.size(), Flt{0});
            std::vector<Flt> ord (_coords.size(), Flt{0});
            for (unsigned int i = 0; i < _coords.size(); ++i) {
                absc[i] = _coords[i][0];
                ord[i] = _coords[i][1];
            }
            this->setdata (absc, ord, name, axisside);
        }

        //! setdata overload that plots quivers on a grid, scaling the grid's coordinates suitably?
        void setdata (const morph::Grid<unsigned int, Flt>& g, const morph::vvec<morph::vec<Flt, 2>>& _quivs,
                      const DatasetStyle& ds)
        {
            // _quivs should have same size as g.n()
            if (_quivs.size() != g.n()) {
                std::stringstream ee;
                ee << "GraphVisual::setdata: Size mismatch. Grid has " << g.n()
                   << " elements but there are " << _quivs.size() << " quivers";
                throw std::runtime_error (ee.str());
            }

            if (ds.markerstyle != morph::markerstyle::quiver
                && ds.markerstyle != morph::markerstyle::quiver_fromcoord
                && ds.markerstyle != morph::markerstyle::quiver_tocoord) {
                throw std::runtime_error ("GraphVisual::setdata: markerstyle must be "
                                          "morph::markerstyle::quiver(_fromcoord/_tocoord)"
                                          " for this setdata() overload");
            }

            // Copy _quivs
            this->quivers.resize (_quivs.size(), { Flt{0}, Flt{0}, Flt{0} });
            for (unsigned int i = 0; i < _quivs.size(); ++i) {
                this->quivers[i][0] = _quivs[i][0];
                this->quivers[i][1] = _quivs[i][1];
            }

            auto _abscissae = g.get_abscissae();
            auto _data = g.get_ordinates();

            // From g.v_c we get coordinates. These have to be copied into graphDataCoords
            // with the appropriate scaling.
            // from grid get absc1 and ord1
            if (ds.axisside == morph::axisside::left) {
                this->absc1.set_from (_abscissae);
                this->ord1.set_from (_data);
                this->ds_ord1 = ds;
            } else {
                this->absc2.set_from (_abscissae);
                this->ord2.set_from (_data);
                this->ds_ord2 = ds;
            }

            unsigned int dsize = _quivs.size();
            unsigned int didx = this->graphDataCoords.size();
            this->graphDataCoords.push_back (std::make_unique<std::vector<morph::vec<float>>>(dsize, morph::vec<float>{ 0.0f, 0.0f, 0.0f }));
            this->datastyles.push_back (ds);

            // Compute the ord1_scale and asbcissa_scale for the first added dataset only
            if (ds.axisside == morph::axisside::left) {
                if (this->ord1_scale.ready() == false) { this->compute_scaling (_abscissae, _data, ds.axisside); }
            } else {
                if (this->ord2_scale.ready() == false) { this->compute_scaling (_abscissae, _data, ds.axisside); }
            }

            if (dsize > 0) {
                // Transform the coordinate data into temporary containers
                std::vector<Flt> ad (g.n(), Flt{0});
                std::vector<Flt> sd (g.n(), Flt{0});
                // Extract x coordinates and y coordinates from Grid
                morph::vvec<Flt> g_v_x (g.n(), Flt{0});
                morph::vvec<Flt> g_v_y (g.n(), Flt{0});
                for (unsigned int i = 0; i < g.n(); i++) {
                    g_v_x[i] = g.v_c[i][0];
                    g_v_y[i] = g.v_c[i][1];
                }

                auto _dx = g.get_dx();
                if (ds.axisside == morph::axisside::left) {
                    this->ord1_scale.transform (g_v_y, sd);
                    this->quiver_grid_spacing[1] = _dx[1] * this->ord1_scale.getParams(0);
                } else {
                    this->ord2_scale.transform (g_v_y, sd);
                    this->quiver_grid_spacing[1] = _dx[1] * this->ord2_scale.getParams(0);
                }
                this->abscissa_scale.transform (g_v_x, ad);
                this->quiver_grid_spacing[0] = _dx[0] * this->abscissa_scale.getParams(0);

                // Now sd and ad can be used to construct dataCoords x/y. They are used to
                // set the position of each datum into dataCoords
                for (unsigned int i = 0; i < dsize; ++i) {
                    this->graphDataCoords[didx].get()->at(i) = morph::vec<float>{ static_cast<float>(ad[i]), static_cast<float>(sd[i]), float{0} };
                }
            }
        }

        //! Set a dataset into the graph. Provide abscissa and ordinate and a dataset
        //! style. The locations of the markers for each dataset are computed and stored
        //! in this->graphDataCoords, one vector for each dataset.
        template <typename Ctnr1, typename Ctnr2>
        std::enable_if_t<morph::is_copyable_container<Ctnr1>::value
                         && morph::is_copyable_container<Ctnr2>::value, void>
        setdata (const Ctnr1& _abscissae, const Ctnr2& _data, const DatasetStyle& ds)
        {
            if (_abscissae.size() != _data.size()) {
                std::stringstream ee;
                ee << "GraphVisual::setdata: size mismatch. abscissa size " << _abscissae.size() << " and data size: " << _data.size();
                throw std::runtime_error (ee.str());
            }

            // Save data first
            if (ds.axisside == morph::axisside::left) {
                this->absc1.set_from (_abscissae);
                this->ord1.set_from (_data);
                this->ds_ord1 = ds;
            } else {
                this->absc2.set_from (_abscissae);
                this->ord2.set_from (_data);
                this->ds_ord2 = ds;
            }

            uint64_t dsize = _data.size();
            unsigned int didx = static_cast<unsigned int>(this->graphDataCoords.size());

            // Allocate memory for the new data coords, add the data style info and the
            // starting index for dataCoords
            this->graphDataCoords.push_back (std::make_unique<std::vector<morph::vec<float>>>(dsize, morph::vec<float>{0,0,0}));

            this->datastyles.push_back (ds);

            // Compute the ord1_scale and asbcissa_scale for the first added dataset only
            if (ds.axisside == morph::axisside::left) {
                if (this->ord1_scale.ready() == false) { this->compute_scaling (_abscissae, _data, ds.axisside); }
            } else {
                if (this->ord2_scale.ready() == false) { this->compute_scaling (_abscissae, _data, ds.axisside); }
            }

            if (dsize > 0) {
                // Transform the data into temporary containers sd and ad
                morph::vvec<Flt> ad (dsize, Flt{0});
                morph::vvec<Flt> sd (dsize, Flt{0});
                if (ds.axisside == morph::axisside::left) {
                    this->ord1_scale.transform (_data, sd);
                } else {
                    this->ord2_scale.transform (_data, sd);
                }
                this->abscissa_scale.transform (_abscissae, ad);

                // Now sd and ad can be used to construct dataCoords x/y. They are used to
                // set the position of each datum into dataCoords
                for (uint64_t i = 0; i < dsize; ++i) {
                    this->graphDataCoords[didx].get()->at(i) = morph::vec<float>{ static_cast<float>(ad[i]), static_cast<float>(sd[i]), float{0} };
                }
            }
        }

        //! Set data using two ranges as input
        void setdata (const morph::range<Flt> xx, const morph::range<Flt> yy, const DatasetStyle& ds)
        {
            morph::vvec<Flt> xxvv = { xx.min, xx.max };
            morph::vvec<Flt> yyvv = { yy.min, yy.max };
            this->setdata (xxvv, yyvv, ds);
        }

        //! setdata overload that accepts vvec of coords (as morph::vec<Flt, 2>)
        void setdata (const morph::vvec<morph::vec<Flt, 2>>& _coords, const DatasetStyle& ds)
        {
            // Split coords into two vectors then call setdata() overload
            std::vector<Flt> absc (_coords.size(), Flt{0});
            std::vector<Flt> ord (_coords.size(), Flt{0});
            for (unsigned int i = 0; i < _coords.size(); ++i) {
                absc[i] = _coords[i][0];
                ord[i] = _coords[i][1];
            }
            this->setdata (absc, ord, ds);
        }

        //! Special setdata for a morph::histo object
        template<typename H>
        void setdata (const morph::histo<H, Flt>& h, const std::string name = "")
        {
            DatasetStyle ds(morph::stylepolicy::bar);
            if (!name.empty()) { ds.datalabel = name; }
            // Because this overload of setdata sets bargraph data, I want it to force the graph to be stylepolicy::bar
            ds.policy = morph::stylepolicy::bar;
            ds.markersize = (this->width - this->width * 2 * this->dataaxisdist) * (h.binwidth / static_cast<Flt>(h.datarange.span()));

            // User may wish to change these by calling the setdata (const histo&, DatasetStyle&) overload
            ds.showlines = true;
            ds.linewidth = ds.markersize / 10.0f;
            unsigned int data_index = this->graphDataCoords.size();
            ds.markercolour = DatasetStyle::datacolour(data_index);
            ds.linecolour = morph::colour::black;

            this->setdata (h, ds);
        }

        /*!
         * Set graph from histogram with pre-configured datasetstyle (though it must have policy
         * stylepolicy::bar)
         *
         * \tparam bar_width_auto: if true, always automatically change the dataset style's
         * markersize based on the GraphVisual width and the histogram.
         *
         * If you want to manually change the histogram bar widths, then call
         *
         * morph::histo<H, Flt> h(data);
         * morph::DatasetStyle ds(morph::stylepolicy::bar);
         * // ds setup goes here including ds.markersize for bar width
         * gv->setdata<H, false> (h, ds);
         */
        template<typename H, bool bar_width_auto = true>
        void setdata (const morph::histo<H, Flt>& h, morph::DatasetStyle& ds)
        {
            if (ds.policy != morph::stylepolicy::bar) {
                throw std::runtime_error ("GraphVisual::setdata(histo, DatasetStyle): Your DatasetStyle policy must be morph::stylepolicy::bar");
            }
            if constexpr (bar_width_auto == true) {
                ds.markersize = (this->width - this->width * 2 * this->dataaxisdist) * (h.binwidth / static_cast<Flt>(h.datarange.span()));
            }
            if (this->scalingpolicy_y == morph::scalingpolicy::autoscale) {
                // Because this is bar graph data, make sure to compute the ord1_scale now from 0 ->
                // max and NOT from min -> max; change scaling_policy to manual_min
                this->scalingpolicy_y = morph::scalingpolicy::manual_min; // to autoscale max only
            } else if (this->scalingpolicy_y == morph::scalingpolicy::manual_max) {
                this->scalingpolicy_y = morph::scalingpolicy::manual;
            }
            // datarange_y min is always 0
            this->datarange_y.min = Flt{0};

            this->setdata (h.bins, h.proportions, ds);
        }

        /*!
         * Add vertical lines representing the x locations at which the function has the value
         * y_value on the graph. Note that the same abscissae and data must be passed to this
         * function as to the setdata() function. Line colour is copied from the passed-in dataset.
         *
         * This method does not draw a horizontal line on the graph at y_value.
         */
        template <typename Ctnr1, typename Ctnr2>
        requires (morph::is_copyable_container<Ctnr1>::value && morph::is_copyable_container<Ctnr2>::value)
        morph::vvec<Flt> add_y_crossing_lines (const Ctnr1& _abscissae, const Ctnr2& _data, const Flt y_value, const morph::DatasetStyle& ds_data)
        {
            morph::vvec<Flt> xvals = morph::GraphVisual<Flt>::x_at_y_value (_abscissae, _data, y_value);
            morph::range<Flt> yy (morph::range_init::for_search);
            for (auto d : _data) { yy.update (d); } // Find the range
            morph::DatasetStyle dsv (morph::stylepolicy::lines);
            if (ds_data.policy == morph::stylepolicy::lines) {
                dsv.linecolour = ds_data.linecolour;
            } else {
                dsv.linecolour = ds_data.markercolour;
            }
            dsv.linewidth = ds_data.linewidth * 0.5f; // Use a reduced width cf the original dataset style
            dsv.datalabel = ""; // Always empty the datalabel
            for (auto xv : xvals) {
                morph::range<Flt> xx = { xv, xv };
                this->setdata (xx, yy, dsv);
            }
            return xvals;
        }

        /*!
         * Add vertical lines representing the x locations at which the function has the value
         * y_value on the graph. Note that the same abscissae and data must be passed to this
         * function as to the setdata() function. Line colour is copied from the passed-in dataset.
         *
         * This method ALSO draws a horizontal line on the graph at y_value, using the DatasetStyle
         * ds_hline.
         */
        template <typename Ctnr1, typename Ctnr2>
        requires (morph::is_copyable_container<Ctnr1>::value && morph::is_copyable_container<Ctnr2>::value)
        morph::vvec<Flt> add_y_crossing_lines (const Ctnr1& _abscissae, const Ctnr2& _data, const Flt y_value,
                                               const morph::DatasetStyle& ds_data, const morph::DatasetStyle& ds_hline)
        {
            // Draw the horizontal line
            morph::range<Flt> yy = { y_value, y_value };
            morph::range<Flt> xx (morph::range_init::for_search);
            for (auto a : _abscissae) { xx.update (a); }
            this->setdata (xx, yy, ds_hline);
            // And the vertical y crossings
            return this->add_y_crossing_lines (_abscissae, _data, y_value, ds_data);
        }

        // Static function to find the crossings in the right money. Requires data to be passed in.
        // Return all the x values where the function crosses the y_value.
        template <typename Ctnr1, typename Ctnr2>
        requires (morph::is_copyable_container<Ctnr1>::value && morph::is_copyable_container<Ctnr2>::value)
        static morph::vvec<Flt> x_at_y_value (const Ctnr1& _abscissae, const Ctnr2& _data, const Flt y_value)
        {
            if (_abscissae.size() != _data.size()) {
                std::stringstream ee;
                ee << "GraphVisual::x_at_y_value: size mismatch. abscissa size "
                   << _abscissae.size() << " and data size: " << _data.size();
                throw std::runtime_error (ee.str());
            }
            // First find crossing points, for which we require that the y values are in vvec format
            morph::vvec<Flt> y_values (_data);
            morph::vvec<float> crossings = y_values.crossing_points (y_value);
            // Now, for each of crossings, we have to interpolate the points in _abscissae to get the x to return
            morph::vvec<Flt> x_values = {};
            for (const float crs : crossings) {
                // bool up = crs > float{0} ? true : false; // Don't care here, but could
                float crs_abs = std::abs (crs);
                int crs_i = static_cast<int>(crs_abs);
                if (crs_abs - static_cast<float>(crs_i) > 0.25f) {
                    // intermediate. Interpolate
                    morph::scale<Flt> interp;
                    interp.output_range = morph::range<Flt>{static_cast<Flt>(_data.at(crs_i)), static_cast<Flt>(_data.at(crs_i + 1))};
                    interp.compute_scaling (static_cast<Flt>(_abscissae.at(crs_i)), static_cast<Flt>(_abscissae.at(crs_i + 1)));
                    x_values.push_back (interp.inverse_one (y_value));
                } else {
                    // crossing is *on* crs_i
                    x_values.push_back (_abscissae.at(crs_i));
                }
            }
            return x_values;
        }

        // Now the x crossing lines
        template <typename Ctnr1, typename Ctnr2>
        requires (morph::is_copyable_container<Ctnr1>::value && morph::is_copyable_container<Ctnr2>::value)
        morph::vvec<Flt> add_x_crossing_lines (const Ctnr1& _abscissae, const Ctnr2& _data, const Flt x_value, const morph::DatasetStyle& ds_data)
        {
            morph::vvec<Flt> yvals = morph::GraphVisual<Flt>::y_at_x_value (_abscissae, _data, x_value);

            morph::range<Flt> xx (morph::range_init::for_search);
            for (auto a : _abscissae) { xx.update (a); } // Find the range
            morph::DatasetStyle dsv (morph::stylepolicy::lines);
            if (ds_data.policy == morph::stylepolicy::lines) {
                dsv.linecolour = ds_data.linecolour;
            } else {
                dsv.linecolour = ds_data.markercolour;
            }
            dsv.linewidth = ds_data.linewidth * 0.5f; // Use a reduced width cf the original dataset style
            dsv.datalabel = ""; // Always empty the datalabel
            for (auto yv : yvals) {
                morph::range<Flt> yy = { yv, yv };
                this->setdata (xx, yy, dsv);
            }
            return yvals;
        }
        /*!
         * Add horizontal lines representing the y locations at which the function has the value
         * x_value on the graph. Note that the same abscissae and data must be passed to this
         * function as to the setdata() function. Line colour is copied from the passed-in dataset.
         *
         * This method ALSO draws a vertical line on the graph at x_value, using the DatasetStyle
         * ds_vline.
         */
        template <typename Ctnr1, typename Ctnr2>
        requires (morph::is_copyable_container<Ctnr1>::value && morph::is_copyable_container<Ctnr2>::value)
        morph::vvec<Flt> add_x_crossing_lines (const Ctnr1& _abscissae, const Ctnr2& _data, const Flt x_value,
                                               const morph::DatasetStyle& ds_data, const morph::DatasetStyle& ds_vline)
        {
            // Draw the vertical line
            morph::range<Flt> xx = { x_value, x_value };
            morph::range<Flt> yy (morph::range_init::for_search);
            for (auto d : _data) { yy.update (d); }
            this->setdata (xx, yy, ds_vline);
            // And the horizontal x crossings
            return this->add_x_crossing_lines (_abscissae, _data, x_value, ds_data);
        }

        template <typename Ctnr1, typename Ctnr2>
        requires (morph::is_copyable_container<Ctnr1>::value && morph::is_copyable_container<Ctnr2>::value)
        static morph::vvec<Flt> y_at_x_value (const Ctnr1& _abscissae, const Ctnr2& _data, const Flt x_value)
        {
            if (_abscissae.size() != _data.size()) {
                std::stringstream ee;
                ee << "GraphVisual::y_at_x_value: size mismatch. abscissa size "
                   << _abscissae.size() << " and data size: " << _data.size();
                throw std::runtime_error (ee.str());
            }
            // First find crossing points, for which we require that the x values are in vvec format
            morph::vvec<Flt> x_values (_abscissae);
            morph::vvec<float> crossings = x_values.crossing_points (x_value);
            // Now, for each of crossings, we have to interpolate the points in _data to get the y to return
            morph::vvec<Flt> y_values = {};
            for (const float crs : crossings) {
                // bool up = crs > float{0} ? true : false; // Don't care here, but could
                float crs_abs = std::abs (crs);
                int crs_i = static_cast<int>(crs_abs);
                if (crs_abs - static_cast<float>(crs_i) > 0.25f) {
                    // intermediate. Interpolate
                    morph::scale<Flt> interp;
                    interp.output_range = morph::range<Flt>{static_cast<Flt>(_data.at(crs_i)), static_cast<Flt>(_data.at(crs_i + 1))};
                    interp.compute_scaling (static_cast<Flt>(_abscissae.at(crs_i)), static_cast<Flt>(_abscissae.at(crs_i + 1)));
                    y_values.push_back (interp.transform_one (x_value));
                } else {
                    // crossing is *on* crs_i
                    y_values.push_back (_data.at(crs_i));
                }
            }
            return y_values;
        }

    protected:
        //! Compute the scaling of ord1_scale and abscissa_scale according to the scalingpolicies
        template <typename Ctnr1, typename Ctnr2>
        std::enable_if_t<morph::is_copyable_container<Ctnr1>::value
                         && morph::is_copyable_container<Ctnr2>::value, void>
        compute_scaling (const Ctnr1& _abscissae, const Ctnr2& _data, const morph::axisside axisside)
        {
            morph::range<Flt> data_range = morph::MathAlgo::maxmin (_data);
            morph::range<Flt> absc_range = morph::MathAlgo::maxmin (_abscissae);

            this->resetsize (this->width, this->height);

            // x axis - the abscissa
            switch (this->scalingpolicy_x) {
            case morph::scalingpolicy::manual:
            {
                this->abscissa_scale.compute_scaling (this->datarange_x);
                break;
            }
            case morph::scalingpolicy::manual_min:
            {
                this->abscissa_scale.compute_scaling (this->datarange_x.min, absc_range.max);
                break;
            }
            case morph::scalingpolicy::manual_max:
            {
                this->abscissa_scale.compute_scaling (absc_range.min, this->datarange_x.max);
                break;
            }
            case morph::scalingpolicy::autoscale:
            default:
            {
                this->abscissa_scale.compute_scaling (absc_range);
                break;
            }
            }

            // y axis - the ordinate
            switch (this->scalingpolicy_y) {
            case morph::scalingpolicy::manual:
            {
                if (axisside == morph::axisside::left) {
                    this->ord1_scale.compute_scaling (this->datarange_y);
                } else {
                    this->ord2_scale.compute_scaling (this->datarange_y2);
                }
                break;
            }
            case morph::scalingpolicy::manual_min:
            {
                if (axisside == morph::axisside::left) {
                    this->ord1_scale.compute_scaling (this->datarange_y.min, data_range.max);
                } else {
                    this->ord2_scale.compute_scaling (this->datarange_y2.min, data_range.max);
                }
                break;
            }
            case morph::scalingpolicy::manual_max:
            {
                if (axisside == morph::axisside::left) {
                    this->ord1_scale.compute_scaling (data_range.min, this->datarange_y.max);
                } else {
                    this->ord2_scale.compute_scaling (data_range.min, this->datarange_y2.max);
                }
                break;
            }
            case morph::scalingpolicy::autoscale:
            default:
            {
                if (axisside == morph::axisside::left) {
                    this->ord1_scale.compute_scaling (data_range);
                } else {
                    this->ord2_scale.compute_scaling (data_range);
                }
                break;
            }
            }
        }

    public:

        //! Call before initializeVertices() to scale quiver lengths logarithmically
        void quiver_setlog() { this->quiver_length_scale.setlog(); }

        //! Setter for the dataaxisdist attribute
        void setdataaxisdist (float proportion)
        {
            if (!this->graphDataCoords.empty()) {
                throw std::runtime_error ("GraphVisual::setdataaxisdist: Call this function *before* using setdata to set the data");
            }
            this->dataaxisdist = proportion;
        }

        //! When the width/height change we have to change the output_range of our axis scales
        void resetsize (float _width, float _height)
        {
            this->width = _width;
            this->height = _height;

            // dataaxisdist is padding inside the axes
            float _extra = this->dataaxisdist * this->height;
            this->ord1_scale.output_range.min = _extra;
            this->ord1_scale.output_range.max = this->height - _extra;
            this->ord2_scale.output_range.min = _extra;
            this->ord2_scale.output_range.max = this->height - _extra;
            _extra = this->dataaxisdist * this->width;
            this->abscissa_scale.output_range.min = _extra;
            this->abscissa_scale.output_range.max = this->width - _extra;

            this->thickness = this->relative_thickness * this->width;
        }

        //! Set the graph size, in model units. Call before finalize() and setdata() and
        //! any manual setlimits calls.
        void setsize (float _width, float _height)
        {
            if (!this->graphDataCoords.empty()) {
                throw std::runtime_error ("GraphVisual::setsize: Set the size of your graph with setsize *before* using setdata to set the data");
            }
            this->resetsize (_width, _height);
        }

        // Make all the bits of the graph - fonts, line thicknesses, etc, bigger by
        // factor. Call before finalize() and setdata() and any manual setlimits calls.
        void zoomgraph (Flt factor)
        {
            if (!this->graphDataCoords.empty()) {
                throw std::runtime_error ("GraphVisual::zoomgraph: Set the size of your graph with zoomgraph *before* using setdata to set the data");
            }
            float _w = this->width;
            float _h = this->height;
            this->resetsize (_w*factor, _h*factor);

            this->fontsize *= factor;
            this->axislabelfontsize *= factor;
            this->ticklabelgap *= factor;
            this->axislabelgap *= factor;
            this->ticklength *= factor;
            this->axislinewidth *= factor;
            this->relative_thickness *= factor;
        }

        //! Set manual limits for the x axis (abscissa). Call after setsize/zoomgraph,
        //! but before setdata and finalize.
        void setlimits_x (const Flt _xmin, const Flt _xmax)
        {
            morph::range<Flt> range_x(_xmin, _xmax);
            this->setlimits_x (range_x);
        }

        //! Set manual limits for the x axis (abscissa) passing by morph::range. Call
        //! after setsize/zoomgraph, but before setdata and finalize.
        void setlimits_x (const morph::range<Flt>& range_x, bool force = false)
        {
            if (!force && !this->graphDataCoords.empty()) {
                throw std::runtime_error ("GraphVisual::setlimits_x: Set your axis limits *before* using setdata to set the data");
            }
            this->scalingpolicy_x = morph::scalingpolicy::manual;
            this->datarange_x = range_x;
            this->resetsize (this->width, this->height);
        }

        //! Set manual limits for the y axis (ordinate)
        void setlimits_y (const Flt _ymin, const Flt _ymax)
        {
            morph::range<Flt> range_y(_ymin, _ymax);
            this->setlimits_y (range_y);
        }

        //! Set manual limits for the x axis (abscissa) passing by morph::range
        void setlimits_y (const morph::range<Flt>& range_y, bool force = false)
        {
            if (!force && !this->graphDataCoords.empty()) {
                throw std::runtime_error ("GraphVisual::setlimits_y: Set your axis limits *before* using setdata to set the data");
            }
            this->scalingpolicy_y = morph::scalingpolicy::manual;
            this->datarange_y = range_y;
            this->resetsize (this->width, this->height);
        }

        //! Set manual limits for the second y axis (ordinate)
        void setlimits_y2 (const Flt _ymin2, const Flt _ymax2)
        {
            morph::range<Flt> range_y2(_ymin2, _ymax2);
            this->setlimits_y2 (range_y2);
        }

        //! Set manual limits for the x axis (abscissa) passing by morph::range
        void setlimits_y2 (const morph::range<Flt>& range_y2, bool force = false)
        {
            if (!force && !this->graphDataCoords.empty()) {
                throw std::runtime_error ("GraphVisual::setlimits_y2: Set your axis limits *before* using setdata to set the data");
            }
            this->scalingpolicy_y = morph::scalingpolicy::manual; // scalingpolicy_y common to both left and right axes?
            this->datarange_y2 = range_y2;
            this->resetsize (this->width, this->height);
        }

        // Axis ranges. The length of each axis could be determined from the data and
        // abscissas for a static graph, but for a dynamically updating graph, it's
        // going to be necessary to give a hint at how far the data/abscissas might need
        // to extend.
        void setlimits (const Flt _xmin, const Flt _xmax, const Flt _ymin, const Flt _ymax)
        {
            morph::range<Flt> range_x(_xmin, _xmax);
            morph::range<Flt> range_y(_ymin, _ymax);
            this->setlimits (range_x, range_y);
        }

        // Set axis limits for x/y passing by morph::range
        void setlimits (const morph::range<Flt>& range_x, const morph::range<Flt>& range_y)
        {
            if (!this->graphDataCoords.empty()) {
                throw std::runtime_error ("GraphVisual::setlimits: Set your axis limits *before* using setdata to set the data");
            }

            // Set limits with 4 args gives fully manual scaling
            this->scalingpolicy_x = morph::scalingpolicy::manual;
            this->datarange_x = range_x;
            this->scalingpolicy_y = morph::scalingpolicy::manual;
            this->datarange_y = range_y;

            // First make sure that the range_min/max are correctly set
            this->resetsize (this->width, this->height);
        }

        //! setlimits overload that sets BOTH left and right axes limits
        void setlimits (const Flt _xmin, const Flt _xmax,
                        const Flt _ymin, const Flt _ymax, const Flt _ymin2, const Flt _ymax2)
        {
            morph::range<Flt> range_x(_xmin, _xmax);
            morph::range<Flt> range_y(_ymin, _ymax);
            morph::range<Flt> range_y2(_ymin2, _ymax2);
            this->setlimits (range_x, range_y, range_y2);
        }

        //! setlimits overload that sets BOTH left and right axes limits, passing by morph::range
        void setlimits (const morph::range<Flt>& range_x,
                        const morph::range<Flt>& range_y, const morph::range<Flt>& range_y2)
        {
            if (!this->graphDataCoords.empty()) {
                throw std::runtime_error ("GraphVisual::setlimits: Set your axis limits *before* using setdata to set the data");
            }

            // Set limits with 4 args gives fully manual scaling
            this->scalingpolicy_x = morph::scalingpolicy::manual;
            this->datarange_x = range_x;
            this->scalingpolicy_y = morph::scalingpolicy::manual;
            this->datarange_y = range_y;
            this->datarange_y2 = range_y2;

            // First make sure that the range_min/max are correctly set
            this->resetsize (this->width, this->height);
        }

        //! Set the 'object thickness' attribute (maybe used just for 'object spacing')
        void setthickness (float th) { this->relative_thickness = th; }

        //! Tell this GraphVisual that it's going to be rendered on a dark background. Updates axis colour.
        void setdarkbg()
        {
            this->darkbg = true;
            this->axiscolour = {0.8f, 0.8f, 0.8f};
        }

    protected:

        //! Stores the length of each entry in graphDataCoords - i.e how many data
        //! points are in each graph curve
        std::vector<unsigned int> coords_lengths;

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

        //! Is the passed in coordinate within the graph axes (in the x/y sense, ignoring z)?
        bool within_axes (morph::vec<float>& datapoint)
        {
            bool within = false;
            if (datapoint[0] >= 0 && datapoint[0] <= this->width
                && datapoint[1] >= 0 && datapoint[1] <= this->height) {
                within = true;
            }
            return within;
        }

        //! Is the passed in coordinate within the graph axes (in the x sense, ignoring z)?
        bool within_axes_x (morph::vec<float>& dpt) { return (dpt[0] >= 0 && dpt[0] <= this->width); }
        bool within_axes_y (morph::vec<float>& dpt) { return (dpt[1] >= 0 && dpt[1] <= this->height); }

        //! dsi: data set iterator
        void drawDataCommon (unsigned int dsi, unsigned int coords_start, unsigned int coords_end, bool appending = false)
        {
            // Draw data markers
            if (this->datastyles[dsi].markerstyle != markerstyle::none) {

                if (this->datastyles[dsi].markerstyle == markerstyle::bar) { // Data markers are bars

                    for (unsigned int i = coords_start; i < coords_end; ++i) {
                        this->bar ((*this->graphDataCoords[dsi])[i], this->datastyles[dsi]);
                    }

                } else if (this->datastyles[dsi].markerstyle == markerstyle::quiver) { // Markers are quivers

                    // Check quivers exist and then proceed with code adapted from morph::QuiverVisual
                    uint64_t nquiv = this->quivers.size();
                    if ((*this->graphDataCoords[dsi]).size() == nquiv) {

                        // Prepare scaling functions
                        if (!this->quiver_colour_scale.ready()) { this->quiver_colour_scale.do_autoscale = true; }
                        if (!this->quiver_linear_scale.ready()) { this->quiver_linear_scale.do_autoscale = true; }
                        if (!this->quiver_length_scale.ready()) { this->quiver_length_scale.do_autoscale = true; }

                        // Have to derive some scaling info from the quivers first.
                        //morph::vvec<Flt> raw_qlengths (nquiv, Flt{0});           // 'raw' quiver lengths
                        morph::vvec<Flt> userscaled_qlengths (nquiv, Flt{0});    // 'user-scaled' quiver lengths (may exaggerate one axis)
                        morph::vvec<Flt> renorm_qlengths (nquiv, Flt{0});        // renormalized user-scaled quiver lengths with linear OR log scaling
                        morph::vvec<Flt> renorm_linear_qlengths (nquiv, Flt{0}); // renormalized user-scaled quiver lengths with linear scaling

                        // Compute the length of each quiver
                        for (uint64_t i = 0; i < nquiv; ++i) {
                            //raw_qlengths[i] = this->quivers[i].length();
                            userscaled_qlengths[i] = (this->quivers[i] * this->datastyles[dsi].quiver_gain * (Flt{0.5} * this->quiver_grid_spacing)).length();
                        }
                        // Transform the user scaled lengths into a length 0->1, possibly with a log transform
                        this->quiver_length_scale.transform (userscaled_qlengths, renorm_qlengths);
                        // Also create a guaranteed linearly renormalized vvec of lengths
                        this->quiver_linear_scale.transform (userscaled_qlengths, renorm_linear_qlengths);
                        // Compute a scaling factor from these to apply to the final quiver lengths
                        morph::vvec<Flt> lfactor = renorm_qlengths / renorm_linear_qlengths;
                        for (auto& lf : lfactor) { lf = lf < Flt{0} ? Flt{0} : lf; } // replace any negative factors with 0

                        // 'final' quivers, with scaling applied. From these computed final_qlengths which will give colours
                        morph::vvec<Flt> final_qlengths (nquiv, Flt{0});
                        morph::vvec<morph::vec<Flt, 3>> final_quivers (this->quivers);
                        for (uint64_t i = 0; i < nquiv; ++i) {
                            final_quivers[i] *= this->datastyles[dsi].quiver_gain * (Flt{0.5} * this->quiver_grid_spacing) * lfactor[i];
                            final_qlengths[i] = final_quivers[i].length();
                        }
                        // Replace any zero lengths with the value of the minimum length to ensure colour range goes from min to max not 0 to max
                        Flt fqlmin = final_qlengths.prune_zero().min();
                        final_qlengths.search_replace (Flt{0}, fqlmin);
                        // Then scaled the final_qlengths to range [0-1]
                        morph::vvec<Flt> colour_qlengths (nquiv, Flt{0});
                        this->quiver_colour_scale.transform (final_qlengths, colour_qlengths);

                        // Finally loop thru coords, drawing a quiver for each
                        if (static_cast<uint64_t>(coords_end) > nquiv) {
                            throw std::runtime_error ("GraphVisual::drawDataCommon: coords_end is off the end of quivers");
                        }
                        for (unsigned int i = coords_start; i < coords_end; ++i) {
                            this->quiver ((*this->graphDataCoords[dsi])[i], final_quivers[i], colour_qlengths[i], this->datastyles[dsi]);
                        }

                    } else {
                        std::cout << "(*this->graphDataCoords[dsi]).size() = "  << (*this->graphDataCoords[dsi]).size()
                                  << " does not match quivers size: " << quivers.size() << std::endl;
                    }

                } else { // Regular data markers

                    for (unsigned int i = coords_start; i < coords_end; ++i) {
                        if (this->within_axes ((*this->graphDataCoords[dsi])[i])) {
                            this->marker ((*this->graphDataCoords[dsi])[i], this->datastyles[dsi]);
                        } // else marker is outside graph axes so don't draw it
                    }
                }
            }
            if (this->datastyles[dsi].markerstyle == markerstyle::bar && this->datastyles[dsi].showlines == true) {
                // No need to do anything, lines will have been drawn by GraphVisual::bar()
            } else if (this->datastyles[dsi].showlines == true) {

                // If appending markers to a dataset, need to add the line preceding the first marker
                if (appending == true) { if (coords_start != 0) { coords_start -= 1; } }

                for (unsigned int i = coords_start+1; i < coords_end; ++i) {
                    // Draw tube from location -1 to location 0.
                    if (this->draw_beyond_axes == true
                        || (this->within_axes ((*this->graphDataCoords[dsi])[i-1])
                            && this->within_axes ((*this->graphDataCoords[dsi])[i]))) {

                        if (this->datastyles[dsi].markergap > 0.0f) {
                            auto point_to_point = (*this->graphDataCoords[dsi])[i] - (*this->graphDataCoords[dsi])[i-1];
                            if (point_to_point.length() > this->datastyles[dsi].markergap * 2.0f) {
                                // Draw solid lines between marker points with gaps between line and marker
                                this->computeFlatLine ((*this->graphDataCoords[dsi])[i-1], (*this->graphDataCoords[dsi])[i], this->uz,
                                                       this->datastyles[dsi].linecolour,
                                                       this->datastyles[dsi].linewidth, this->datastyles[dsi].markergap);
                            }
                        } else if (appending == true) {
                            // We are appending a line to an existing graph, so compute a single line with rounded ends
                            this->computeFlatLineRnd ((*this->graphDataCoords[dsi])[i-1], // start
                                                      (*this->graphDataCoords[dsi])[i],   // end
                                                      this->uz,
                                                      this->datastyles[dsi].linecolour,
                                                      this->datastyles[dsi].linewidth, 0.0f, true, false);
                        } else {
                            // No gaps, so draw a perfect set of joined up lines.

                            // To make this draw dotted or dashed lines, we have to
                            // track the length of the lines we've added to the graph
                            // and draw the alt colour (which may be bg colour) between the dashes.
                            if (i == 1+coords_start && (coords_end-coords_start)==2) {
                                // First and only line
                                this->computeFlatLine ((*this->graphDataCoords[dsi])[i-1], // start
                                                       (*this->graphDataCoords[dsi])[i],   // end
                                                       this->uz,
                                                       this->datastyles[dsi].linecolour,
                                                       this->datastyles[dsi].linewidth);
                            } else if (i == 1+coords_start) {
                                // First line
                                this->computeFlatLineN ((*this->graphDataCoords[dsi])[i-1], // start
                                                        (*this->graphDataCoords[dsi])[i],   // end
                                                        (*this->graphDataCoords[dsi])[i+1], // next
                                                        this->uz,
                                                        this->datastyles[dsi].linecolour,
                                                        this->datastyles[dsi].linewidth);
                            } else if (i == (coords_end-1)) {
                                // last line
                                this->computeFlatLineP ((*this->graphDataCoords[dsi])[i-1], (*this->graphDataCoords[dsi])[i],
                                                        (*this->graphDataCoords[dsi])[i-2],
                                                        this->uz,
                                                        this->datastyles[dsi].linecolour,
                                                        this->datastyles[dsi].linewidth);
                            } else {
                                // An intermediate line
                                this->computeFlatLine ((*this->graphDataCoords[dsi])[i-1], (*this->graphDataCoords[dsi])[i],
                                                       (*this->graphDataCoords[dsi])[i-2], (*this->graphDataCoords[dsi])[i+1],
                                                       this->uz,
                                                       this->datastyles[dsi].linecolour,
                                                       this->datastyles[dsi].linewidth);
                            }
                        }

                    } // else line segment is outside axes and we're not to draw it. IDEALLY I'd interpolate to draw the line UP to the axis.
                }
            }
        }

        // Defines a boolean 'true' that can be provided as arg to drawDataCommon()
        static constexpr bool appending_data = true;

        //! Draw markers and lines for data points that are being appended to a graph
        void drawAppendedData()
        {
            for (unsigned int dsi = 0; dsi < this->graphDataCoords.size(); ++dsi) {
                // Start is old end:
                unsigned int coords_start = this->coords_lengths[dsi];
                unsigned int coords_end = static_cast<unsigned int>(this->graphDataCoords[dsi]->size());
                this->coords_lengths[dsi] = coords_end;
                this->drawDataCommon (dsi, coords_start, coords_end, appending_data);
            }
        }

        //! Draw all markers and lines for datasets in the graph (as stored in graphDataCoords)
        void drawData()
        {
            unsigned int coords_start = 0;
            this->coords_lengths.resize (this->graphDataCoords.size());
            for (unsigned int dsi = 0; dsi < static_cast<unsigned int>(this->graphDataCoords.size()); ++dsi) {
                unsigned int coords_end = this->graphDataCoords[dsi]->size();
                // Record coords length for future appending:
                this->coords_lengths[dsi] = coords_end;
                this->drawDataCommon (dsi, coords_start, coords_end);
            }
        }

        //! Draw the graph legend, above the graph, rather than inside it (so much simpler!)
        void drawLegend()
        {
            unsigned int num_legends_max = static_cast<unsigned int>(this->graphDataCoords.size());

            // Text offset from marker to text
            morph::vec<float> toffset = {this->fontsize, 0.0f, 0.0f};

            // To determine the legend layout, will need all the text geometries
            std::vector<morph::TextGeometry> geom;

            std::map<unsigned int, std::unique_ptr<morph::VisualTextModel<glver>>> legtexts;

            morph::vvec<unsigned int> ds_indices; // dataset indices.

            float text_advance = 0.0f;
            int num_legends = 0;
            morph::TextFeatures tf(this->fontsize, this->fontres, false, morph::colour::black, this->font);
            for (unsigned int dsi = 0; dsi < num_legends_max; ++dsi) {
                // If no label, then draw no legend. Thus the effective num_legends may be smaller
                // than num_legends_max.
                if (this->datastyles[dsi].datalabel.empty()) { continue; }
                // Legend text. If all is well, this will be pushed onto the texts attribute and
                // deleted when the model is deconstructed.

                auto ltp = this->makeVisualTextModel (tf);
                geom.push_back (ltp->getTextGeometry (this->datastyles[dsi].datalabel));
                if (geom.back().total_advance > text_advance) { text_advance = geom.back().total_advance; }
                legtexts[dsi] = std::move(ltp);
                ds_indices.push_back (dsi);
                ++num_legends;
            }

            // If there are no legend texts to show, then clean up and return
            if (text_advance == 0.0f && !legtexts.empty()) {
                legtexts.clear();
                return;
            }

            // Adjust the text offset by the last entry in geom
            if (!geom.empty()) { toffset[1] -= geom.back().height()/2.0f; }

            // What's our legend grid arrangement going to be? Each column will advance by the
            // text_advance, some space and the size of the marker
            float col_advance = 2 * toffset[0] + text_advance;
            if (!datastyles.empty()) { col_advance += this->datastyles[0].markersize; }
            int max_cols = static_cast<int>((1.0f - this->dataaxisdist) / col_advance);
            if (max_cols < 1) { max_cols = 1; }
            int num_cols = num_legends <= max_cols ? num_legends : max_cols;
            int num_rows = num_legends;
            if (num_cols != 0) {
                num_rows = (num_legends / num_cols);
                num_rows += num_legends % num_cols ? 1 : 0;
            }

            // Label position
            morph::vec<float> lpos = {this->dataaxisdist, 0.0f, 0.0f};
            int cur_entry = 0;
            for (auto _dsi : ds_indices) {
                int dsi = static_cast<int>(_dsi);
                // Compute the row and column for this legend entry
                if (num_cols == 0) { throw std::runtime_error ("GraphVisual::drawLegend: Why is num_cols 0?"); }
                int col = cur_entry % num_cols;
                int row = (num_rows-1) - (cur_entry / num_cols);
                ++cur_entry;

                lpos[0] = this->dataaxisdist + (static_cast<float>(col) * col_advance);
                lpos[1] = this->height + 1.5f*this->fontsize + static_cast<float>(row)*2.0f*this->fontsize;
                // Legend line/marker
                if (this->datastyles[dsi].showlines == true && this->datastyles[dsi].markerstyle != markerstyle::bar) {
                    // draw short line at lpos (rounded ends)
                    morph::vec<float, 3> abit = { 0.5f * toffset[0], 0.0f, 0.0f };
                    this->computeFlatLineRnd (lpos - abit, lpos + abit,
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
                this->texts.push_back (std::move(legtexts[dsi]));
            }
        }

        //! Add the axis labels
        void drawAxisLabels()
        {
            // x axis label (easy)
            morph::TextFeatures tf(this->fontsize, this->fontres, false, morph::colour::black, this->font);
            auto lbl = this->makeVisualTextModel (tf);
            morph::TextGeometry geom = lbl->getTextGeometry (this->xlabel);
            morph::vec<float> lblpos;
            if (this->axisstyle == axisstyle::cross) {
                float _y0_mdl = this->ord1_scale.transform_one (0);
                lblpos = {{0.9f * this->width,
                           _y0_mdl-(this->axislabelgap+geom.height()+this->ticklabelgap+this->xtick_label_height), 0 }};
            } else {
                lblpos = {{0.5f * this->width - geom.half_width(),
                           -(this->axislabelgap+this->ticklabelgap+geom.height()+this->xtick_label_height), 0}};
            }
            lbl->setupText (this->xlabel, lblpos+this->mv_offset, this->axiscolour);
            this->texts.push_back (std::move(lbl));

            // y axis label (have to rotate)
            auto lbl2 = this->makeVisualTextModel (tf);
            geom = lbl2->getTextGeometry (this->ylabel);

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
                morph::quaternion<float> leftrot(this->uz, morph::mathconst<float>::pi_over_2);
                lbl2->setupText (this->ylabel, leftrot, lblpos+this->mv_offset, this->axiscolour);
            } else {
                lbl2->setupText (this->ylabel, lblpos+this->mv_offset, this->axiscolour);
            }
            this->texts.push_back (std::move(lbl2));

            if (this->axisstyle == axisstyle::twinax) {
                // y2 axis label (have to rotate)
                auto lbl3 = this->makeVisualTextModel (tf);
                geom = lbl3->getTextGeometry (this->ylabel2);

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
                    morph::quaternion<float> leftrot(this->uz, morph::mathconst<float>::pi_over_2);
                    lbl3->setupText (this->ylabel2, leftrot, lblpos+this->mv_offset, this->axiscolour);
                } else {
                    lbl3->setupText (this->ylabel2, lblpos+this->mv_offset, this->axiscolour);
                }
                this->texts.push_back (std::move(lbl3));
            }
        }

        static constexpr float max_label_prop = 0.9f;

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

            morph::TextFeatures tf(this->fontsize, this->fontres, false, morph::colour::black, this->font);

            if (!this->omit_x_tick_labels) {

                // Pre-test the xtick labels to see if the length of the labels would make the text
                // too crowded. If so, reduce font size. The combined length of the longest tick
                // label should be less than the proportion 'max_label_prop' of the xtick
                // spacing. Note that this comes AFTER the logic in maketicks() which could in
                // principle be changed to reduce the number of ticks when the number of ticks
                // combined with the font size and tick label string length might cause problems.
                float x_font_factor = 1.0f;
                float max_label_length = 0.0f;
                float xtick_spacing = this->width;
                {
                    if (this->xtick_posns.size() >= 2) { xtick_spacing = this->xtick_posns[1] - this->xtick_posns[0]; }
                    // Create a temporary VisualTextModel to find the length of all the tick text
                    auto lbl = this->makeVisualTextModel (tf);
                    // Find longest string (more or less)
                    for (unsigned int i = 0; i < this->xtick_posns.size(); ++i) {
                        std::string s = morph::graphing::number_format (this->xticks[i], this->xticks[i==0 ? 1 : i-1]);
                        morph::TextGeometry geom = lbl->getTextGeometry (s);
                        max_label_length = geom.width() > max_label_length ? geom.width() : max_label_length;
                    }
                }

                if (max_label_length > max_label_prop * xtick_spacing) {
                    // Labels are too long, compute the adjustment factor
                    x_font_factor = (xtick_spacing * max_label_prop) / max_label_length;
                }

                for (unsigned int i = 0; i < this->xtick_posns.size(); ++i) {

                    // Omit the 0 for 'cross' axes (or maybe shift its position)
                    if (this->axisstyle == axisstyle::cross && this->xticks[i] == 0) { continue; }

                    std::string s = morph::graphing::number_format (this->xticks[i], this->xticks[i==0 ? 1 : i-1]);
                    // Issue: I need the width of the text ss.str() before I can create the
                    // VisualTextModel, so need a static method like this:
                    tf.fontsize = x_font_factor * this->fontsize;
                    auto lbl = this->makeVisualTextModel (tf);
                    tf.fontsize = this->fontsize; // reset
                    morph::TextGeometry geom = lbl->getTextGeometry (s);
                    this->xtick_label_height = geom.height() > this->xtick_label_height ? geom.height() : this->xtick_label_height;
                    morph::vec<float> lblpos = {(float)this->xtick_posns[i]-geom.half_width(), y_for_xticks-(this->ticklabelgap+geom.height()), 0};
                    lbl->setupText (s, lblpos+this->mv_offset, this->axiscolour);
                    this->texts.push_back (std::move(lbl));
                }
            }
            if (!this->omit_y_tick_labels) {
                for (unsigned int i = 0; i < this->ytick_posns.size(); ++i) {

                    // Omit the 0 for 'cross' axes (or maybe shift its position)
                    if (this->axisstyle == axisstyle::cross && this->yticks[i] == 0) { continue; }

                    std::string s = morph::graphing::number_format (this->yticks[i], this->yticks[i==0 ? 1 : i-1]);
                    auto lbl = this->makeVisualTextModel (tf);
                    morph::TextGeometry geom = lbl->getTextGeometry (s);
                    this->ytick_label_width = geom.width() > this->ytick_label_width ? geom.width() : this->ytick_label_width;
                    morph::vec<float> lblpos = {x_for_yticks-this->ticklabelgap-geom.width(), (float)this->ytick_posns[i]-geom.half_height(), 0};
                    std::array<float, 3> clr = this->axiscolour;
                    if (this->axisstyle == axisstyle::twinax && this->datastyles.size() > 0) {
                        clr = this->datastyles[0].policy == stylepolicy::lines ? this->datastyles[0].linecolour : this->datastyles[0].markercolour;
                    }
                    lbl->setupText (s, lblpos+this->mv_offset, clr);
                    this->texts.push_back (std::move(lbl));
                }
            }
            if ((this->axisstyle == axisstyle::twinax || !this->ytick_posns2.empty()) && !this->omit_y_tick_labels) {
                x_for_yticks = this->width;
                this->ytick_label_width2 = 0.0f;
                for (unsigned int i = 0; i < this->ytick_posns2.size(); ++i) {
                    std::string s = morph::graphing::number_format (this->yticks2[i], this->yticks2[i==0 ? 1 : i-1]);
                    auto lbl = this->makeVisualTextModel (tf);
                    morph::TextGeometry geom = lbl->getTextGeometry (s);
                    this->ytick_label_width2 = geom.width() > this->ytick_label_width2 ? geom.width() : this->ytick_label_width2;
                    morph::vec<float> lblpos = {x_for_yticks+this->ticklabelgap, (float)this->ytick_posns2[i]-geom.half_height(), 0};
                    std::array<float, 3> clr = this->axiscolour;
                    if (this->datastyles.size() > 1) {
                        clr = this->datastyles[1].policy == stylepolicy::lines ? this->datastyles[1].linecolour : this->datastyles[1].markercolour;

                    }
                    lbl->setupText (s, lblpos+this->mv_offset, clr);
                    this->texts.push_back (std::move(lbl));
                }
            }
        }

        void drawCrossAxes()
        {
            // Vert zero is not at model(0,0), have to get model coords of data(0,0)
            float _x0_mdl = this->abscissa_scale.transform_one (0);
            float _y0_mdl = this->ord1_scale.transform_one (0);
            this->computeFlatLine ({_x0_mdl, -(this->axislinewidth*0.5f),             -this->thickness},
                                   {_x0_mdl, this->height+(this->axislinewidth*0.5f), -this->thickness},
                                   this->uz, this->axiscolour, this->axislinewidth*0.7f);
            // Horz zero
            this->computeFlatLine ({0,           _y0_mdl, -this->thickness},
                                   {this->width, _y0_mdl, -this->thickness},
                                   this->uz, this->axiscolour, this->axislinewidth*0.7f);

            for (auto xt : this->xtick_posns) {
                // Want to place lines in screen units. So transform the data units
                this->computeFlatLine ({(float)xt, _y0_mdl,                      -this->thickness},
                                       {(float)xt, _y0_mdl - this->ticklength,   -this->thickness}, this->uz,
                                       this->axiscolour, this->axislinewidth*0.5f);
            }
            for (auto yt : this->ytick_posns) {
                this->computeFlatLine ({_x0_mdl,                    (float)yt, -this->thickness},
                                       {_x0_mdl - this->ticklength, (float)yt, -this->thickness}, this->uz,
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
                this->computeFlatLine ({0, -(this->axislinewidth*0.5f),             -this->thickness},
                                       {0, this->height + this->axislinewidth*0.5f, -this->thickness},
                                       this->uz, this->axiscolour, this->axislinewidth);
                // x axis
                this->computeFlatLine ({0,           0, -this->thickness},
                                       {this->width, 0, -this->thickness},
                                       this->uz, this->axiscolour, this->axislinewidth);

                // Draw left and bottom ticks
                float tl = -this->ticklength;
                if (this->tickstyle == tickstyle::ticksin) { tl = this->ticklength; }

                for (auto xt : this->xtick_posns) {
                    // Want to place lines in screen units. So transform the data units
                    this->computeFlatLine ({(float)xt, 0.0f, -this->thickness},
                                           {(float)xt, tl,   -this->thickness}, this->uz,
                                           this->axiscolour, this->axislinewidth*0.5f);
                }
                for (auto yt : this->ytick_posns) {
                    this->computeFlatLine ({0.0f, (float)yt, -this->thickness},
                                           {tl,   (float)yt, -this->thickness}, this->uz,
                                           this->axiscolour, this->axislinewidth*0.5f);
                }

            }

            if (this->axisstyle == axisstyle::box
                || this->axisstyle == axisstyle::twinax
                || this->axisstyle == axisstyle::boxfullticks
                || this->axisstyle == axisstyle::boxcross) {
                // right axis
                this->computeFlatLine ({this->width, -this->axislinewidth*0.5f,               -this->thickness},
                                       {this->width, this->height+(this->axislinewidth*0.5f), -this->thickness},
                                       this->uz, this->axiscolour, this->axislinewidth);
                // top axis
                this->computeFlatLine ({0,           this->height, -this->thickness},
                                       {this->width, this->height, -this->thickness},
                                       this->uz, this->axiscolour, this->axislinewidth);

                float tl = this->ticklength;
                if (this->tickstyle == tickstyle::ticksin) {
                    tl = -this->ticklength;
                }
                // Draw top and right ticks if necessary
                if (this->axisstyle == axisstyle::boxfullticks) {
                    // Tick positions
                    for (auto xt : this->xtick_posns) {
                        // Want to place lines in screen units. So transform the data units
                        this->computeFlatLine ({(float)xt, this->height,      -this->thickness},
                                               {(float)xt, this->height + tl, -this->thickness}, this->uz,
                                               this->axiscolour, this->axislinewidth*0.5f);
                    }
                    for (auto yt : this->ytick_posns) {
                        this->computeFlatLine ({this->width,      (float)yt, -this->thickness},
                                               {this->width + tl, (float)yt, -this->thickness}, this->uz,
                                               this->axiscolour, this->axislinewidth*0.5f);
                    }
                } else if (this->axisstyle == axisstyle::twinax || !this->ytick_posns2.empty()) {
                    // Draw ticks for y2
                    for (auto yt : this->ytick_posns2) {
                        this->computeFlatLine ({this->width,      (float)yt, -this->thickness},
                                               {this->width + tl, (float)yt, -this->thickness}, this->uz,
                                               this->axiscolour, this->axislinewidth*0.5f);
                    }
                }

                if (this->axisstyle == axisstyle::boxcross) { this->drawCrossAxes(); }
            }
        }

        //! Draw a single quiver at point coords_i with direction/magnitude quiv.
        void quiver (morph::vec<float>& coords_i, morph::vec<Flt, 3>& quiv,
                     const Flt lengthcolour, const morph::DatasetStyle& style)
        {
            morph::vec<Flt> halfquiv, half = { Flt{0.5}, Flt{0.5}, Flt{0.5} };
            morph::vec<float> start, end;

            Flt dlength = quiv.length();
            if ((std::isnan(dlength) || dlength == Flt{0})
                && style.quiver_flagset.test(morph::quiver_flags::show_zeros) == true) {
                // NaNs denote zero vectors when the lengths have been log scaled.
                this->computeSphere (coords_i, style.quiver_zero_colour,
                                     style.markersize * style.quiver_thickness_gain);
            } else { // Not a zero marker, draw a quiver

                if (style.markerstyle == morph::markerstyle::quiver_fromcoord) {
                    start = coords_i;
                    std::transform (coords_i.begin(), coords_i.end(), quiv.begin(), end.begin(), std::plus<Flt>());
                } else if (style.markerstyle == morph::markerstyle::quiver_tocoord) {
                    std::transform (coords_i.begin(), coords_i.end(), quiv.begin(), start.begin(), std::minus<Flt>());
                    end = coords_i;
                } else /* default is on-coord */ {
                    std::transform (half.begin(), half.end(), quiv.begin(), halfquiv.begin(), std::multiplies<Flt>());
                    std::transform (coords_i.begin(), coords_i.end(), halfquiv.begin(), start.begin(), std::minus<Flt>());
                    std::transform (coords_i.begin(), coords_i.end(), halfquiv.begin(), end.begin(), std::plus<Flt>());
                }

                // Quiver thickness is either the linewidth (* user-supplied thickness_gain) or a
                // tenth of the length (* thickness_gain)
                float quiv_thick = style.quiver_flagset.test(morph::quiver_flags::thickness_fixed)
                ? style.linewidth * style.quiver_thickness_gain : quiv.length() * 0.1f * style.quiver_thickness_gain;

                // The right way to draw an arrow.
                morph::vec<float> arrow_line = end - start;
                morph::vec<float> cone_start = arrow_line.shorten (quiv.length() * style.quiver_arrowhead_prop);
                cone_start += start;
                constexpr int shapesides = 12;
                std::array<float, 3> clr = style.quiver_colourmap.convert (lengthcolour);
                this->computeTube (start, cone_start, clr, clr, quiv_thick, shapesides);
                float conelen = (end-cone_start).length();
                if (arrow_line.length() > conelen) {
                    this->computeCone (cone_start, end, 0.0f, clr, quiv_thick * style.quiver_conewidth, shapesides);
                }

                if (style.quiver_flagset.test(morph::quiver_flags::marker_sphere) == true) {
                    // Draw a sphere on the coordinate:
                    this->computeSphere (coords_i, clr, quiv_thick * style.quiver_conewidth, shapesides/2, shapesides);
                }
            }
        }

        //! Generate vertices for a bar of a bar graph, with p1 and p2 defining the top
        //! left and right corners of the bar. bottom left and right assumed to be on x
        //! axis.
        void bar (morph::vec<float>& p, const morph::DatasetStyle& style)
        {
            // To update the z position of the data, must also add z thickness to p[2]
            p[2] += thickness;

            morph::vec<float> p1 = p;
            p1[0] -= style.markersize/2.0f;
            morph::vec<float> p2 = p;
            p2[0] += style.markersize/2.0f;

            // Zero is at (height*dataaxisdist)
            morph::vec<float> p1b = p1;
            p1b[1] = this->height * this->dataaxisdist;
            morph::vec<float> p2b = p2;
            p2b[1] = this->height * this->dataaxisdist;

            this->computeFlatQuad (p1b, p1, p2, p2b, style.markercolour);

            if (style.showlines == true) {
                p1b[2] += this->thickness/2.0f;
                p1[2] += this->thickness/2.0f;
                p2[2] += this->thickness/2.0f;
                p2b[2] += this->thickness/2.0f;
                this->computeFlatLineRnd (p1b, p1,  this->uz, style.linecolour, style.linewidth, 0.0f, false, true);
                this->computeFlatLineRnd (p1,  p2,  this->uz, style.linecolour, style.linewidth, 0.0f, true, true);
                this->computeFlatLineRnd (p2,  p2b, this->uz, style.linecolour, style.linewidth, 0.0f, true, false);
            }
        }

        //! Special code to draw a marker representing a bargraph bar for the legend
        void bar_symbol (morph::vec<float>& p, const morph::DatasetStyle& style)
        {
            p[2] += this->thickness;

            morph::vec<float> p1 = p;
            p1[0] -= 0.035f; // Note fixed size for legend
            morph::vec<float> p2 = p;
            p2[0] += 0.035f;

            // Zero is at (height*dataaxisdist)
            morph::vec<float> p1b = p1;
            p1b[1] -= 0.03f;
            morph::vec<float> p2b = p2;
            p2b[1] -= 0.03f;

            float outline_width = 0.005f; // also fixed

            this->computeFlatQuad (p1b, p1, p2, p2b, style.markercolour);

            if (style.showlines == true) {
                p1b[2] += this->thickness;
                p1[2] += this->thickness;
                p2[2] += this->thickness;
                p2b[2] += this->thickness;
                this->computeFlatLineRnd (p1b, p1,  this->uz, style.linecolour, outline_width, 0.0f, true, true);
                this->computeFlatLineRnd (p1,  p2,  this->uz, style.linecolour, outline_width, 0.0f, true, true);
                this->computeFlatLineRnd (p2,  p2b, this->uz, style.linecolour, outline_width, 0.0f, true, true);
                this->computeFlatLineRnd (p2b, p1b, this->uz, style.linecolour, outline_width, 0.0f, true, true);
            }
        }

        //! Generate vertices for a marker of the given style at location p
        void marker (morph::vec<float>& p, const morph::DatasetStyle& style)
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

        // Create an n sided polygon with first vertex 'pointing up'
        void polygonMarker  (morph::vec<float> p, int n, const morph::DatasetStyle& style)
        {
            p[2] += this->thickness;
            this->computeFlatPoly (p, this->ux, this->uy,
                                   style.markercolour,
                                   style.markersize*Flt{0.5}, n);
        }

        // Create an n sided polygon with a flat edge 'pointing up'
        void polygonFlattop (morph::vec<float> p, int n, const morph::DatasetStyle& style)
        {
            p[2] += this->thickness;
            this->computeFlatPoly (p, this->ux, this->uy,
                                   style.markercolour,
                                   style.markersize*Flt{0.5}, n, morph::mathconst<float>::pi/static_cast<float>(n));
        }

        // Given the data, compute the ticks (or use the ones that client code gave us)
        void computeTickPositions()
        {
            if (this->manualticks == true) {
                std::cout << "Writeme: Implement a manual tick-setting scheme\n";
            } else {
                if (this->ord2_scale.ready()) {
                    if (!this->abscissa_scale.ready()) {
                        throw std::runtime_error ("GraphVisual::computeTickPositions: abscissa scale is not set (though ord2 scale is set). Is there abscissa (x) data?");
                    }
                } else if (!(this->abscissa_scale.ready() && this->ord1_scale.ready())) {
                    throw std::runtime_error ("GraphVisual::computeTickPositions: abscissa and ordinate scales not set. Is there data?");
                }
                // Compute locations for ticks...
                Flt _xmin = this->abscissa_scale.inverse_one (this->abscissa_scale.output_range.min);
                Flt _xmax = this->abscissa_scale.inverse_one (this->abscissa_scale.output_range.max);
                Flt _ymin = Flt{0};
                Flt _ymax = Flt{1};
                if (this->ord1_scale.ready()) {
                    _ymin = this->ord1_scale.inverse_one (this->ord1_scale.output_range.min);
                    _ymax = this->ord1_scale.inverse_one (this->ord1_scale.output_range.max);
                }
                Flt _ymin2 = Flt{0};
                Flt _ymax2 = Flt{1};
                if (this->ord2_scale.ready()) {
                    _ymin2 = this->ord2_scale.inverse_one (this->ord2_scale.output_range.min);
                    _ymax2 = this->ord2_scale.inverse_one (this->ord2_scale.output_range.max);
                }
                if constexpr (gv_debug) {
                    std::cout << "x ticks between " << _xmin << " and " << _xmax << " in data units\n";
                    std::cout << "y ticks between " << _ymin << " and " << _ymax << " in data units\n";
                }

                float realmin = this->abscissa_scale.inverse_one (0);
                float realmax = this->abscissa_scale.inverse_one (this->width);
                this->xticks = morph::graphing::maketicks (_xmin, _xmax, realmin, realmax, this->num_ticks_range_x);
                this->xtick_posns.resize (this->xticks.size());
                this->abscissa_scale.transform (xticks, xtick_posns);

                if (this->ord1_scale.ready()) {
                    realmin = this->ord1_scale.inverse_one (0);
                    realmax = this->ord1_scale.inverse_one (this->height);
                    this->yticks = morph::graphing::maketicks (_ymin, _ymax, realmin, realmax, this->num_ticks_range_y);
                    this->ytick_posns.resize (this->yticks.size());
                    this->ord1_scale.transform (yticks, ytick_posns);
                }

                if (this->ord2_scale.ready()) {
                    realmin = this->ord2_scale.inverse_one (0);
                    realmax = this->ord2_scale.inverse_one (this->height);
                    this->yticks2 = morph::graphing::maketicks (_ymin2, _ymax2, realmin, realmax, this->num_ticks_range_y2);
                    this->ytick_posns2.resize (this->yticks2.size());
                    this->ord2_scale.transform (yticks2, ytick_posns2);
                }
            }
        }

    public:
        /*!
         * Graph data coordinates. A vector of vectors of unique pointers to data, with
         * one pointer for each graph in the model.
         *
         * The current scheme for GraphVisual is that this structure holds the data that
         * is displayed in the GraphVisual. These coords are scaled into
         * 'morph::VisualModel space' by abscissa_scale, and either of ord1_scale or
         * ord2_scale.
         *
         * The upshot of this is that abscissa_scale, etc have to be set up to correctly
         * scale from the data units into model coords. These scalings scale from the
         * datarange_x/y/y2 to the model space which is the graph width and height.
         *
         * Any time you change the graph width or height (setsize/resetsize), or update
         * the data, you might have to re-compute graphDataCoords.
         *
         * For this reason, there are points in the usual set up of a GraphVisual where
         * you cannot setlimits - you cannot simply call setlimits anytime before you
         * call GraphVisual::finalize(). For example, if you setdata (x_data, y_data)
         * then in order to populated graphDataCoords, the abscissa_scale and ord1_scale
         * are determined. If you subsequently call setlimits_x, the graphDataCoords
         * would need to be re-computed based on the new x limits. Instead of doing
         * this, there are runtime exceptions that guide you to call setlimits_x before
         * setdata.
         */
        std::vector<std::unique_ptr<std::vector<vec<float>>>> graphDataCoords;
        //! Quiver data, if used. Limitation: You can ONLY have ONE quiver field per
        //! GraphVisual. Note that the quivers can point in three dimensions. That's intentional,
        //! even though 2D quivers are going to be used most. The locations for the quivers for
        //! dataset i are stored in graphDataCoords, like normal points in a non-quiver graph.
        morph::vvec<morph::vec<Flt, 3>> quivers;
        //! The input vectors are scaled in length to the range [0, 1], which is then modified by the
        //! user using quiver_length_gain. This scaling can be made logarithmic by calling
        //! GraphVisual::quiver_setlog() before calling finalize(). The scaling can be ignored by calling
        //! GraphVisual::quiver_length_scale.compute_scaling (0, 1); before finalize().
        morph::scale<float> quiver_length_scale;
        //! Linear scaling for any quivers, which is independent from the length scaling and can be used for colours
        morph::scale<float> quiver_linear_scale;
        morph::scale<float> quiver_colour_scale;
        //! The dx from the morph::Grid, but scaled with abscissa_scale and ord1_scale to be in 'VisualModel units'
        morph::vec<Flt, 3> quiver_grid_spacing;
        //! A scaling for the abscissa.
        morph::scale<Flt> abscissa_scale;
        //! A scaling for the first (left hand) ordinate
        morph::scale<Flt> ord1_scale;
        //! A scaling for the second (right hand) ordinate, if it's a twin axis graph
        morph::scale<Flt> ord2_scale;
        //! A copy of the abscissa data values for ord1
        morph::vvec<Flt> absc1;
        //! A copy of the abscissa data values for ord2
        morph::vvec<Flt> absc2;
        //! A copy of the first (left hand) ordinate data values
        morph::vvec<Flt> ord1;
        //! A copy of the second (right hand) ordinate data values
        morph::vvec<Flt> ord2;
        //! What's the scaling policy for the abscissa?
        morph::scalingpolicy scalingpolicy_x = morph::scalingpolicy::autoscale;
        //! What's the scaling policy for the ordinate?
        morph::scalingpolicy scalingpolicy_y = morph::scalingpolicy::autoscale;
        //! If required, the abscissa's minimum/max data values
        morph::range<Flt> datarange_x{ Flt{0}, Flt{1} };
        //! If required, the ordinate's minimum/max data values
        morph::range<Flt> datarange_y{ Flt{0}, Flt{1} };
        //! If required, the second ordinate's minimum/max data values (twinax)
        morph::range<Flt> datarange_y2{ Flt{0}, Flt{1} };
        //! Auto-rescale x axis if data goes off the edge of the graph (by setting the out of range data as new boundary)
        bool auto_rescale_x = false;
        //! Auto-rescale y axis if data goes off the edge of the graph (by setting the out of range data as new boundary)
        bool auto_rescale_y = false;
        //! in the update function, it fits the scale with the range of the data (/!\ will scope only on the last datasets per y axis)
        bool auto_rescale_fit = false;
        //! Current DatasetStyle for ord1
        morph::DatasetStyle ds_ord1;
        //! DatasetStyle for ord2
        morph::DatasetStyle ds_ord2;
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
        //! Should the x tick *labels* be omitted?
        bool omit_x_tick_labels = false;
        //! Should the y (and y2) tick *labels* be omitted?
        bool omit_y_tick_labels = false;
        //! The number of tick labels permitted, stored as a morph::range
        morph::range<Flt> num_ticks_range_x{ Flt{5}, Flt{10} };
        morph::range<Flt> num_ticks_range_y{ Flt{5}, Flt{10} };
        morph::range<Flt> num_ticks_range_y2{ Flt{5}, Flt{10} };
        // Default font
        morph::VisualFont font = morph::VisualFont::DVSans;
        //! Font resolution - determines how textures for glyphs are generated. If your
        //! labels will be small, this should be smaller. If labels are large, then it
        //! should be increased.
        int fontres = 24;
        //! The font size is the width of an m in the chosen font, in model units
        float fontsize = 0.05f;
        //! A separate fontsize for the axis labels, incase these should be different from the tick labels
        float axislabelfontsize = fontsize;
        // might need tickfontsize and axisfontsize
        //! If this is true, then draw data lines even where they extend beyond the axes.
        bool draw_beyond_axes = false;
        //! EITHER Gap from the y axis to the right hand of the y axis tick label text
        //! quads OR from the x axis to the top of the x axis tick label text quads
        float ticklabelgap = 0.05f;
        //! The gap from the left side of the y tick labels to the right side of the
        //! axis label (or similar for the x axis label)
        float axislabelgap = 0.05f;
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
        float relative_thickness = 0.002f;  // reference thickness relative to width to compute the absolute thickness
        float thickness = relative_thickness;

        //! width is how wide the graph axes will be, in 3D model coordinates
        float width = 1.0f;
        //! height is how high the graph axes will be, in 3D model coordinates
        float height = 1.0f;
        //! What proportion of the graph width/height should be allowed as a space
        //! between the min/max and the axes? Can be 0.0f;
        float dataaxisdist = 0.04f;

        //! Temporary storage for the max height of the xtick labels
        float xtick_label_height = 0.0f;
        //! Temporary storage for the max width of the ytick labels
        float ytick_label_width = 0.0f;
        float ytick_label_width2 = 0.0f;
    };

} // namespace morph
