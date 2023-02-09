/*!
 * \file
 *
 * Defines a coordinate arrow class
 *
 * \author Seb James
 * \date 2019
 */
#pragma once

#include <morph/vec.h>
#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <morph/VisualTextModel.h>
#include <array>

namespace morph {

    //! This class creates the vertices for a set of coordinate arrows to be rendered
    //! in a 3-D scene.
    class CoordArrows : public VisualModel
    {
    public:
        CoordArrows()
        {
            this->lengths = {1.0, 1.0, 1.0};
            this->mv_offset = {0.2, 0.2, 0.0};
        }

        //! Construct CoordArrows with given shaderprograms sp (graphics) and tsp
        //! (text). _lengths is the 3 lengths of the coordinate axes, _thickness is a
        //! factor to slim/thicken the axes and _em controls the size of the axis
        //! labels. Set _em to 0.0f to omit the text x/y/z labels.
        CoordArrows(morph::gl::shaderprogs& _shaders,
                    const vec<float, 3> _lengths, const float _thickness = 1.0f, const float _em = 0.02f)
        {
            this->init (_shaders, _lengths, _thickness, _em);
        }

        virtual ~CoordArrows () {}

        void init (morph::gl::shaderprogs& _shaders,
                   const vec<float, 3> _lengths, const float _thickness, const float _em)
        {
            // Set up...
            this->shaders = _shaders;
            this->mv_offset = {0.0, 0.0, 0.0};
            this->lengths = _lengths;
            this->thickness = _thickness;
            this->em = _em;

            this->initAxisLabels();
            // Initialize the vertices that will represent the object
            this->initializeVertices();

            this->postVertexInit();
        }

        //! Make sure coord arrow colours are ok on the given background colour
        void setColourForBackground (const std::array<float, 4>& bgcolour)
        {
            // For now, only worry about the centresphere:
            std::array<float, 3> cscol = {1.0f-bgcolour[0],
                                          1.0f-bgcolour[1],
                                          1.0f-bgcolour[2]};
            if (cscol != this->centresphere_col) {
                this->centresphere_col = cscol;
                this->initializeVertices();
                this->postVertexInit();
            }

            // Give the text labels a suitable, visible colour
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) {
                (*ti)->setVisibleOn (bgcolour);
                ti++;
            }
        }

        void initAxisLabels()
        {
            if (this->em > 0.0f) {
                morph::vec<float> toffset = this->mv_offset;
                toffset[0] += this->lengths[0] + this->em;
                auto vtm1 = std::make_unique<VisualTextModel> (this->shaders.tprog,
                                                               morph::VisualFont::DVSansItalic,
                                                               this->em, 48, toffset,
                                                               this->x_label);
                this->texts.push_back (std::move(vtm1));
                toffset = this->mv_offset;
                toffset[1] += this->lengths[1];
                toffset[0] += this->em;
                auto vtm2 = std::make_unique<VisualTextModel> (this->shaders.tprog,
                                                               morph::VisualFont::DVSansItalic,
                                                               this->em, 48, toffset,
                                                               this->y_label);
                this->texts.push_back (std::move(vtm2));
                toffset = this->mv_offset;
                toffset[2] += this->lengths[2];
                toffset[0] += this->em;
                auto vtm3 = std::make_unique<VisualTextModel> (this->shaders.tprog,
                                                               morph::VisualFont::DVSansItalic,
                                                               this->em, 48, toffset,
                                                               this->z_label);
                this->texts.push_back (std::move(vtm3));
            }
        }

        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();

            // The indices index
            VBOint idx = 0;

            // Draw four spheres to make up the coord frame, with centre at 0,0,0
            // (mv_offset is applied in translation matrices)
            vec<float, 3> reloffset = {0,0,0};
            vec<float, 3> zerocoord = {0,0,0};
            this->computeSphere (idx, zerocoord, centresphere_col, this->thickness*this->lengths[0]/20.0);

            // x
            reloffset[0] += this->lengths[0];
            this->computeSphere (idx, reloffset, x_axis_col, this->thickness*this->lengths[0]/40.0);
            this->computeTube (idx, zerocoord, reloffset, x_axis_col, x_axis_col, this->thickness*this->lengths[0]/80.0);

            // y
            reloffset[0] -= this->lengths[0];
            reloffset[1] += this->lengths[1];
            this->computeSphere (idx, reloffset, y_axis_col, this->thickness*this->lengths[0]/40.0);
            this->computeTube (idx, zerocoord, reloffset, y_axis_col, y_axis_col, this->thickness*this->lengths[0]/80.0);

            // z
            reloffset[1] -= this->lengths[1];
            reloffset[2] += this->lengths[2];
            this->computeSphere (idx, reloffset, z_axis_col, this->thickness*this->lengths[0]/40.0);
            this->computeTube (idx, zerocoord, reloffset, z_axis_col, z_axis_col, this->thickness*this->lengths[0]/80.0);
        }

        //! The lengths of the x, y and z arrows.
        vec<float, 3> lengths;
        //! A thickness scaling factor, to apply to the arrows.
        float thickness = 1.0f;
        //! m size for text labels
        float em = 0.0f;

        //! The colours of the arrows, and of the centre sphere
        std::array<float, 3> centresphere_col = {1.0f, 1.0f, 1.0f};
        std::array<float, 3> x_axis_col = {1.0f, 0.0f, 0.0f}; // Red
        std::array<float, 3> y_axis_col = {0.0f, 1.0f, 0.0f}; // Green
        std::array<float, 3> z_axis_col = {0.0f, 0.0f, 1.0f}; // Blue

        std::string x_label = "X";
        std::string y_label = "Y";
        std::string z_label = "Z";
    };

} // namespace morph
