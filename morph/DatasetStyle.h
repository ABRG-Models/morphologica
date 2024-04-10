#pragma once

#include <array>
#include <string>
#include <bitset>
#include <morph/graphstyles.h>
#include <morph/ColourMap.h>
#include <morph/colour.h>

namespace morph {

    // Boolean flags relating to quiver plots that form part of a DatasetStyle
    enum class quiver_flags
    {
        length_fixed,
        thickness_fixed,
        show_zeros,
        marker_sphere,
        num
    };

    //! The attributes for graphing a single dataset
    struct DatasetStyle
    {
        DatasetStyle() { this->quiver_flag_defaults(); }
        DatasetStyle(stylepolicy p)
        {
            this->policy = p;
            this->quiver_flag_defaults();
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

        //! Quiver parameters
        ///@{
        //! Flags for quiver features
        std::bitset<4> quiver_flagset;
        void quiver_flag_defaults()
        {
            //! If true, use markersize for quiver length. Setting a fixed quiver length can be useful
            //! to focus on the flow of the field.
            this->quiver_flagset.set (static_cast<unsigned int>(quiver_flags::length_fixed), false);
            //! If true, use line width for quiver thickness
            this->quiver_flagset.set (static_cast<unsigned int>(quiver_flags::thickness_fixed), false);
            //! If true, show a marker indicating the location of zero length quivers
            this->quiver_flagset.set (static_cast<unsigned int>(quiver_flags::show_zeros), true);
            //! If true then show a markersize sphere drawn on the coordinate location
            this->quiver_flagset.set (static_cast<unsigned int>(quiver_flags::marker_sphere), false);
        }
        //! Allows user to linearly scale the size of the quivers that are plotted.
        float quiver_length_gain = 1.0f;
        //! Gains for the individual components of the quivers
        morph::vec<float, 3> quiver_gain = { 1.0f, 1.0f, 1.0f };
        //! Allows user to scale the thickness of the quivers.
        float quiver_thickness_gain = 1.0f;
        //! What proportion of the arrow length should the arrowhead cone length be?
        float quiver_arrowhead_prop = 0.25f;
        //! How many times thicker than the quiver width should the arrowhead cone be?
        float quiver_conewidth = 2.0f;
        //! User can choose a colour
        std::array<float, 3> quiver_zero_colour = morph::colour::crimson;
        //! Colourmap for selecting quiver length based colour
        morph::ColourMap<float> quiver_colourmap;
        ///@}

        //! A setter to set both colours to the same value
        void setcolour (const std::array<float, 3>& c)
        {
            this->linecolour = c;
            this->markercolour = c;
        }

        //! Obtain the curated dataset colours, by index. Static public function to
        //! allow other Visuals to set marker style in the same order as a graph.
        static constexpr morph::markerstyle datamarkerstyle (unsigned int data_index)
        {
            morph::markerstyle rtn = morph::markerstyle::square;
            switch (data_index) {
            case 0: { rtn = morph::markerstyle::square; break; }
            case 1: { rtn = morph::markerstyle::triangle; break; }
            case 2: { rtn = morph::markerstyle::circle; break; }
            case 3: { rtn = morph::markerstyle::diamond; break; }
            case 4: { rtn = morph::markerstyle::hexagon; break; }
            case 5: { rtn = morph::markerstyle::heptagon; break; }
            case 6: { rtn = morph::markerstyle::uptriangle; break; }
            case 7: { rtn = morph::markerstyle::octagon; break; }
            case 8: { rtn = morph::markerstyle::uphexagon; break; }
            case 9: { rtn = morph::markerstyle::octagon; break; }
            default: { rtn = morph::markerstyle::upheptagon; break; }
            }
            return rtn;
        }

        //! Obtain the curated dataset colours, by index. Static public function to
        //! allow other Visuals to colour things in the same order as a graph.
        static constexpr std::array<float, 3> datacolour (unsigned int data_index)
        {
            std::array<float, 3> rtn = morph::colour::gray50;
            switch (data_index) {
            case 0: { rtn = morph::colour::royalblue; break; }
            case 1: { rtn = morph::colour::crimson; break; }
            case 2: { rtn = morph::colour::goldenrod2; break; }
            case 3: { rtn = morph::colour::green2; break; }

            case 4: { rtn = morph::colour::blue2; break; }
            case 5: { rtn = morph::colour::blueviolet; break; }
            case 6: { rtn = morph::colour::khaki3; break; }
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

        //! Set defaults on this dataset as if it were for dataset index data_index
        void setdefaults (unsigned int data_index)
        {
            this->markercolour = DatasetStyle::datacolour (data_index);
            // maybe: this->linecolour = DatasetStyle::datacolour (data_index);
            this->markerstyle = DatasetStyle::datamarkerstyle (data_index);
        }
    };

} // namespace morph
