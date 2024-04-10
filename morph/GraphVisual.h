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
#include <morph/gl/version.h>
#include <morph/tools.h>
#include <morph/Scale.h>
#include <morph/range.h>
#include <morph/vec.h>
#include <morph/VisualTextModel.h>
#include <morph/Quaternion.h>
#include <morph/ColourMap.h>
#include <morph/colour.h>
#include <morph/histo.h>
#include <morph/mathconst.h>
#include <morph/Grid.h>
#include <morph/DatasetStyle.h>
#include <morph/graphstyles.h>
#include <iostream>
#include <vector>
#include <deque>
#include <array>
#include <cmath>
#include <sstream>
#include <memory>

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

        ~GraphVisual() { for (auto& gdc : this->graphDataCoords) { delete gdc; } }

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
                o = this->ord1_scale.transform_one (_ordinate);
            } else {
                this->ord2.push_back (_ordinate);
                this->absc2.push_back (_abscissa);
                o = this->ord2_scale.transform_one (_ordinate);
            }
            Flt a = this->abscissa_scale.transform_one (_abscissa);
            //std::cout << "transformed coords: " << a << ", " << o << std::endl;
            // Now sd and ad can be used to construct dataCoords x/y. They are used to
            // set the position of each datum into dataCoords
            unsigned int oldsz = this->graphDataCoords[didx]->size();
            (this->graphDataCoords[didx])->resize (oldsz+1);
            (*this->graphDataCoords[didx])[oldsz][0] = a;
            (*this->graphDataCoords[didx])[oldsz][1] = o;
            (*this->graphDataCoords[didx])[oldsz][2] = Flt{0};

            if (!this->within_axes_x ((*this->graphDataCoords[didx])[oldsz]) && this->auto_rescale_x) {
                std::cout << "RESCALE x!\n";
                this->clear_graph_data();
                this->graphDataCoords.clear();
                this->pendingAppended = true; // as the graph will be re-drawn
                this->ord1_scale.reset();
                this->ord2_scale.reset();
                this->setlimits_x (this->datamin_x, this->datamax_x*2.0f);
                if (!this->ord1.empty()) {
                    // vvec, vvec, datasetstyle
                    this->setdata (this->absc1, this->ord1, this->ds_ord1);
                }
                if (!this->ord2.empty()) {
                    this->setdata (this->absc2, this->ord2, this->ds_ord2);
                }
                VisualModel<glver>::clear(); // Get rid of the vertices.
                this->initializeVertices(); // Re-build
            }

            if (!this->within_axes_y ((*this->graphDataCoords[didx])[oldsz]) && this->auto_rescale_y) {
                std::cout << "RESCALE y!\n";
            }
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

            if (_abscissae.size() != dsize) {
                throw std::runtime_error ("update: size mismatch");
            }

            if (data_idx >= this->graphDataCoords.size()) {
                std::cout << "Can't add data at graphDataCoords index " << data_idx << std::endl;
                return;
            }

            // Ensure the vector at data_idx has enough capacity for the updated data
            this->graphDataCoords[data_idx]->resize (dsize);

            // May need a re-autoscaling option somewhere in here.

            // Transfor the data into temporary containers sd and ad
            std::vector<Flt> ad (dsize, Flt{0});
            this->abscissa_scale.transform (_abscissae, ad);


            std::vector<Flt> sd (dsize, Flt{0});
            this->ord1_scale.transform (_data, sd);

            // Now sd and ad can be used to construct dataCoords x/y. They are used to
            // set the position of each datum into dataCoords
            for (unsigned int i = 0; i < dsize; ++i) {
                (*this->graphDataCoords[data_idx])[i][0] = static_cast<Flt>(ad[i]);
                (*this->graphDataCoords[data_idx])[i][1] = static_cast<Flt>(sd[i]);
                (*this->graphDataCoords[data_idx])[i][2] = Flt{0};
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
            unsigned int data_index = this->graphDataCoords.size();
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
            // _quivs should have same size as g.n
            if (_quivs.size() != g.n) {
                std::stringstream ee;
                ee << "Size mismatch. Grid has " << g.n << " elements but there are "
                   << _quivs.size() << " quivers";
                throw std::runtime_error (ee.str());
            }

            if (ds.markerstyle != morph::markerstyle::quiver
                && ds.markerstyle != morph::markerstyle::quiver_fromcoord
                && ds.markerstyle != morph::markerstyle::quiver_tocoord) {
                throw std::runtime_error ("markerstyle must be morph::markerstyle::quiver(_fromcoord/_tocoord)"
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

            // From g.v_x and g.v_y we get coordinates. These have to be copied into graphDataCoords
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
            this->graphDataCoords.push_back (new std::vector<morph::vec<float>>(dsize, { 0.0f, 0.0f, 0.0f }));
            this->datastyles.push_back (ds);

            // Compute the ord1_scale and asbcissa_scale for the first added dataset only
            if (ds.axisside == morph::axisside::left) {
                if (this->ord1_scale.ready() == false) { this->compute_scaling (_abscissae, _data, ds.axisside); }
            } else {
                if (this->ord2_scale.ready() == false) { this->compute_scaling (_abscissae, _data, ds.axisside); }
            }

            if (dsize > 0) {
                // Transform the coordinate data into temporary containers
                std::vector<Flt> ad (g.n, Flt{0});
                std::vector<Flt> sd (g.n, Flt{0});

                auto _dx = g.get_dx();
                if (ds.axisside == morph::axisside::left) {
                    this->ord1_scale.transform (g.v_y, sd);
                    this->quiver_grid_spacing[1] = _dx[1] * this->ord1_scale.getParams(0);
                } else {
                    this->ord2_scale.transform (g.v_y, sd);
                    this->quiver_grid_spacing[1] = _dx[1] * this->ord2_scale.getParams(0);
                }
                this->abscissa_scale.transform (g.v_x, ad);
                this->quiver_grid_spacing[0] = _dx[0] * this->abscissa_scale.getParams(0);

                // Now sd and ad can be used to construct dataCoords x/y. They are used to
                // set the position of each datum into dataCoords
                for (unsigned int i = 0; i < dsize; ++i) {
                    (*this->graphDataCoords[didx])[i][0] = static_cast<Flt>(ad[i]);
                    (*this->graphDataCoords[didx])[i][1] = static_cast<Flt>(sd[i]);
                    (*this->graphDataCoords[didx])[i][2] = Flt{0};
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
                ee << "size mismatch. abscissa size " << _abscissae.size() << " and data size: " << _data.size();
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

            unsigned int dsize = _data.size();
            unsigned int didx = this->graphDataCoords.size();

            // Allocate memory for the new data coords, add the data style info and the
            // starting index for dataCoords
#ifdef __ICC__
            morph::vec<float> dummyzero = {{0.0f, 0.0f, 0.0f}};
            this->graphDataCoords.push_back (new std::vector<morph::vec<float>>(dsize, dummyzero));
#else
            this->graphDataCoords.push_back (new std::vector<morph::vec<float>>(dsize, {0,0,0}));
#endif
            this->datastyles.push_back (ds);

            // Compute the ord1_scale and asbcissa_scale for the first added dataset only
            if (ds.axisside == morph::axisside::left) {
                if (this->ord1_scale.ready() == false) { this->compute_scaling (_abscissae, _data, ds.axisside); }
            } else {
                if (this->ord2_scale.ready() == false) { this->compute_scaling (_abscissae, _data, ds.axisside); }
            }

            if (dsize > 0) {
                // Transform the data into temporary containers sd and ad
                std::vector<Flt> ad (dsize, Flt{0});
                std::vector<Flt> sd (dsize, Flt{0});
                if (ds.axisside == morph::axisside::left) {
                    this->ord1_scale.transform (_data, sd);
                } else {
                    this->ord2_scale.transform (_data, sd);
                }
                this->abscissa_scale.transform (_abscissae, ad);

                // Now sd and ad can be used to construct dataCoords x/y. They are used to
                // set the position of each datum into dataCoords
                for (unsigned int i = 0; i < dsize; ++i) {
                    (*this->graphDataCoords[didx])[i][0] = static_cast<Flt>(ad[i]);
                    (*this->graphDataCoords[didx])[i][1] = static_cast<Flt>(sd[i]);
                    (*this->graphDataCoords[didx])[i][2] = Flt{0};
                }
            }
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

            unsigned int data_index = this->graphDataCoords.size();
            ds.markercolour = DatasetStyle::datacolour(data_index);
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
        template <typename Ctnr1, typename Ctnr2>
        std::enable_if_t<morph::is_copyable_container<Ctnr1>::value
                         && morph::is_copyable_container<Ctnr2>::value, void>
        compute_scaling (const Ctnr1& _abscissae, const Ctnr2& _data, const morph::axisside axisside)
        {
            morph::range<Flt> data_maxmin = morph::MathAlgo::maxmin (_data);
            morph::range<Flt> absc_maxmin = morph::MathAlgo::maxmin (_abscissae);
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
                this->abscissa_scale.compute_autoscale (this->datamin_x, absc_maxmin.max);
                break;
            }
            case morph::scalingpolicy::manual_max:
            {
                this->abscissa_scale.compute_autoscale (absc_maxmin.min, this->datamax_x);
                break;
            }
            case morph::scalingpolicy::autoscale:
            default:
            {
                this->abscissa_scale.compute_autoscale (absc_maxmin.min, absc_maxmin.max);
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
                    this->ord1_scale.compute_autoscale (this->datamin_y, data_maxmin.max);
                } else {
                    this->ord2_scale.compute_autoscale (this->datamin_y2, data_maxmin.max);
                }
                break;
            }
            case morph::scalingpolicy::manual_max:
            {
                if (axisside == morph::axisside::left) {
                    this->ord1_scale.compute_autoscale (data_maxmin.min, this->datamax_y);
                } else {
                    this->ord2_scale.compute_autoscale (data_maxmin.min, this->datamax_y2);
                }
                break;
            }
            case morph::scalingpolicy::autoscale:
            default:
            {
                if (axisside == morph::axisside::left) {
                    this->ord1_scale.compute_autoscale (data_maxmin.min, data_maxmin.max);
                } else {
                    this->ord2_scale.compute_autoscale (data_maxmin.min, data_maxmin.max);
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
            if (this->ord1_scale.ready()) {
                throw std::runtime_error ("Have already scaled the data, can't set the dataaxisdist now.\n"
                                          "Hint: call GraphVisual::setdataaxisdist() BEFORE GraphVisual::setdata() or ::setlimits()");
            }
            this->dataaxisdist = proportion;
        }

        //! Set the graph size, in model units.
        void setsize (float _width, float _height)
        {
            if (this->ord1_scale.ready()) {
                throw std::runtime_error ("Have already scaled the data, can't set the scale now.\n"
                                          "Hint: call GraphVisual::setsize() BEFORE GraphVisual::setdata() or ::setlimits()");
            }
            this->width = _width;
            this->height = _height;

            float _extra = this->dataaxisdist * this->height;
            this->ord1_scale.output_range.min = _extra;
            this->ord1_scale.output_range.max = this->height - _extra;
            // Same for ord2_scale:
            this->ord2_scale.output_range.min = _extra;
            this->ord2_scale.output_range.max = this->height - _extra;

            _extra = this->dataaxisdist * this->width;
            this->abscissa_scale.output_range.min = _extra;
            this->abscissa_scale.output_range.max = this->width - _extra;

            this->thickness *= this->width;
        }

        // Make all the bits of the graph - fonts, line thicknesses, etc, bigger by factor. Call before finalize().
        void zoomgraph (Flt factor)
        {
            float _w = this->width;
            float _h = this->height;
            this->setsize (_w*factor, _h*factor);

            this->fontsize *= factor;
            //this->fontres /= factor; // maybe
            this->axislabelfontsize *= factor;

            this->ticklabelgap *= factor;
            this->axislabelgap *= factor;

            this->ticklength *= factor;
            this->axislinewidth *= factor;
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
            if constexpr (gv_debug) {
                std::cout << "initial trytick = " << trytick << ", numticks = " << numticks << " max_num_ticks = " << max_num_ticks << std::endl;
            }
            if (numticks > max_num_ticks) {
                while (numticks > max_num_ticks && numticks > min_num_ticks) {
                    trytick = trytick * 2; // bigger tick spacing means fewer ticks
                    numticks = floor(range/trytick);
                }
            } else {
                while (numticks < min_num_ticks && numticks < max_num_ticks) {
                    trytick = trytick * 0.5;
                    numticks = floor(range/trytick);
                    if constexpr (gv_debug) {
                        std::cout << "Trying reduced spacing to increase numticks. trytick = " << trytick << " and numticks= " << numticks << "\n";
                    }
                }
            }
            if constexpr (gv_debug) {
                std::cout << "Try (data) ticks of size " << trytick << ", which makes for " << numticks << " ticks.\n";
            }
            // Realmax and realmin come from the full range of abscissa_scale/ord1_scale
            Flt midrange = (rmin + rmax) * Flt{0.5};
            Flt a = std::round (midrange / trytick);
            Flt atick = a * trytick;
            while (atick <= realmax) {
                // This tick is smaller than 100th of the size of one whole tick to tick spacing, so it must be 0.
                ticks.push_back (std::abs(atick) < Flt{0.01} * std::abs(trytick) ? Flt{0} : atick);
                atick += trytick;
            }
            atick = (a * trytick) - trytick;
            while (atick >= realmin) {
                ticks.push_back (std::abs(atick) < Flt{0.01} * std::abs(trytick) ? Flt{0} : atick);
                atick -= trytick;
            }

            return ticks;
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
                    unsigned int nquiv = this->quivers.size();
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
                        for (unsigned int i = 0; i < nquiv; ++i) {
                            //raw_qlengths[i] = this->quivers[i].length();
                            userscaled_qlengths[i] = (this->quivers[i] * this->datastyles[dsi].quiver_gain * this->quiver_grid_spacing).length();
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
                        for (unsigned int i = 0; i < nquiv; ++i) {
                            final_quivers[i] *= this->datastyles[dsi].quiver_gain * this->quiver_grid_spacing * lfactor[i];
                            final_qlengths[i] = final_quivers[i].length();
                        }
                        // Replace any zero lengths with the value of the minimum length to ensure colour range goes from min to max not 0 to max
                        Flt fqlmin = final_qlengths.prune_zero().min();
                        final_qlengths.search_replace (Flt{0}, fqlmin);
                        // Then scaled the final_qlengths to range [0-1]
                        morph::vvec<Flt> colour_qlengths (nquiv, Flt{0});
                        this->quiver_colour_scale.transform (final_qlengths, colour_qlengths);

                        // Finally loop thru coords, drawing a quiver for each
                        if (coords_end > nquiv) { throw std::runtime_error ("coords_end is off the end of quivers"); }
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
                            // Draw solid lines between marker points with gaps between line and marker
                            this->computeFlatLine (this->idx, (*this->graphDataCoords[dsi])[i-1], (*this->graphDataCoords[dsi])[i], this->uz,
                                                   this->datastyles[dsi].linecolour,
                                                   this->datastyles[dsi].linewidth, this->datastyles[dsi].markergap);
                        } else if (appending == true) {
                            // We are appending a line to an existing graph, so compute a single line with rounded ends
                            this->computeFlatLineRnd (this->idx,
                                                      (*this->graphDataCoords[dsi])[i-1], // start
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
                                this->computeFlatLine (this->idx,
                                                       (*this->graphDataCoords[dsi])[i-1], // start
                                                       (*this->graphDataCoords[dsi])[i],   // end
                                                       this->uz,
                                                       this->datastyles[dsi].linecolour,
                                                       this->datastyles[dsi].linewidth);
                            } else if (i == 1+coords_start) {
                                // First line
                                this->computeFlatLineN (this->idx,
                                                        (*this->graphDataCoords[dsi])[i-1], // start
                                                        (*this->graphDataCoords[dsi])[i],   // end
                                                        (*this->graphDataCoords[dsi])[i+1], // next
                                                        this->uz,
                                                        this->datastyles[dsi].linecolour,
                                                        this->datastyles[dsi].linewidth);
                            } else if (i == (coords_end-1)) {
                                // last line
                                this->computeFlatLineP (this->idx, (*this->graphDataCoords[dsi])[i-1], (*this->graphDataCoords[dsi])[i],
                                                        (*this->graphDataCoords[dsi])[i-2],
                                                        this->uz,
                                                        this->datastyles[dsi].linecolour,
                                                        this->datastyles[dsi].linewidth);
                            } else {
                                // An intermediate line
                                this->computeFlatLine (this->idx, (*this->graphDataCoords[dsi])[i-1], (*this->graphDataCoords[dsi])[i],
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
                unsigned int coords_end = this->graphDataCoords[dsi]->size();
                this->coords_lengths[dsi] = coords_end;
                this->drawDataCommon (dsi, coords_start, coords_end, appending_data);
            }
        }

        //! Draw all markers and lines for datasets in the graph (as stored in graphDataCoords)
        void drawData()
        {
            unsigned int coords_start = 0;
            this->coords_lengths.resize (this->graphDataCoords.size());
            for (unsigned int dsi = 0; dsi < this->graphDataCoords.size(); ++dsi) {
                unsigned int coords_end = this->graphDataCoords[dsi]->size();
                // Record coords length for future appending:
                this->coords_lengths[dsi] = coords_end;
                this->drawDataCommon (dsi, coords_start, coords_end);
            }
        }

        //! Draw the graph legend, above the graph, rather than inside it (so much simpler!)
        void drawLegend()
        {
            unsigned int num_legends_max = this->graphDataCoords.size();

            // Text offset from marker to text
            morph::vec<float> toffset = {this->fontsize, 0.0f, 0.0f};

            // To determine the legend layout, will need all the text geometries
            std::vector<morph::TextGeometry> geom;
            std::map<unsigned int, std::unique_ptr<morph::VisualTextModel<glver>>> legtexts;

            morph::vvec<unsigned int> ds_indices; // dataset indices.

            float text_advance = 0.0f;
            int num_legends = 0;
            for (unsigned int dsi = 0; dsi < num_legends_max; ++dsi) {
                // If no label, then draw no legend. Thus the effective num_legends may be smaller
                // than num_legends_max.
                if (this->datastyles[dsi].datalabel.empty()) { continue; }
                // Legend text. If all is well, this will be pushed onto the texts attribute and
                // deleted when the model is deconstructed.
                auto ltp = std::make_unique<morph::VisualTextModel<glver>> (this->parentVis, this->get_tprog(this->parentVis), this->font, this->fontsize, this->fontres);
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
                this->texts.push_back (std::move(legtexts[dsi]));
            }
        }

        //! Add the axis labels
        void drawAxisLabels()
        {
            // x axis label (easy)
            auto lbl = std::make_unique<morph::VisualTextModel<glver>> (this->parentVis, this->get_tprog(this->parentVis), this->font, this->fontsize, this->fontres);
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
            auto lbl2 = std::make_unique<morph::VisualTextModel<glver>> (this->parentVis, this->get_tprog(this->parentVis), this->font, this->fontsize, this->fontres);
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
                morph::Quaternion<float> leftrot;
                leftrot.initFromAxisAngle (this->uz, -90.0f);
                lbl2->setupText (this->ylabel, leftrot, lblpos+this->mv_offset, this->axiscolour);
            } else {
                lbl2->setupText (this->ylabel, lblpos+this->mv_offset, this->axiscolour);
            }
            this->texts.push_back (std::move(lbl2));

            if (this->axisstyle == axisstyle::twinax) {
                // y2 axis label (have to rotate)
                auto lbl3 = std::make_unique<morph::VisualTextModel<glver>> (this->parentVis, this->get_tprog(this->parentVis), this->font, this->fontsize, this->fontres);
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
                    morph::Quaternion<float> leftrot;
                    leftrot.initFromAxisAngle (this->uz, -90.0f);
                    lbl3->setupText (this->ylabel2, leftrot, lblpos+this->mv_offset, this->axiscolour);
                } else {
                    lbl3->setupText (this->ylabel2, lblpos+this->mv_offset, this->axiscolour);
                }
                this->texts.push_back (std::move(lbl3));
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

            if (!this->omit_x_tick_labels) {
                for (unsigned int i = 0; i < this->xtick_posns.size(); ++i) {

                    // Omit the 0 for 'cross' axes (or maybe shift its position)
                    if (this->axisstyle == axisstyle::cross && this->xticks[i] == 0) { continue; }

                    // Expunge any '0' from 0.123 so that it's .123 and so on.
                    std::string s = this->graphNumberFormat (this->xticks[i]);

                    // Issue: I need the width of the text ss.str() before I can create the
                    // VisualTextModel, so need a static method like this:
                    auto lbl = std::make_unique<morph::VisualTextModel<glver>> (this->parentVis, this->get_tprog(this->parentVis), this->font, this->fontsize, this->fontres);
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

                    std::string s = this->graphNumberFormat (this->yticks[i]);
                    auto lbl = std::make_unique<morph::VisualTextModel<glver>> (this->parentVis, this->get_tprog(this->parentVis), this->font, this->fontsize, this->fontres);
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
            if (this->axisstyle == axisstyle::twinax && !this->omit_y_tick_labels) {
                x_for_yticks = this->width;
                this->ytick_label_width2 = 0.0f;
                for (unsigned int i = 0; i < this->ytick_posns2.size(); ++i) {
                    std::string s = this->graphNumberFormat (this->yticks2[i]);
                    auto lbl = std::make_unique<morph::VisualTextModel<glver>> (this->parentVis, this->get_tprog(this->parentVis), this->font, this->fontsize, this->fontres);
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
            this->computeFlatLine (this->idx,
                                   {_x0_mdl, -(this->axislinewidth*0.5f),             -this->thickness},
                                   {_x0_mdl, this->height+(this->axislinewidth*0.5f), -this->thickness},
                                   this->uz, this->axiscolour, this->axislinewidth*0.7f);
            // Horz zero
            this->computeFlatLine (this->idx,
                                   {0,           _y0_mdl, -this->thickness},
                                   {this->width, _y0_mdl, -this->thickness},
                                   this->uz, this->axiscolour, this->axislinewidth*0.7f);

            for (auto xt : this->xtick_posns) {
                // Want to place lines in screen units. So transform the data units
                this->computeFlatLine (this->idx,
                                       {(float)xt, _y0_mdl,                      -this->thickness},
                                       {(float)xt, _y0_mdl - this->ticklength,   -this->thickness}, this->uz,
                                       this->axiscolour, this->axislinewidth*0.5f);
            }
            for (auto yt : this->ytick_posns) {
                this->computeFlatLine (this->idx,
                                       {_x0_mdl,                    (float)yt, -this->thickness},
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
                this->computeFlatLine (this->idx,
                                       {0, -(this->axislinewidth*0.5f),             -this->thickness},
                                       {0, this->height + this->axislinewidth*0.5f, -this->thickness},
                                       this->uz, this->axiscolour, this->axislinewidth);
                // x axis
                this->computeFlatLine (this->idx,
                                       {0,           0, -this->thickness},
                                       {this->width, 0, -this->thickness},
                                       this->uz, this->axiscolour, this->axislinewidth);

                // Draw left and bottom ticks
                float tl = -this->ticklength;
                if (this->tickstyle == tickstyle::ticksin) { tl = this->ticklength; }

                for (auto xt : this->xtick_posns) {
                    // Want to place lines in screen units. So transform the data units
                    this->computeFlatLine (this->idx,
                                           {(float)xt, 0.0f, -this->thickness},
                                           {(float)xt, tl,   -this->thickness}, this->uz,
                                           this->axiscolour, this->axislinewidth*0.5f);
                }
                for (auto yt : this->ytick_posns) {
                    this->computeFlatLine (this->idx,
                                           {0.0f, (float)yt, -this->thickness},
                                           {tl,   (float)yt, -this->thickness}, this->uz,
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
                                       this->uz, this->axiscolour, this->axislinewidth);
                // top axis
                this->computeFlatLine (this->idx,
                                       {0,           this->height, -this->thickness},
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
                        this->computeFlatLine (this->idx,
                                               {(float)xt, this->height,      -this->thickness},
                                               {(float)xt, this->height + tl, -this->thickness}, this->uz,
                                               this->axiscolour, this->axislinewidth*0.5f);
                    }
                    for (auto yt : this->ytick_posns) {
                        this->computeFlatLine (this->idx,
                                               {this->width,      (float)yt, -this->thickness},
                                               {this->width + tl, (float)yt, -this->thickness}, this->uz,
                                               this->axiscolour, this->axislinewidth*0.5f);
                    }
                } else if (this->axisstyle == axisstyle::twinax) {
                    // Draw ticks for y2
                    for (auto yt : this->ytick_posns2) {
                        this->computeFlatLine (this->idx,
                                               {this->width,      (float)yt, -this->thickness},
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
            // To update the z position of the data, must also add z thickness to p[2]
            coords_i[2] += thickness;

            morph::vec<Flt> halfquiv, half = { Flt{0.5}, Flt{0.5}, Flt{0.5} };
            morph::vec<float> start, end;

            Flt dlength = quiv.length();
            if ((std::isnan(dlength) || dlength == Flt{0})
                && style.quiver_flagset.test(static_cast<unsigned int>(morph::quiver_flags::show_zeros)) == true) {
                // NaNs denote zero vectors when the lengths have been log scaled.
                this->computeSphere (this->idx, coords_i, style.quiver_zero_colour,
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

                // How thick to draw the quiver arrows? Can scale by length (default) or keep
                // constant (set fixed_quiver_thickness > 0)
                // float len = nrmlzedlength * style.quiver_gain.length() * style.quiver_length_gain;

                float quiv_thick = style.quiver_flagset.test(static_cast<unsigned int>(morph::quiver_flags::thickness_fixed))
                ? style.linewidth * style.quiver_thickness_gain : quiv.length() * style.quiver_thickness_gain;

                // The right way to draw an arrow.
                morph::vec<float> arrow_line = end - start;
                morph::vec<float> cone_start = arrow_line.shorten (quiv.length() * style.quiver_arrowhead_prop);
                cone_start += start;
                constexpr int shapesides = 12;
                std::array<float, 3> clr = style.quiver_colourmap.convert (lengthcolour);
                this->computeTube (this->idx, start, cone_start, clr, clr, quiv_thick, shapesides);
                float conelen = (end-cone_start).length();
                if (arrow_line.length() > conelen) {
                    this->computeCone (this->idx, cone_start, end, 0.0f, clr, quiv_thick*2.0f, shapesides);
                }

                if (style.quiver_flagset.test(static_cast<unsigned int>(morph::quiver_flags::marker_sphere)) == true) {
                    // Draw a sphere on the coordinate:
                    this->computeSphere (this->idx, coords_i, clr, quiv_thick*2.0f, shapesides/2, shapesides);
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
            this->computeFlatPoly (this->idx, p, this->ux, this->uy,
                                   style.markercolour,
                                   style.markersize*Flt{0.5}, n);
        }

        // Create an n sided polygon with a flat edge 'pointing up'
        void polygonFlattop (morph::vec<float> p, int n, const morph::DatasetStyle& style)
        {
            p[2] += this->thickness;
            this->computeFlatPoly (this->idx, p, this->ux, this->uy,
                                   style.markercolour,
                                   style.markersize*Flt{0.5}, n, morph::mathconst<float>::pi/static_cast<float>(n));
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
                Flt _xmin = this->abscissa_scale.inverse_one (this->abscissa_scale.output_range.min);
                Flt _xmax = this->abscissa_scale.inverse_one (this->abscissa_scale.output_range.max);
                Flt _ymin = this->ord1_scale.inverse_one (this->ord1_scale.output_range.min);
                Flt _ymax = this->ord1_scale.inverse_one (this->ord1_scale.output_range.max);
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
                this->xticks = this->maketicks (_xmin, _xmax, realmin, realmax, this->max_num_ticks, this->min_num_ticks);
                realmin = this->ord1_scale.inverse_one (0);
                realmax = this->ord1_scale.inverse_one (this->height);
                this->yticks = this->maketicks (_ymin, _ymax, realmin, realmax, this->max_num_ticks, this->min_num_ticks);

                if (this->ord2_scale.ready()) {
                    realmin = this->ord2_scale.inverse_one (0);
                    realmax = this->ord2_scale.inverse_one (this->height);
                    this->yticks2 = this->maketicks (_ymin2, _ymax2, realmin, realmax, this->max_num_ticks, this->min_num_ticks);
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
        std::vector<std::vector<vec<float>>*> graphDataCoords;
        //! Quiver data, if used. Limitation: You can ONLY have ONE quiver field per
        //! GraphVisual. Note that the quivers can point in three dimensions. That's intentional,
        //! even though 2D quivers are going to be used most. The locations for the quivers for
        //! dataset i are stored in graphDataCoords, like normal points in a non-quiver graph.
        morph::vvec<morph::vec<Flt, 3>> quivers;
        //! The input vectors are scaled in length to the range [0, 1], which is then modified by the
        //! user using quiver_length_gain. This scaling can be made logarithmic by calling
        //! GraphVisual::quiver_setlog() before calling finalize(). The scaling can be ignored by calling
        //! GraphVisual::quiver_length_scale.compute_autoscale (0, 1); before finalize().
        morph::Scale<float> quiver_length_scale;
        //! Linear scaling for any quivers, which is independent from the length scaling and can be used for colours
        morph::Scale<float> quiver_linear_scale;
        morph::Scale<float> quiver_colour_scale;
        //! The dx from the morph::Grid, but scaled with abscissa_scale and ord1_scale to be in 'VisualModel units'
        morph::vec<Flt, 3> quiver_grid_spacing;
        //! A scaling for the abscissa.
        morph::Scale<Flt> abscissa_scale;
        //! A copy of the abscissa data values for ord1
        morph::vvec<Flt> absc1;
        //! A copy of the abscissa data values for ord2
        morph::vvec<Flt> absc2;
        //! A scaling for the first (left hand) ordinate
        morph::Scale<Flt> ord1_scale;
        //! A copy of the first (left hand) ordinate data values
        morph::vvec<Flt> ord1;
        //! ds_ord1
        morph::DatasetStyle ds_ord1;
        morph::DatasetStyle ds_ord2;
        //! A scaling for the second (right hand) ordinate, if it's a twin axis graph
        morph::Scale<Flt> ord2_scale;
        //! A copy of the second (right hand) ordinate data values
        morph::vvec<Flt> ord2;
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
        //! Auto-rescale x axis if data goes off the edge of the graph (by doubling range?)
        bool auto_rescale_x = false;
        //! Auto-rescale y axis if data goes off the edge of the graph
        bool auto_rescale_y = false;
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
        //! Max number of tick labels permitted
        Flt max_num_ticks = Flt{10};
        //! Min number of tick labels permitted
        Flt min_num_ticks = Flt{3};
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
        //! If this is true, then draw data lines even where they extend beyond the axes.
        bool draw_beyond_axes = false;
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

        //! Temporary storage for the max height of the xtick labels
        float xtick_label_height = 0.0f;
        //! Temporary storage for the max width of the ytick labels
        float ytick_label_width = 0.0f;
        float ytick_label_width2 = 0.0f;
    };

} // namespace morph
