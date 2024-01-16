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
#include <morph/MathAlgo.h>
#include <morph/Scale.h>
#include <morph/vec.h>
#include <iostream>
#include <vector>
#include <array>
#include <stdexcept>

namespace morph {

    template <typename Flt, int glver = morph::gl::version_4_1>
    class QuadsVisual : public VisualDataModel<Flt, glver>
    {
    public:
        QuadsVisual(const std::vector<std::array<Flt,12>>* _quads,
                    const vec<float> _offset,
                    const std::vector<Flt>* _data,
                    const Scale<Flt>& _scale,
                    ColourMapType _cmt,
                    const float _hue = 0.0f)
        {
            // Set up...
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->colourScale = _scale;

            // How to deal with quads? Each quad has a centroid. The coordinate of the
            // centroid of the quad is the location for the data point. Thus, convert
            // the quad data into centroids and save these in VisualDataModel<>::dataCoords.
            this->quads = _quads;

            // From quads, build dataCoords:
            this->dataCoords = new std::vector<vec<float>>(this->quads->size());
            unsigned int qi = 0;
            for (auto q : (*this->quads)) {
                // q is an array<Flt, 12>. These lines compute the centroid:
                (*this->dataCoords)[qi][0] = 0.25f * static_cast<float>(q[0]+q[3]+q[6]+q[9]);
                (*this->dataCoords)[qi][1] = 0.25f * static_cast<float>(q[1]+q[4]+q[7]+q[10]);
                (*this->dataCoords)[qi][2] = 0.25f * static_cast<float>(q[2]+q[5]+q[8]+q[11]);
                ++qi;
            }

            this->scalarData = _data;

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);
        }

        //! Version with std::array \a _offset
        QuadsVisual(const std::vector<std::array<Flt,12>>* _quads,
                    const std::array<float, 3> _offset,
                    const std::vector<Flt>* _data,
                    const Scale<Flt>& _scale,
                    ColourMapType _cmt,
                    const float _hue = 0.0f)
        {
            vec<float> offset_vec;
            offset_vec.set_from(_offset);
            QuadsVisual(_quads, offset_vec, _data, _scale, _cmt, _hue);
        }

        ~QuadsVisual() { delete this->dataCoords; }

        //! Initialize the vertices that will represent the Quads.
        void initializeVertices()
        {
            unsigned int nquads = this->quads->size();
            unsigned int ndata = this->scalarData->size();

            if (nquads != ndata) {
                std::cout << "nquads != ndata, return." << std::endl;
                return;
            }

            std::vector<Flt> dcopy = *(this->scalarData);
            this->colourScale.do_autoscale = true;
            this->colourScale.transform ((*this->scalarData), dcopy);

            morph::vec<float> v0, v1, v2, v3;
            for (unsigned int qi = 0; qi < nquads; ++qi) {

                std::array<float, 12> quad = (*this->quads)[qi];
                // Convert the array of 12 floats into 4 vecs
                v0 = {quad[0], quad[1], quad[2]};
                v1 = {quad[3], quad[4], quad[5]};
                v2 = {quad[6], quad[7], quad[8]};
                v3 = {quad[9], quad[10], quad[11]};

                this->vertex_push (v0, this->vertexPositions);
                this->vertex_push (v1, this->vertexPositions);
                this->vertex_push (v2, this->vertexPositions);
                this->vertex_push (v3, this->vertexPositions);

                // Compute normal
                morph::vec<float> plane1 = v1 - v0;
                morph::vec<float> plane2 = v2 - v0;
                morph::vec<float> vnorm = plane2.cross (plane1);
                vnorm.renormalize();

                // All same colours
                std::array<float, 3> clr = this->cm.convert(dcopy[qi]);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);

                this->vertex_push (vnorm, this->vertexNormals);
                this->vertex_push (vnorm, this->vertexNormals);
                this->vertex_push (vnorm, this->vertexNormals);
                this->vertex_push (vnorm, this->vertexNormals);

                if (this->computeBackQuads == true) {
                    // Compute a 'depth' in the direction of the normal
                    morph::vec<float> depth = vnorm;
                    depth *= plane1.length();
                    depth *= 0.01f;

                    // Back quad vertices
                    this->vertex_push (v0-depth, this->vertexPositions);
                    this->vertex_push (v1-depth, this->vertexPositions);
                    this->vertex_push (v2-depth, this->vertexPositions);
                    this->vertex_push (v3-depth, this->vertexPositions);

                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (clr, this->vertexColors);
                    this->vertex_push (clr, this->vertexColors);

                    this->vertex_push (-vnorm, this->vertexNormals);
                    this->vertex_push (-vnorm, this->vertexNormals);
                    this->vertex_push (-vnorm, this->vertexNormals);
                    this->vertex_push (-vnorm, this->vertexNormals);
                }

                // Two triangles per quad, two quads per quad (front and back)
                // qi * 4 + 1, 2 3 or 4
                unsigned int ib = qi;
                ib *= (this->computeBackQuads == true ? 8 : 4);

                this->indices.push_back (ib++); // 0
                this->indices.push_back (ib++); // 1
                this->indices.push_back (ib);   // 2

                this->indices.push_back (ib++); // 2
                this->indices.push_back (ib);   // 3
                ib -= 3;
                this->indices.push_back (ib);   // 0

                if (this->computeBackQuads == true) {
                    ib += 4;
                    // Back face
                    this->indices.push_back (ib++); // 0
                    this->indices.push_back (ib++); // 1
                    this->indices.push_back (ib);   // 2

                    this->indices.push_back (ib++); // 2
                    this->indices.push_back (ib);   // 3
                    ib -= 3;
                    this->indices.push_back (ib);   // 0
                }
            }
        }

    private:
        //! The Quads to visualize. This is a vector of 12 values which define 4
        //! coordinates that define boxes (and we'll vis them as triangles). Note that
        //! the coordinates of the locations of the data are the centroids of each
        //! quad.
        const std::vector<std::array<Flt,12>>* quads;

        //! Should additional quads for the 'back' be created, with an opposite normal?
        //! Probably not. Probably I now need to read up about face culling.
        bool computeBackQuads = false;
    };

} // namespace morph
