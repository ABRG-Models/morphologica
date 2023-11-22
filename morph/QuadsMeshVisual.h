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
#include <set>
#include <stdexcept>

namespace morph {

    template <typename Flt>
    class QuadsMeshVisual : public VisualDataModel<Flt>
    {
    public:
        QuadsMeshVisual(const std::vector<std::array<Flt,12>>* _quads,
                        const vec<float> _offset,
                        const std::vector<Flt>* _data,
                        const Scale<Flt>& _scale,
                        ColourMapType _cmt,
                        const float _hue = 0.0f,
                        const float _sat = 1.0f,
                        const float _radius = 0.05f)
        {
            // Set up...
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->colourScale = _scale;
            this->radius = _radius;

            // How to deal with quads? Each quad has a centroid. The coordinate of the
            // centroid of the quad is the location for the data point. Thus, convert
            // the quad data into centroids and save these in VisualDataModel<>::dataCoords.
            this->quads = _quads;

            // From quads, build dataCoords:
            this->dataCoords = new std::vector<vec<float>>;
            this->dataCoords->resize (this->quads->size());
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
            if (_cmt == ColourMapType::Fixed) { this->cm.setSat (_sat); }
        }

        //! Version with std::array \a _offset
        QuadsMeshVisual(const std::vector<std::array<Flt,12>>* _quads,
                        const std::array<float, 3> _offset,
                        const std::vector<Flt>* _data,
                        const Scale<Flt>& _scale,
                        ColourMapType _cmt,
                        const float _hue = 0.0f)
        {
            vec<float> offset_vec;
            offset_vec.set_from(_offset);
            QuadsMeshVisual<Flt>(_quads, offset_vec, _data, _scale, _cmt, _hue);
        }

        ~QuadsMeshVisual() { delete this->dataCoords; }

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

            // Index buffer index
            GLuint ib = 0;

            std::set<vec<float, 6>> lastQuadLines;
            std::cout << "nquads: " << nquads << std::endl;
            for (unsigned int qi = 0; qi < nquads; ++qi) {
                // Extract coordinates from this->quads
                vec<float> q0 = {(*this->quads)[qi][0], (*this->quads)[qi][1], (*this->quads)[qi][2]};
                vec<float> q1 = {(*this->quads)[qi][3], (*this->quads)[qi][4], (*this->quads)[qi][5]};
                vec<float> q2 = {(*this->quads)[qi][6], (*this->quads)[qi][7], (*this->quads)[qi][8]};
                vec<float> q3 = {(*this->quads)[qi][9], (*this->quads)[qi][10], (*this->quads)[qi][11]};
                // Draw a frame from the 4 coordinates
                std::array<float, 3> clr = this->cm.convert(dcopy[qi]);

                // Check that previous quad didn't include any of these pairs of points
                vec<float, 6> line0 = { q1[0], q1[1], q1[2], q0[0], q0[1], q0[2] };
                vec<float, 6> rline0 = { q0[0], q0[1], q0[2], q1[0], q1[1], q1[2] };

                vec<float, 6> line1 = { q2[0], q2[1], q2[2], q1[0], q1[1], q1[2] };
                vec<float, 6> rline1 = { q1[0], q1[1], q1[2], q2[0], q2[1], q2[2] };

                vec<float, 6> line2 = { q3[0], q3[1], q3[2], q2[0], q2[1], q2[2] };
                vec<float, 6> rline2 = { q2[0], q2[1], q2[2], q3[0], q3[1], q3[2] };

                vec<float, 6> line3 = { q0[0], q0[1], q0[2], q3[0], q3[1], q3[2] };
                vec<float, 6> rline3 = { q3[0], q3[1], q3[2], q0[0], q0[1], q0[2] };

                if (!lastQuadLines.empty()) {
                    std::cout << "Test and draw...\n";
                    // Test each line for the current quad. If it has already been drawn
                    // by the last quad, omit drawing it again.
                    if (lastQuadLines.count(line0) == 0 && lastQuadLines.count(rline0) == 0) {
                        std::cout << "draw\n";
                        this->computeTube (ib, q0, q1, clr, clr, this->radius, this->tseg);
                    }
                    if (lastQuadLines.count(line1) == 0 && lastQuadLines.count(rline1) == 0) {
                        std::cout << "draw\n";
                        this->computeTube (ib, q1, q2, clr, clr, this->radius, this->tseg);
                    }
                    if (lastQuadLines.count(line2) == 0 && lastQuadLines.count(rline2) == 0) {
                        std::cout << "draw\n";
                        this->computeTube (ib, q2, q3, clr, clr, this->radius, this->tseg);
                    }
                    if (lastQuadLines.count(line3) == 0 && lastQuadLines.count(rline3) == 0) {
                        std::cout << "draw\n";
                        this->computeTube (ib, q3, q0, clr, clr, this->radius, this->tseg);
                    }
                    lastQuadLines.clear();
                } else { // No last quad, so draw all the lines in the current quad
                    std::cout << "Draw all\n";
                    this->computeTube (ib, q0, q1, clr, clr, this->radius, this->tseg);
                    this->computeTube (ib, q1, q2, clr, clr, this->radius, this->tseg);
                    this->computeTube (ib, q2, q3, clr, clr, this->radius, this->tseg);
                    this->computeTube (ib, q3, q0, clr, clr, this->radius, this->tseg);
                }
                // Record the lastQuadLines (and their inverses) for the next loop
                lastQuadLines.insert (line0);
                lastQuadLines.insert (line1);
                lastQuadLines.insert (line2);
                lastQuadLines.insert (line3);
                lastQuadLines.insert (rline0);
                lastQuadLines.insert (rline1);
                lastQuadLines.insert (rline2);
                lastQuadLines.insert (rline3);
            }
            std::cout << "QuadsMeshVisual has " << ib << " vertex indices\n";
        }

    private:
        //! The Quads to visualize. This is a vector of 12 values which define 4
        //! coordinates that define boxes (and we'll vis them as rods). Note that
        //! the coordinates of the locations of the data are the centroids of each
        //! quad.
        const std::vector<std::array<Flt,12>>* quads;

        //! Tube radius
        float radius = 0.05f;
        //! Tube number of segments
        int tseg = 8;
    };

} // namespace morph
