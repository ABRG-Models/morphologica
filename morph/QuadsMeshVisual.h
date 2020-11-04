#pragma once

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#include <morph/tools.h>
#include <morph/VisualDataModel.h>
#include <morph/MathAlgo.h>
#include <morph/Scale.h>
#include <morph/Vector.h>
#include <iostream>
#include <vector>
#include <array>
#include <stdexcept>

namespace morph {

    template <typename Flt>
    class QuadsMeshVisual : public VisualDataModel<Flt>
    {
    public:
        QuadsMeshVisual(GLuint sp,
                        const std::vector<std::array<Flt,12>>* _quads,
                        const Vector<float> _offset,
                        const std::vector<Flt>* _data,
                        const Scale<Flt>& _scale,
                        ColourMapType _cmt,
                        const float _hue = 0.0f,
                        const float _sat = 1.0f,
                        const float _radius = 0.05f)
        {
            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);
            this->colourScale = _scale;
            this->radius = _radius;

            // How to deal with quads? Each quad has a centroid. The coordinate of the
            // centroid of the quad is the location for the data point. Thus, convert
            // the quad data into centroids and save these in VisualDataModel<>::dataCoords.
            this->quads = _quads;

            // From quads, build dataCoords:
            this->dataCoords = new std::vector<Vector<float>>;
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

            this->initializeVertices();
            this->postVertexInit();
        }

        //! Version with std::array \a _offset
        QuadsMeshVisual(GLuint sp,
                        const std::vector<std::array<Flt,12>>* _quads,
                        const std::array<float, 3> _offset,
                        const std::vector<Flt>* _data,
                        const Scale<Flt>& _scale,
                        ColourMapType _cmt,
                        const float _hue = 0.0f)
        {
            Vector<float> offset_vec;
            offset_vec.set_from(_offset);
            QuadsVisual(sp, _quads, offset_vec, _data, _scale, _cmt, _hue);
        }

        ~QuadsMeshVisual() { delete this->dataCoords; }

        virtual void updateCoords (std::vector<Vector<Flt>>* _coords)
        {
            throw std::runtime_error ("This won't work.");
        }

        //! Initialize the vertices that will represent the Quads.
        void initializeVertices (void)
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
            VBOint ib = 0;

            for (unsigned int qi = 0; qi < nquads; ++qi) {
                // Extract coordinates from this->quads
                Vector<float> q0 = {(*this->quads)[qi][0], (*this->quads)[qi][1], (*this->quads)[qi][2]};
                Vector<float> q1 = {(*this->quads)[qi][3], (*this->quads)[qi][4], (*this->quads)[qi][5]};
                Vector<float> q2 = {(*this->quads)[qi][6], (*this->quads)[qi][7], (*this->quads)[qi][8]};
                Vector<float> q3 = {(*this->quads)[qi][9], (*this->quads)[qi][10], (*this->quads)[qi][11]};
                // Draw a frame from the 4 coordinates
                std::array<float, 3> clr = this->cm.convert(dcopy[qi]);
                this->computeTube (ib, q0, q1, clr, clr, this->radius, this->tseg);
                this->computeTube (ib, q1, q2, clr, clr, this->radius, this->tseg);
                this->computeTube (ib, q2, q3, clr, clr, this->radius, this->tseg);
                this->computeTube (ib, q3, q0, clr, clr, this->radius, this->tseg);
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
