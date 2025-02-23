/*
 * A colour bar visual
 */

#pragma once

#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <morph/scale.h>
#include <morph/vec.h>
#include <morph/GraphVisual.h> // share some features from GraphVisual

namespace morph {

    // Should our colourbar be horizontal or vertical? Horizontal bars always have
    // min->max from left to right. Vertical bars always have min->max from bottom to
    // top. Make your own ColourBarVisual if you need reversed mappings!
    enum class colourbar_orientation
    {
        horizontal,
        vertical
    };

    // Which side for the ticks and tick labels? right_or_below means "on the right for
    // a vertical colourbar or below for a horizontal
    // colourbar". right_or_below_ticksboth, this means plot tick labels right or below,
    // but plot ticks right AND left OR above AND below.4
    enum class colourbar_tickside
    {
        right_or_below,
        left_or_above,
        right_or_below_ticksboth,
        left_or_above_ticksboth
    };

    template <typename F, int glver = morph::gl::version_4_1>
    class ColourBarVisual : public VisualModel<glver>
    {
    public:
        //! Constructor
        //! \param _offset The offset within morph::Visual space to place these axes
        ColourBarVisual (const vec<float> _offset)
        {
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->scale.do_autoscale = true;
            this->tickscale.do_autoscale = true;
            // Set default text features
            this->tf.fontsize = 0.05f;
            this->tf.fontres = 48;
            this->tf.colour = this->framecolour;
            // Like graphs, colourbars don't rotate by default. If you want yours to, set this false in your client code.
            this->twodimensional = true;
        }

        //! Set the colour of the frame, ticks and text
        void setColour (const std::array<float, 3>& c)
        {
            this->tf.colour = c;
            this->framecolour = c;
        }

        void initializeVertices()
        {
            // If client code provided no scale, then show colour bar from 0->1
            if (!this->scale.ready()) { this->tickscale.compute_scaling (0, 1); }

            this->tickscale.output_range.max = this->length;
            this->tickscale.compute_scaling (this->scale.inverse_one (this->scale.output_range.min),
                                             this->scale.inverse_one (this->scale.output_range.max));

            this->computeTickPositions();
            this->drawFrame();
            // Draw ticks, tick labels and axis label
            this->drawTicks();
            this->drawTickLabels();
            this->drawAxisLabel();
            this->fillFrameWithColour();
        }

        //! Based on axis scaling, compute tick positions
        void computeTickPositions()
        {
            if (this->manualticks == true) {
                std::cout << "Writeme: Implement a manual tick-setting scheme\n";
            } else {
                // Compute locations for ticks
                F _min = this->tickscale.inverse_one (this->tickscale.output_range.min);
                F _max = this->tickscale.inverse_one (this->tickscale.output_range.max);
                float realmin = this->tickscale.inverse_one (0);
                float realmax = this->tickscale.inverse_one (this->length);

                if (this->justmaxmin == true) {
                    // This may not work quite right, depending on the range in this->scale.
                    this->ticks = morph::GraphVisual<F, glver>::maketicks (_min, _max, realmin, realmax, 2, 2); // max 2, min 2
                } else {
                    this->ticks = morph::GraphVisual<F, glver>::maketicks (_min, _max, realmin, realmax, 8); // 8 is max num ticks
                }
                this->tick_posns.resize (this->ticks.size());
                this->tickscale.transform (ticks, tick_posns);
            }
        }

        //! Draw the frame around the colourbar
        void drawFrame()
        {
            // Use flat lines for the frame
            morph::vec<float, 2> extents = { width, length };
            if (this->orientation == colourbar_orientation::horizontal) { extents = { length, width }; }

            this->computeFlatLine ({-this->framelinewidth,            -(this->framelinewidth*0.5f), this->z},
                                   {extents.x()+this->framelinewidth, -(this->framelinewidth*0.5f), this->z},
                                   this->uz, this->framecolour, this->framelinewidth);

            this->computeFlatLine ({extents.x()+this->framelinewidth*0.5f, 0.0f,        this->z},
                                   {extents.x()+this->framelinewidth*0.5f, extents.y(), this->z},
                                   this->uz, this->framecolour, this->framelinewidth);

            this->computeFlatLine ({extents.x()+this->framelinewidth, extents.y()+(this->framelinewidth*0.5f), this->z},
                                   {-this->framelinewidth,            extents.y()+(this->framelinewidth*0.5f), this->z},
                                   this->uz, this->framecolour, this->framelinewidth);

            this->computeFlatLine ({-this->framelinewidth*0.5f, extents.y(), this->z},
                                   {-this->framelinewidth*0.5f, 0.0f,        this->z},
                                   this->uz, this->framecolour, this->framelinewidth);
        }

        // Draw the tick marks on each axis
        void drawTicks()
        {
            float tl = this->tickstyle == tickstyle::ticksin ? -this->ticklength : this->ticklength;

            // left ticks or above ticks
            if (this->tickside == colourbar_tickside::left_or_above || this->tickside == colourbar_tickside::right_or_below_ticksboth) {
                // Draw ticks left or above (depending on orientation)
                if (this->orientation == colourbar_orientation::horizontal) {
                    // above
                    for (auto t : this->tick_posns) {
                        this->computeFlatLine ({static_cast<float>(t), width+framelinewidth*0.5f,    this->z},
                                               {static_cast<float>(t), width+framelinewidth*0.5f+tl, this->z}, this->uz,
                                               this->framecolour, this->framelinewidth*0.5f);
                    }
                } else {
                    // left
                    for (auto t : this->tick_posns) {
                        this->computeFlatLine ({-framelinewidth*0.5f,    static_cast<float>(t), this->z},
                                               {-framelinewidth*0.5f-tl, static_cast<float>(t), this->z}, this->uz,
                                               this->framecolour, this->framelinewidth*0.5f);
                    }
                }
            }


            if (this->tickside == colourbar_tickside::right_or_below || this->tickside == colourbar_tickside::left_or_above_ticksboth) {
                // Draw ticks right or below (depending on orientation)
                if (this->orientation == colourbar_orientation::horizontal) {
                    // below
                    for (auto t : this->tick_posns) {
                        this->computeFlatLine ({static_cast<float>(t), -framelinewidth*0.5f,      this->z},
                                               {static_cast<float>(t), -(framelinewidth*0.5f)-tl, this->z}, this->uz,
                                               this->framecolour, this->framelinewidth*0.5f);
                    }
                } else {
                    // right
                    for (auto t : this->tick_posns) {
                        this->computeFlatLine ({width+framelinewidth*0.5f,    static_cast<float>(t), this->z},
                                               {width+framelinewidth*0.5f+tl, static_cast<float>(t), this->z}, this->uz,
                                               this->framecolour, this->framelinewidth*0.5f);
                    }
                }
            }
        }

        //! Draw the tick labels (the numbers)
        void drawTickLabels()
        {
            // Reset these members
            this->ticklabelheight = 0.0f;
            this->ticklabelwidth = 0.0f;

            if (this->tickside == colourbar_tickside::left_or_above || this->tickside == colourbar_tickside::left_or_above_ticksboth) {
                if (this->orientation == colourbar_orientation::horizontal) {
                    // Labels above
                    for (unsigned int i = 0; i < this->tick_posns.size(); ++i) {
                        std::string s = morph::GraphVisual<F, glver>::graphNumberFormat (this->ticks[i]);
                        auto lbl = std::make_unique<morph::VisualTextModel<glver>> (this->parentVis, this->get_tprog(this->parentVis), this->tf,
                                                                                    this->get_glfn(this->parentVis));
                        morph::TextGeometry geom = lbl->getTextGeometry (s);
                        this->ticklabelheight = geom.height() > this->ticklabelheight ? geom.height() : this->ticklabelheight;
                        this->ticklabelwidth = geom.width() > this->ticklabelwidth ? geom.width() : this->ticklabelwidth;
                        morph::vec<float> lblpos = {
                            static_cast<float>(this->tick_posns[i])-geom.half_width(),
                            this->width + this->ticklabelgap,
                            this->z
                        };
                        lbl->setupText (s, lblpos+this->mv_offset, this->framecolour);
                        this->texts.push_back (std::move(lbl));
                    }
                } else {
                    // Labels left
                    for (unsigned int i = 0; i < this->tick_posns.size(); ++i) {
                        std::string s = morph::GraphVisual<F, glver>::graphNumberFormat (this->ticks[i]);
                        auto lbl = std::make_unique<morph::VisualTextModel<glver>> (this->parentVis, this->get_tprog(this->parentVis), this->tf,
                                                                                    this->get_glfn(this->parentVis));
                        morph::TextGeometry geom = lbl->getTextGeometry (s);
                        this->ticklabelheight = geom.height() > this->ticklabelheight ? geom.height() : this->ticklabelheight;
                        this->ticklabelwidth = geom.width() > this->ticklabelwidth ? geom.width() : this->ticklabelwidth;
                        morph::vec<float> lblpos = {
                            -this->ticklabelgap - geom.width(),
                            static_cast<float>(this->tick_posns[i])-geom.half_height(),
                            this->z
                        };
                        lbl->setupText (s, lblpos+this->mv_offset, this->framecolour);
                        this->texts.push_back (std::move(lbl));
                    }
                }

            } else if (this->tickside == colourbar_tickside::right_or_below || this->tickside == colourbar_tickside::right_or_below_ticksboth) {
                if (this->orientation == colourbar_orientation::horizontal) {
                    // Labels below
                    for (unsigned int i = 0; i < this->tick_posns.size(); ++i) {
                        std::string s = morph::GraphVisual<F, glver>::graphNumberFormat (this->ticks[i]);
                        auto lbl = std::make_unique<morph::VisualTextModel<glver>> (this->parentVis, this->get_tprog(this->parentVis), this->tf,
                                                                                    this->get_glfn(this->parentVis));
                        morph::TextGeometry geom = lbl->getTextGeometry (s);
                        this->ticklabelheight = geom.height() > this->ticklabelheight ? geom.height() : this->ticklabelheight;
                        this->ticklabelwidth = geom.width() > this->ticklabelwidth ? geom.width() : this->ticklabelwidth;
                        morph::vec<float> lblpos = {
                            static_cast<float>(this->tick_posns[i])-geom.half_width(),
                            -(this->ticklabelgap + geom.height()),
                            this->z
                        };
                        lbl->setupText (s, lblpos+this->mv_offset, this->framecolour);
                        this->texts.push_back (std::move(lbl));
                    }
                } else {
                    // Labels right
                    for (unsigned int i = 0; i < this->tick_posns.size(); ++i) {
                        std::string s = morph::GraphVisual<F, glver>::graphNumberFormat (this->ticks[i]);
                        auto lbl = std::make_unique<morph::VisualTextModel<glver>> (this->parentVis, this->get_tprog(this->parentVis), this->tf,
                                                                                    this->get_glfn(this->parentVis));
                        morph::TextGeometry geom = lbl->getTextGeometry (s);
                        this->ticklabelheight = geom.height() > this->ticklabelheight ? geom.height() : this->ticklabelheight;
                        this->ticklabelwidth = geom.width() > this->ticklabelwidth ? geom.width() : this->ticklabelwidth;
                        morph::vec<float> lblpos = {
                            this->width + this->ticklabelgap,
                            static_cast<float>(this->tick_posns[i])-geom.half_height(),
                            this->z
                        };
                        lbl->setupText (s, lblpos+this->mv_offset, this->framecolour);
                        this->texts.push_back (std::move(lbl));
                    }
                }
            }
        }

        //! Draw the axis label
        void drawAxisLabel()
        {
            if (this->label.empty()) { return; }

            float ticksgap = this->tickstyle == tickstyle::ticksin ? 0.0f : this->ticklength;

            auto lbl = std::make_unique<morph::VisualTextModel<glver>> (this->parentVis, this->get_tprog(this->parentVis), this->tf,
                                                                        this->get_glfn(this->parentVis));
            morph::TextGeometry geom = lbl->getTextGeometry (this->label);
            morph::vec<float> lblpos = {0,0,0};

            // Conditions to find lbl pos:
            if (this->tickside == colourbar_tickside::left_or_above || this->tickside == colourbar_tickside::left_or_above_ticksboth) {
                if (this->orientation == colourbar_orientation::horizontal) {
                    // Label above
                    lblpos = {
                        0.5f * this->length - geom.half_width(),
                         this->width + (ticksgap + this->ticklabelgap + this->ticklabelheight + this->axislabelgap),
                         this->z
                    };
                } else {
                    // Label left
                    lblpos = {
                        -(ticksgap + this->ticklabelgap + this->ticklabelwidth + this->axislabelgap + geom.width()),
                        this->length * 0.5f - geom.height() * 0.5f,
                        this->z
                    };
                }

            } else if (this->tickside == colourbar_tickside::right_or_below || this->tickside == colourbar_tickside::right_or_below_ticksboth) {
                if (this->orientation == colourbar_orientation::horizontal) {
                    // Label below
                    lblpos = {
                        0.5f * this->length - geom.half_width(),
                         -(ticksgap + this->ticklabelgap + this->ticklabelheight + this->axislabelgap + geom.height()),
                         this->z
                    };
                } else {
                    // Label right
                    lblpos = {
                        this->width + (ticksgap + this->ticklabelgap + this->ticklabelwidth + this->axislabelgap),
                         this->length * 0.5f - geom.height() * 0.5f,
                         this->z
                    };
                }
            }

            lbl->setupText (this->label, lblpos+this->mv_offset, this->framecolour);
            this->texts.push_back (std::move(lbl));
        }

        void fillFrameWithColour()
        {
            morph::vec<float, 2> extents = { width, length };
            if (this->orientation == colourbar_orientation::horizontal) { extents = { length, width }; }

            float segstart = 0.0f;
            float segend = 0.0f;
            float seglen = this->length / this->numsegs;
            float colourval = 0.5f/this->numsegs;
            // Used for quad corners
            vec<float> c1;
            vec<float> c2;
            vec<float> c3;
            vec<float> c4;
            if (this->orientation == colourbar_orientation::horizontal) {
                for (unsigned int seg = 0; seg < this->numsegs; ++seg) {
                    segstart = seg * seglen;
                    segend = segstart + seglen;
                    c1 = { segstart,           0, this->z };
                    c2 = { segstart, this->width, this->z };
                    c3 = { segend,   this->width, this->z };
                    c4 = { segend,             0, this->z };
                    this->computeFlatQuad (c1, c2, c3, c4, this->cm.convert(colourval));
                    colourval += 1.0f/this->numsegs;
                }
            } else { // vertical
                for (unsigned int seg = 0; seg < this->numsegs; ++seg) {
                    segstart = seg * seglen;
                    segend = segstart + seglen;
                    c1 = { 0,           segstart, this->z };
                    c2 = { 0,           segend,   this->z };
                    c3 = { this->width, segend,   this->z };
                    c4 = { this->width, segstart, this->z };
                    this->computeFlatQuad (c1, c2, c3, c4, this->cm.convert(colourval));
                    colourval += 1.0f/this->numsegs;
                }
            }
        }

        //! The ColourMap to show (copy in)
        morph::ColourMap<F> cm;
        //! A copy of the scaling for the data. This will map data_min -> data_max to 0->1
        morph::scale<F> scale;
        //! A scaling between colourbar value and model position. Scales 0->1 to 0->this->length
        morph::scale<F> tickscale;
        //! The width of the ColourBar
        float width = 0.1f;
        //! The length of the ColourBar (the colours vary along this direction)
        float length = 0.6f;
        //! Orientation. Two options. Horz and vert. Vertical by default
        colourbar_orientation orientation = colourbar_orientation::vertical;
        //! Which side to place the ticks and tick labels?
        colourbar_tickside tickside = colourbar_tickside::right_or_below;
        //! Position in z in model space. Default is just 0.
        float z = 0.0f;
        //! colour for the axis box/lines. Text also takes this colour.
        std::array<float, 3> framecolour = morph::colour::black;
        //! Set axis and text colours for a dark or black background
        bool darkbg = false;
        //! Plot ONLY the max and min values of the scaling?
        bool justmaxmin = false;
        //! The line width of the colourbar frame
        float framelinewidth = 0.006f;
        //! How long should the ticks be?
        float ticklength = 0.02f;
        //! Ticks in or ticks out? Or something else?
        morph::tickstyle tickstyle = tickstyle::ticksout;
        //! Should ticks be manually set? Placeholder for future implementation
        bool manualticks = false;
        //! The ytick values that should be displayed
        std::deque<F> ticks;
        //! The positions, along the length of the frame (in model space) for the ticks
        std::deque<F> tick_posns;
        // Stores all the text features for this ColourBar (font, colour, font size, font res)
        morph::TextFeatures tf;
        // might need tickfontsize and axisfontsize
        //! Gap to x axis tick labels
        float ticklabelgap = 0.05f;
        //! Gap from tick labels to axis label
        float axislabelgap = 0.05f;
        //! The axis label
        std::string label = "";
        //! The number of segments to make in the colourmap
        unsigned int numsegs = 256;
    protected:
        //! tick label height
        float ticklabelheight = 0.0f;
        //! tick label width
        float ticklabelwidth = 0.0f;
    };

} // namespace
