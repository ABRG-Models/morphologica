#ifndef _QUADSVISUAL_H_
#define _QUADSVISUAL_H_

#include "GL3/gl3.h"

#include "tools.h"

#include "VisualModel.h"

#include "MathAlgo.h"

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;
#include <array>
using std::array;

namespace morph {

    //! The template argument Flt is the type of the data which this QuadsVisual will visualize.
    template <class Flt>
    class QuadsVisual : public VisualModel
    {
    public:
        QuadsVisual(GLuint sp,
                    const vector<array<Flt,12>>* _quads,
                    const array<float, 3> _offset,
                    const vector<Flt>* _data,
                    const array<Flt, 2> _scale,
                    ColourMapType _cmt,
                    const float _hue = 0.0f) {

            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);
            this->scale = _scale;
            this->quads = _quads;
            this->data = _data;

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);

            this->initializeVertices();
            this->postVertexInit();
        }

        //! Convert datum using our scale into a colour triplet (RGB).
        array<float, 3> datumToColour (Flt datum_in) {
            // Scale the input...
            Flt datum = datum_in * this->scale[0] + this->scale[1];
            datum = datum > static_cast<Flt>(1.0) ? static_cast<Flt>(1.0) : datum;
            datum = datum < static_cast<Flt>(0.0) ? static_cast<Flt>(0.0) : datum;
            // ...then turn it into a colour:
            return this->cm.convert (datum);
        }

        //! Do the computations to initialize the vertices that will represent the
        //! Quads.
        void initializeVertices (void) {

            unsigned int nquads = this->quads->size();
            unsigned int ndata = this->data->size();

            if (nquads != ndata) {
                cout << "nquads != ndata, return." << endl;
                return;
            }

            vector<Flt> dcopy = *(this->data);
            if (this->scale[0] == 0.0f && this->scale[1] == 0.0f) {
                // Special 0,0 scale means auto scale data
                dcopy = MathAlgo<Flt>::autoscale (dcopy);
                this->scale[0] = 1.0f;
            }

            for (unsigned int qi = 0; qi < nquads; ++qi) {

                /*
                // Scale colour
                Flt datum = dcopy[qi] * this->scale[0] + this->scale[1];
                datum = datum > static_cast<Flt>(1.0) ? static_cast<Flt>(1.0) : datum;
                datum = datum < static_cast<Flt>(0.0) ? static_cast<Flt>(0.0) : datum;
                // And turn it into a colour:
                array<float, 3> clr = morph::Tools::getJetColorF((double)datum);
                */
                array<float, 12> quad = (*this->quads)[qi];
                this->vertex_push (quad[0], quad[1], quad[2], this->vertexPositions);   //1
                this->vertex_push (quad[3], quad[4], quad[5], this->vertexPositions);   //2
                this->vertex_push (quad[6], quad[7], quad[8], this->vertexPositions);   //3
                this->vertex_push (quad[9], quad[10], quad[11], this->vertexPositions); //4

                // All same colours
                //this->vertex_push (clr, this->vertexColors);
                //this->vertex_push (clr, this->vertexColors);
                //this->vertex_push (clr, this->vertexColors);
                //this->vertex_push (clr, this->vertexColors);
                this->vertex_push (this->datumToColour(dcopy[qi]), this->vertexColors);
                this->vertex_push (this->datumToColour(dcopy[qi]), this->vertexColors);
                this->vertex_push (this->datumToColour(dcopy[qi]), this->vertexColors);
                this->vertex_push (this->datumToColour(dcopy[qi]), this->vertexColors);

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

        //! Update the data and re-compute the vertices.
        void updateData (const vector<Flt>* _data, const array<Flt, 2> _scale) {
            this->scale = _scale;
            this->data = _data;
            // Fixme: Better not to clear, then repeatedly pushback here:
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->initializeVertices();
            // Now re-set up the VBOs
            int sz = this->indices.size() * sizeof(VBOint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, colLoc);
        }

        /*!
         * The relevant colour map. Change the type/hue of this colour map object to
         * generate different types of map.
         */
        ColourMap<Flt> cm;

        //! The linear scaling for the colour is y1 = m1 x + c1 (m1 = scale[0] and c1 =
        //! scale[1]) If all entries of scale are static_cast<Flt>(0), then auto-scale.
        array<Flt, 2> scale;

    private:
        //! The Quads to visualize. This is a vector of 12 values which define 4
        //! coordinates that define boxes (and we'll vis them as triangles)
        const vector<array<Flt,12>>* quads;

        //! The data to visualize as colour (modulated by the linear scaling
        //! provided in this->scale)
        const vector<Flt>* data;
    };

} // namespace morph

#endif // _QUADSVISUAL_H_
