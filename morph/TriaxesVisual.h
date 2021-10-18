/*
 * A VisualModel for rendering a set of 3D axes, either 3 axes or a kind of framework
 * box. Use along with ScatterVisual or HexGridVisual for plotting 3D graph
 * visualisations.
 */
#pragma once

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
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
            // First compute the x/y/z scales
            this->x_scale.compute_autoscale (0, this->axis_ends[0]);
            this->y_scale.compute_autoscale (0, this->axis_ends[1]);
            this->z_scale.compute_autoscale (0, this->axis_ends[2]);

            // Now ensure that this->[x/y/z]tick_posns/[x/y/z]ticks are populated
            this->computeTickPositions();

            // Draw the main axes
            // x
            this->computeTube (this->idx,
                               {0, 0, 0}, // start
                               {this->axis_ends[0], 0, 0},
                               uy, uz,
                               this->axiscolour, this->axiscolour,
                               this->axislinewidth, 4, morph::mathconst<float>::pi_over_4);
            // y
            this->computeTube (this->idx,
                               {0, 0, 0}, // start
                               {0, this->axis_ends[1], 0},
                               ux, uz,
                               this->axiscolour, this->axiscolour,
                               this->axislinewidth, 4, morph::mathconst<float>::pi_over_4);
            // z
            this->computeTube (this->idx,
                               {0, 0, 0}, // start
                               {0, 0, this->axis_ends[2]},
                               ux, uy,
                               this->axiscolour, this->axiscolour,
                               this->axislinewidth, 4, morph::mathconst<float>::pi_over_4);

            // Then draw ticks
        }

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
                this->xticks = morph::GraphVisual<Flt>::maketicks (_xmin, _xmax, realmin, realmax);

                realmin = this->y_scale.inverse_one (0);
                realmax = this->y_scale.inverse_one (this->axis_ends[1]);
                this->yticks = morph::GraphVisual<Flt>::maketicks (_ymin, _ymax, realmin, realmax);

                realmin = this->z_scale.inverse_one (0);
                realmax = this->z_scale.inverse_one (this->axis_ends[2]);
                this->zticks = morph::GraphVisual<Flt>::maketicks (_zmin, _zmax, realmin, realmax);

                this->xtick_posns.resize (this->xticks.size());
                this->x_scale.transform (xticks, xtick_posns);

                this->ytick_posns.resize (this->yticks.size());
                this->y_scale.transform (yticks, ytick_posns);

                this->ztick_posns.resize (this->zticks.size());
                this->z_scale.transform (zticks, ztick_posns);
            }
        }

        // OpenGL indices index
        VBOint idx = 0;

        // Axes parameters

        //! x axis max location in model space. Default behaviour is a 1x1x1 cube
        morph::Vector<Flt, 3> axis_ends = {1,1,1};
        //! colour for the axis box/lines. Text also takes this colour.
        morph::Vector<float, 3> axiscolour = {0,0,0};
        //! Set axis and text colours for a dark or black background
        bool darkbg = false;
        //! The line width of the main axis bars
        float axislinewidth = 0.01f;
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
        //! Unit vectors
        morph::Vector<float> ux = {1,0,0};
        morph::Vector<float> uy = {0,1,0};
        morph::Vector<float> uz = {0,0,1};
    };

} // namespace morph
