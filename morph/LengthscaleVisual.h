/*
 * This is a VisualModel to place a length marker in your scene
 */

#pragma once

#include <morph/vec.h>
#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <morph/scale.h>
#include <array>

namespace morph {

    //! A length scale marker
    template<int glver = morph::gl::version_4_1>
    class LengthscaleVisual : public VisualModel<glver>
    {
    public:
        LengthscaleVisual()  : morph::VisualModel<glver>()
        {
            this->scene_to_units_scale.compute_scaling (0.0f, 1.0f);
            this->text_features.colour = this->colr;
        }
        LengthscaleVisual(const vec<float, 3> _offset) : morph::VisualModel<glver>(_offset)
        {
            this->scene_to_units_scale.compute_scaling (0.0f, 1.0f);
            this->text_features.colour = this->colr;
        }

        void initializeVertices()
        {
            // start coord is always the origin, the length is the inverse transf. of represented distance
            morph::vec<float> end_coord = this->axis * this->scene_to_units_scale.inverse_one (this->represented_distance);
            // The length scale marker is a flat line
            this->computeFlatLine (morph::vec<float>{0.0f}, end_coord,
                                   morph::vec<float>{0.0f}, end_coord, this->upaxis,
                                   this->colr, this->width);
            this->drawCaptionLabel (end_coord);
        }

        void drawCaptionLabel (morph::vec<float>& end_coord)
        {
            if (this->label.empty()) { return; }
            morph::vec<float> bar_centre = end_coord / 2.0f;
            auto lbl = this->makeVisualTextModel (this->text_features);
            morph::TextGeometry geom = lbl->getTextGeometry (this->label);
            morph::vec<float> lblpos = { 0.0f };

            morph::quaternion<float> text_rotn; // no rotation by default

            if (this->axis == this->ux) {
                lblpos = bar_centre + morph::vec<float>{ -geom.half_width(), -this->width * 0.5f - 3.0f * geom.half_height(), 0.0f  };

            } else if (this->axis == this->uy) {
                // Rotated label
                lblpos = bar_centre + morph::vec<float>{ this->width * 0.5f + 3.0f * geom.half_height(), -geom.half_width(), 0.0f  };
                // Set rotation
                text_rotn.rotate (this->upaxis, morph::mathconst<float>::pi_over_2);

            } else {
                // write me for other orientations
                lblpos = bar_centre;
            }
            lbl->setupText (this->label, text_rotn, lblpos + this->mv_offset, this->text_features.colour);
            this->texts.push_back (std::move(lbl));
        }


        //! A width for the length scale line
        float width = 0.025f;
        //! A colour for the length scale tube (caption defaults to this colour, too)
        std::array<float, 3> colr = morph::colour::black;
        //! The distance represented by this length scale marker
        float represented_distance = 1.0f;
        //! The scaling between scene units and the units that the length scale represents (by default a 1:1 mapping)
        morph::scale<float> scene_to_units_scale;
        //! A caption label to write next to the length scale
        std::string label = "1 unit";
        //! Font size, etc
        morph::TextFeatures text_features;
        // A unit vector defining the axis along which this length scale will be drawn
        morph::vec<float> axis = this->ux;
        //! A unit vector defining what direction we want to look at the text from
        morph::vec<float> upaxis = this->uz;
    };

} // namespace morph
