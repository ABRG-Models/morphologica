/*!
 * \file
 *
 * Defines a coordinate arrow class
 *
 * \author Seb James
 * \date 2019
 */
#pragma once

#include <morph/gl/version.h>
#include <morph/vec.h>
#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <morph/colour.h>
#include <array>

namespace morph {

    //! This class creates the vertices for a set of coordinate arrows to be rendered
    //! in a 3-D scene.
    template<int glver = morph::gl::version_4_1>
    class CoordArrows : public VisualModel<glver>
    {
    public:
        CoordArrows() : morph::VisualModel<glver>() {}
        CoordArrows (const morph::vec<float, 3>& offset) : morph::VisualModel<glver>(offset) {}
        virtual ~CoordArrows () {}

        //! Must make the boilerplate bindmodel call before calling init() (for text handling)
        void init (const vec<float, 3> _lengths, const float _thickness, const float _em)
        {
            this->lengths = _lengths;
            this->thickness = _thickness;
            this->em = _em;
        }

        //! Make sure coord arrow colours are ok on the given background colour. Call this *after* finalize.
        void setColourForBackground (const std::array<float, 4>& bgcolour)
        {
            // For now, only worry about the centresphere:
            std::array<float, 3> cscol = { 1.0f - bgcolour[0], 1.0f - bgcolour[1], 1.0f - bgcolour[2] };
            if (cscol != this->centresphere_col) {
                this->centresphere_col = cscol;
                this->reinit(); // sets context, does not release it

                // Give the text labels a suitable, visible colour
                if (this->setContext != nullptr) { this->setContext (this->parentVis); }
                auto ti = this->texts.begin();
                while (ti != this->texts.end()) {
                    (*ti)->setVisibleOn (bgcolour);
                    ti++;
                }
                if (this->releaseContext != nullptr) { this->releaseContext (this->parentVis); }
            }
        }

        void initAxisLabels()
        {
            if (this->em > 0.0f) {

                if (this->setContext != nullptr) { this->setContext (this->parentVis); } // For VisualTextModel

                morph::TextFeatures tfca(this->em, 48, false, morph::colour::black, morph::VisualFont::DVSansItalic);

                // These texts are black by default
                morph::vec<float> toffset = this->mv_offset;
                toffset[0] += this->lengths[0] + this->em;
                auto vtm1 = this->makeVisualTextModel (tfca);
                vtm1->setupText (this->x_label, toffset);
                this->texts.push_back (std::move(vtm1));
                toffset = this->mv_offset;
                toffset[1] += this->lengths[1];
                toffset[0] += this->em;
                auto vtm2 = this->makeVisualTextModel (tfca);
                vtm2->setupText (this->y_label, toffset);
                this->texts.push_back (std::move(vtm2));
                toffset = this->mv_offset;
                toffset[2] += this->lengths[2];
                toffset[0] += this->em;
                auto vtm3 = this->makeVisualTextModel (tfca);
                vtm3->setupText (this->z_label, toffset);
                this->texts.push_back (std::move(vtm3));

                if (this->releaseContext != nullptr) { this->releaseContext (this->parentVis); }
            }
        }

        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();
            this->idx = 0;

            // Draw four spheres to make up the coord frame, with centre at 0,0,0
            // (mv_offset is applied in translation matrices)
            vec<float, 3> reloffset = {0,0,0};
            vec<float, 3> zerocoord = {0,0,0};
            this->computeSphere (zerocoord, centresphere_col, this->thickness*this->lengths[0]/20.0);

            // x
            reloffset[0] += this->lengths[0];
            this->computeSphere (reloffset, x_axis_col, this->thickness*this->lengths[0]/40.0);
            this->computeTube (zerocoord, reloffset, x_axis_col, x_axis_col, this->thickness*this->lengths[0]/80.0);

            // y
            reloffset[0] -= this->lengths[0];
            reloffset[1] += this->lengths[1];
            this->computeSphere (reloffset, y_axis_col, this->thickness*this->lengths[0]/40.0);
            this->computeTube (zerocoord, reloffset, y_axis_col, y_axis_col, this->thickness*this->lengths[0]/80.0);

            // z
            reloffset[1] -= this->lengths[1];
            reloffset[2] += this->lengths[2];
            this->computeSphere (reloffset, z_axis_col, this->thickness*this->lengths[0]/40.0);
            this->computeTube (zerocoord, reloffset, z_axis_col, z_axis_col, this->thickness*this->lengths[0]/80.0);

            this->initAxisLabels();
        }

        //! The lengths of the x, y and z arrows.
        vec<float, 3> lengths = { 1.0f, 1.0f, 1.0f };
        //! A thickness scaling factor, to apply to the arrows.
        float thickness = 1.0f;
        //! m size for text labels
        float em = 0.0f;

        //! The colours of the arrows, and of the centre sphere (where default of black is suitable
        //! for a white background)
        std::array<float, 3> centresphere_col = morph::colour::black;
        std::array<float, 3> x_axis_col = morph::colour::crimson;
        std::array<float, 3> y_axis_col = morph::colour::springgreen2;
        std::array<float, 3> z_axis_col = morph::colour::blue2;

        std::string x_label = "X";
        std::string y_label = "Y";
        std::string z_label = "Z";
    };

} // namespace morph
