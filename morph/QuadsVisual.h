#pragma once

#include "GL3/gl3.h"
#include "tools.h"
#include "VisualDataModel.h"
#include "MathAlgo.h"
#include "Scale.h"
using morph::Scale;

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;
#include <array>
using std::array;
#include <stdexcept>
using std::runtime_error;

namespace morph {

    template <typename Flt>
    class QuadsVisual : public VisualDataModel<Flt>
    {
    public:
        QuadsVisual(GLuint sp,
                    const vector<array<Flt,12>>* _quads,
                    const array<float, 3> _offset,
                    const vector<Flt>* _data,
                    const Scale<Flt>& _scale,
                    ColourMapType _cmt,
                    const float _hue = 0.0f) {

            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);
            this->colourScale = _scale;

            // How to deal with quads? Each quad has a centroid. The coordinate of the
            // centroid of the quad is the location for the data point. Thus, convert
            // the quad data into centroids and save these in VisualDataModel<>::dataCoords.
            this->quads = _quads;

            // From quads, build dataCoords:
            this->dataCoords = new vector<array<float, 3>>;
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

            this->initializeVertices();
            this->postVertexInit();
        }

        ~QuadsVisual() {
            delete this->dataCoords;
        }

        virtual void updateCoords (vector<array<Flt, 3>>* _coords) {
            throw runtime_error ("This won't work.");
        }

        //! Initialize the vertices that will represent the Quads.
        void initializeVertices (void) {

            unsigned int nquads = this->quads->size();
            unsigned int ndata = this->scalarData->size();

            if (nquads != ndata) {
                cout << "nquads != ndata, return." << endl;
                return;
            }

            vector<Flt> dcopy = *(this->scalarData);
            this->colourScale.do_autoscale = true;
            this->colourScale.transform ((*this->scalarData), dcopy);

            for (unsigned int qi = 0; qi < nquads; ++qi) {

                array<float, 12> quad = (*this->quads)[qi];
                this->vertex_push (quad[0], quad[1], quad[2], this->vertexPositions);   //1
                this->vertex_push (quad[3], quad[4], quad[5], this->vertexPositions);   //2
                this->vertex_push (quad[6], quad[7], quad[8], this->vertexPositions);   //3
                this->vertex_push (quad[9], quad[10], quad[11], this->vertexPositions); //4

                // All same colours
                array<float, 3> clr = this->cm.convert(dcopy[qi]);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);

                // All same normals
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);

                // Two triangles per quad
                // qi * 4 + 1, 2 3 or 4
                unsigned int ib = qi*4;
                this->indices.push_back (ib++); // 0
                this->indices.push_back (ib++); // 1
                this->indices.push_back (ib); // 2

                this->indices.push_back (ib++); // 2
                this->indices.push_back (ib);   // 3
                ib -= 3;
                this->indices.push_back (ib);   // 0
            }
        }

    private:
        //! The Quads to visualize. This is a vector of 12 values which define 4
        //! coordinates that define boxes (and we'll vis them as triangles). Note that
        //! the coordinates of the locations of the data are the centroids of each
        //! quad.
        const vector<array<Flt,12>>* quads;
    };

} // namespace morph
