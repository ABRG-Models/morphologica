/*
 * A VisualModel for rendering a set of 3D axes, either 3 axes or a kind of framework
 * box. Use along with ScatterVisual or HexGridVisual for plotting 3D graph
 * visualisations.
 */
#pragma once

#ifndef USE_GLEW
#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#endif
#include <morph/MathConst.h>
#include <morph/Scale.h>
#include <morph/Vector.h>
#include <morph/GraphVisual.h> // Share tickstyle, axestyle and possibly scalingpolicy from GraphVisual

namespace morph {

    template <typename Flt>
    class TriaxesVisual : public VisualModel
    {
    public:
        //! Constructor
        //! \param sp shader program id
        //! \param tsp text shader program id
        //! \param _offset The offset within morph::Visual space to place these axes
        TriaxesVisual (GLuint sp, GLuint tsp, const Vector<float> _offset)
        {
            this->shaderprog = sp;
            this->tshaderprog = tsp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->x_scale.do_autoscale = true;
            this->y_scale.do_autoscale = true;
            this->z_scale.do_autoscale = true;
        }

        void initializeVertices()
        {
            // First compute the x/y/z scales. Set the range_max of each to the ends of
            // the axes leaving range_mins at 0.
            this->x_scale.range_max = this->axis_ends[0];
            this->y_scale.range_max = this->axis_ends[1];
            this->z_scale.range_max = this->axis_ends[2];

            this->x_scale.compute_autoscale (this->input_min[0], this->input_max[0]);
            this->y_scale.compute_autoscale (this->input_min[1], this->input_max[1]);
            this->z_scale.compute_autoscale (this->input_min[2], this->input_max[2]);

            // Now ensure that this->[x/y/z]tick_posns/[x/y/z]ticks are populated
            this->computeTickPositions();
            this->drawAxes();
            // Draw ticks, tick labels and axis labels
            this->drawTicks();
            this->drawTickLabels();
            this->drawAxisLabels();
        }

        //! Based on axis scaling, compute tick positions
        void computeTickPositions()
        {
            if (this->manualticks == true) {
                std::cout << "Writeme: Implement a manual tick-setting scheme\n";
            } else {
                // Compute locations for ticks...
                Flt _xmin = this->x_scale.inverse_one (this->x_scale.range_min);
                Flt _xmax = this->x_scale.inverse_one (this->x_scale.range_max);
                Flt _ymin = this->y_scale.inverse_one (this->y_scale.range_min);
                Flt _ymax = this->y_scale.inverse_one (this->y_scale.range_max);
                Flt _zmin = this->z_scale.inverse_one (this->z_scale.range_min);
                Flt _zmax = this->z_scale.inverse_one (this->z_scale.range_max);

                float realmin = this->x_scale.inverse_one (0);
                float realmax = this->x_scale.inverse_one (this->axis_ends[0]);
                this->xticks = morph::GraphVisual<Flt>::maketicks (_xmin, _xmax, realmin, realmax, 8);

                realmin = this->y_scale.inverse_one (0);
                realmax = this->y_scale.inverse_one (this->axis_ends[1]);
                this->yticks = morph::GraphVisual<Flt>::maketicks (_ymin, _ymax, realmin, realmax, 8);

                realmin = this->z_scale.inverse_one (0);
                realmax = this->z_scale.inverse_one (this->axis_ends[2]);
                this->zticks = morph::GraphVisual<Flt>::maketicks (_zmin, _zmax, realmin, realmax, 8);

                this->xtick_posns.resize (this->xticks.size());
                this->x_scale.transform (xticks, xtick_posns);

                this->ytick_posns.resize (this->yticks.size());
                this->y_scale.transform (yticks, ytick_posns);

                this->ztick_posns.resize (this->zticks.size());
                this->z_scale.transform (zticks, ztick_posns);
            }
        }

        //! Draw the axis bars
        void drawAxes()
        {
            // Draw the main axes x. Draw a rectangular tube of side axislinewidth
            // (Specifying radius = axislinewidth/root(2) and 45 degree rotation)
            this->computeTube (this->idx,
                               { -0.5f*axislinewidth,                   0, 0 }, // start
                               { this->axis_ends[0]+0.5f*axislinewidth, 0, 0 }, // end
                               -uy, uz,
                               this->axiscolour, this->axiscolour,
                               morph::mathconst<float>::one_over_root_2*this->axislinewidth,
                               4, morph::mathconst<float>::pi_over_4);
            // y
            this->computeTube (this->idx,
                               { 0, -0.5f*axislinewidth,                   0 },
                               { 0, this->axis_ends[1]+0.5f*axislinewidth, 0 },
                               ux, uz,
                               this->axiscolour, this->axiscolour,
                               morph::mathconst<float>::one_over_root_2*this->axislinewidth,
                               4, morph::mathconst<float>::pi_over_4);
            // z
            this->computeTube (this->idx,
                               {0, 0, -0.5f*axislinewidth},
                               {0, 0, this->axis_ends[2]+0.5f*axislinewidth},
                               ux, uy,
                               this->axiscolour, this->axiscolour,
                               morph::mathconst<float>::one_over_root_2*this->axislinewidth,
                               4, morph::mathconst<float>::pi_over_4);

            // Complete the box side panels if required
            if (this->axisstyle == axisstyle::box || this->axisstyle == axisstyle::panels) {
                // x
                this->computeTube (this->idx,
                                   { -0.5f*axislinewidth,                   0, this->axis_ends[2] },
                                   { this->axis_ends[0]+0.5f*axislinewidth, 0, this->axis_ends[2] },
                                   -uy, uz,
                                   this->axiscolour2, this->axiscolour2,
                                   morph::mathconst<float>::one_over_root_2*this->axislinewidth,
                                   4, morph::mathconst<float>::pi_over_4);
                this->computeTube (this->idx,
                                   { -0.5f*axislinewidth,                   this->axis_ends[1], 0 },
                                   { this->axis_ends[0]+0.5f*axislinewidth, this->axis_ends[1], 0 },
                                   -uy, uz,
                                   this->axiscolour2, this->axiscolour2,
                                   morph::mathconst<float>::one_over_root_2*this->axislinewidth,
                                   4, morph::mathconst<float>::pi_over_4);
                // y
                this->computeTube (this->idx,
                                   { 0, -0.5f*axislinewidth,                   this->axis_ends[2] },
                                   { 0, this->axis_ends[1]+0.5f*axislinewidth, this->axis_ends[2] },
                                   ux, uz,
                                   this->axiscolour2, this->axiscolour2,
                                   morph::mathconst<float>::one_over_root_2*this->axislinewidth,
                                   4, morph::mathconst<float>::pi_over_4);
                this->computeTube (this->idx,
                                   { this->axis_ends[0], -0.5f*axislinewidth,                   0 },
                                   { this->axis_ends[0], this->axis_ends[1]+0.5f*axislinewidth, 0 },
                                   ux, uz,
                                   this->axiscolour2, this->axiscolour2,
                                   morph::mathconst<float>::one_over_root_2*this->axislinewidth,
                                   4, morph::mathconst<float>::pi_over_4);
                // z
                this->computeTube (this->idx,
                                   { this->axis_ends[0], 0, -0.5f*axislinewidth },
                                   { this->axis_ends[0], 0, this->axis_ends[2]+0.5f*axislinewidth },
                                   ux, uy,
                                   this->axiscolour2, this->axiscolour2,
                                   morph::mathconst<float>::one_over_root_2*this->axislinewidth,
                                   4, morph::mathconst<float>::pi_over_4);
                this->computeTube (this->idx,
                                   { 0, this->axis_ends[1], -0.5f*axislinewidth },
                                   { 0, this->axis_ends[1], this->axis_ends[2]+0.5f*axislinewidth },
                                   ux, uy,
                                   this->axiscolour2, this->axiscolour2,
                                   morph::mathconst<float>::one_over_root_2*this->axislinewidth,
                                   4, morph::mathconst<float>::pi_over_4);
            }

            if (this->axisstyle == axisstyle::box) {
                // Last few here.
            }
        }

        // Draw the tick marks on each axis
        void drawTicks()
        {
            float tl = -this->ticklength;
            if (this->tickstyle == tickstyle::ticksin) { tl = this->ticklength; }
            // x ticks
            for (auto xt : this->xtick_posns) {
                this->computeFlatLine (this->idx,
                                       {(float)xt, 0.0f, 0.0f},
                                       {(float)xt, tl,   0.0f}, uz,
                                       this->axiscolour, this->axislinewidth*0.5f);
            }
            // y ticks
            for (auto yt : this->ytick_posns) {
                this->computeFlatLine (this->idx,
                                       {tl, (float)yt, 0.0f},
                                       {0.0f, (float)yt, 0.0f}, uz,
                                       this->axiscolour, this->axislinewidth*0.5f);
            }
            // z ticks
            for (auto zt : this->ztick_posns) {
                this->computeFlatLine (this->idx,
                                       {tl, 0.0f, (float)zt},
                                       {0.0f, 0.0f, (float)zt}, uy,
                                       this->axiscolour, this->axislinewidth*0.5f);
            }
        }

        //! Draw the tick labels (the numbers)
        void drawTickLabels()
        {
            // Reset these members
            this->xtick_height = 0.0f;
            this->ytick_width = 0.0f;
            this->ztick_height = 0.0f;
            this->ztick_width = 0.0f;

            float x_for_yticks = 0.0f;
            float y_for_xticks = 0.0f;
            float y_for_zticks = 0.0f;

            for (unsigned int i = 0; i < this->xtick_posns.size(); ++i) {
                std::string s = morph::GraphVisual<Flt>::graphNumberFormat (this->xticks[i]);
                // Issue: I need the width of the text ss.str() before I can create the
                // VisualTextModel, so need a static method like this:
                morph::VisualTextModel* lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, this->fontres);
                morph::TextGeometry geom = lbl->getTextGeometry (s);
                this->xtick_height = geom.height() > this->xtick_height ? geom.height() : this->xtick_height;
                this->xtick_width = geom.width() > this->xtick_width ? geom.width() : this->xtick_width;
                morph::Vector<float> lblpos = {(float)this->xtick_posns[i]-geom.half_width(), y_for_xticks-(this->ticklabelgap+geom.height()), 0};
                lbl->setupText (s, lblpos+this->mv_offset, this->axiscolour);
                this->texts.push_back (lbl);
            }

            for (unsigned int i = 0; i < this->ytick_posns.size(); ++i) {
                std::string s = morph::GraphVisual<Flt>::graphNumberFormat (this->yticks[i]);
                morph::VisualTextModel* lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, this->fontres);
                morph::TextGeometry geom = lbl->getTextGeometry (s);
                this->ytick_height = geom.height() > this->ytick_height ? geom.height() : this->ytick_height;
                this->ytick_width = geom.width() > this->ytick_width ? geom.width() : this->ytick_width;
                morph::Vector<float> lblpos = {x_for_yticks-this->ticklabelgap-geom.width(), (float)this->ytick_posns[i]-geom.half_height(), 0};
                lbl->setupText (s, lblpos+this->mv_offset, this->axiscolour);
                this->texts.push_back (lbl);
            }

            for (unsigned int i = 0; i < this->ztick_posns.size(); ++i) {
                std::string s = morph::GraphVisual<Flt>::graphNumberFormat (this->zticks[i]);
                morph::VisualTextModel* lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, this->fontres);
                morph::TextGeometry geom = lbl->getTextGeometry (s);
                this->ztick_height = geom.height() > this->ztick_height ? geom.height() : this->ztick_height;
                this->ztick_width = geom.width() > this->ztick_width ? geom.width() : this->ztick_width;
                morph::Vector<float> lblpos = {y_for_zticks-this->ticklabelgap-geom.width(), 0, (float)this->ztick_posns[i]};
                lbl->setupText (s, lblpos+this->mv_offset, this->axiscolour);
                this->texts.push_back (lbl);
            }
        }

        //! Draw the axis labels
        void drawAxisLabels()
        {
            // x axis label (easy)
            morph::VisualTextModel* lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, this->fontres);
            morph::TextGeometry geom = lbl->getTextGeometry (this->xlabel);
            morph::Vector<float> lblpos;
            lblpos = {{0.5f * this->axis_ends[0] - geom.half_width(),
                       -(this->axislabelgap+this->ticklabelgap+geom.height()+this->xtick_height), 0}};
            lbl->setupText (this->xlabel, lblpos+this->mv_offset, this->axiscolour);
            this->texts.push_back (lbl);

            // y axis label (have to rotate)
            lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, this->fontres);
            geom = lbl->getTextGeometry (this->ylabel);

            // Rotate label if it's long
            float leftshift = geom.width();
            float downshift = geom.height();
            if (geom.width() > 2*this->fontsize) { // rotate so shift by text height
                leftshift = geom.height();
                downshift = geom.half_width();
            }

            lblpos = {{ -(this->axislabelgap+leftshift+this->ticklabelgap+this->ytick_width),
                        0.5f*this->axis_ends[1] - downshift, 0 }};

            if (geom.width() > 2*this->fontsize) {
                morph::Quaternion<float> leftrot;
                leftrot.initFromAxisAngle (this->uz, -90.0f);
                lbl->setupText (this->ylabel, leftrot, lblpos+this->mv_offset, this->axiscolour);
            } else {
                lbl->setupText (this->ylabel, lblpos+this->mv_offset, this->axiscolour);
            }
            this->texts.push_back (lbl);

            // z axis
            lbl = new morph::VisualTextModel (this->tshaderprog, this->font, this->fontsize, this->fontres);
            geom = lbl->getTextGeometry (this->zlabel);
            lblpos = {{ -(this->axislabelgap+this->ticklabelgap+geom.width()+this->ztick_width),
                        0,
                        0.5f * this->axis_ends[1] - geom.half_height() }};
            lbl->setupText (this->zlabel, lblpos+this->mv_offset, this->axiscolour);
            this->texts.push_back (lbl);
        }

        //! Set the input_min to be the values at the zero points of the graph axes
        morph::Vector<Flt, 3> input_min = {0,0,0};
        //! Set the input_min to be the values at the maxes of the graph axes
        morph::Vector<Flt, 3> input_max = {1,1,1};

        // Axes parameters

        //! x axis max location in model space. Default behaviour is a 1x1x1 cube
        morph::Vector<Flt, 3> axis_ends = {1,1,1};
        //! colour for the axis box/lines. Text also takes this colour.
        morph::Vector<float, 3> axiscolour = {0,0,0};
        morph::Vector<float, 3> axiscolour2 = { 0.7f, 0.7f, 0.7f };
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
        //! A scaling for the x axis
        morph::Scale<Flt> x_scale;
        //! A scaling for the y axis
        morph::Scale<Flt> y_scale;
        //! A scaling for the z axis
        morph::Scale<Flt> z_scale;
        //! The xtick values that should be displayed
        std::deque<Flt> xticks;
        //! The positions, along the x axis (in model space) for the xticks
        std::deque<Flt> xtick_posns;
        //! The ytick values that should be displayed
        std::deque<Flt> yticks;
        //! The positions, along the y axis (in model space) for the yticks
        std::deque<Flt> ytick_posns;
        //! The ztick values that should be displayed
        std::deque<Flt> zticks;
        //! The positions, along the y axis (in model space) for the yticks
        std::deque<Flt> ztick_posns;
        // Default font
        morph::VisualFont font = morph::VisualFont::Vera;
        //! Font resolution - determines how textures for glyphs are generated. If your
        //! labels will be small, this should be smaller. If labels are large, then it
        //! should be increased.
        int fontres = 24;
        //! The font size is the width of an m in the chosen font, in model units
        float fontsize = 0.05;
        // might need tickfontsize and axisfontsize
        //! Gap to x axis tick labels
        float ticklabelgap = 0.05;
        //! Gap from tick labels to axis label
        float axislabelgap = 0.05;
        //! The x axis label
        std::string xlabel = "x";
        //! The y axis label
        std::string ylabel = "y";
        //! The z axis label
        std::string zlabel = "z";
    protected:
        //! Unit vectors
        morph::Vector<float> ux = {1,0,0};
        morph::Vector<float> uy = {0,1,0};
        morph::Vector<float> uz = {0,0,1};
        //! xtick label height
        float xtick_height = 0.0f;
        //! ytick label height
        float ytick_height = 0.0f;
        //! ztick label height
        float ztick_height = 0.0f;
        //! xtick label width
        float xtick_width = 0.0f;
        //! ytick label width
        float ytick_width = 0.0f;
        //! ztick label width
        float ztick_width = 0.0f;
    };

} // namespace morph
