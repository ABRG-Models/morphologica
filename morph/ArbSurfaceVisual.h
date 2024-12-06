/*!
 * Visualize an arbitrary surface defined by values at points in 3D space (similar to
 * ScatterVisual). Compute a 2.5D Delaunay triangulization around the data points to create the
 * 'panels' to colourize. An assumption is that the z value of the data coordinates can be 'set
 * aside' and then a 2D Delaunay triangulation applied to the (x,y) coordinates.
 *
 * \author Seb James
 * \date 2024
 */
#pragma once

#include <morph/tools.h>
#include <morph/VisualDataModel.h>
#include <morph/scale.h>
#include <morph/vec.h>
#include <iostream>
#include <vector>
#include <array>

#define JC_VORONOI_IMPLEMENTATION
#include <morph/jcvoronoi/jc_voronoi.h>

namespace morph {

    //! The template argument F is the type of the data which this ArbSurfaceVisual
    //! will visualize.
    template <typename F, int glver = morph::gl::version_4_1>
    class ArbSurfaceVisual : public VisualDataModel<F, glver>
    {
    public:
        ArbSurfaceVisual (const vec<float> _offset)
        {
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->zScale.setParams (1, 0);
            this->colourScale.do_autoscale = true;
        }

        void setupScaling (size_t n)
        {
            if (this->scalarData != nullptr) {
                // Check scalar data has same size as Grid
                if (this->scalarData->size() != n) {
                    throw std::runtime_error ("Error: scalarData size does not match n");
                }

                this->dcopy.resize (n);
                this->zScale.transform (*(this->scalarData), dcopy);
                this->dcolour.resize (n);
                this->colourScale.transform (*(this->scalarData), dcolour);

            } else if (this->vectorData != nullptr) {

                // Check vector data
                if (this->vectorData->size() != n) {
                    throw std::runtime_error ("Error: size does not match vectorData size");
                }

                this->dcopy.resize (n);
                this->dcolour.resize (n);
                this->dcolour2.resize (n);
                this->dcolour3.resize (n);
                std::vector<float> veclens(dcopy);
                for (unsigned int i = 0; i < n; ++i) {
                    veclens[i] = (*this->vectorData)[i].length();
                    this->dcolour[i] = (*this->vectorData)[i][0];
                    this->dcolour2[i] = (*this->vectorData)[i][1];
                    // Could also extract a third colour for Trichrome vs Duochrome (or for raw RGB signal)
                    this->dcolour3[i] = (*this->vectorData)[i][2];
                }
                this->zScale.transform (veclens, this->dcopy);

                // Handle case where this->cm.getType() == morph::ColourMapType::RGB and there is
                // exactly one colour. ColourMapType::RGB assumes R/G/B data all in range 0->1
                // ALREADY and therefore they don't need to be re-scaled with this->colourScale.
                if (this->cm.getType() != morph::ColourMapType::RGB) {
                    this->colourScale.transform (this->dcolour, this->dcolour);
                    // Dual axis colour maps like Duochrome and HSV will need to use colourScale2 to
                    // transform their second colour/axis,
                    this->colourScale2.transform (this->dcolour2, this->dcolour2);
                    // Similarly for Triple axis maps
                    this->colourScale3.transform (this->dcolour3, this->dcolour3);
                } // else assume dcolour/dcolour2/dcolour3 are all in range 0->1 (or 0-255) already
            }
        }

        //! An overridable function to set the colour of rect ri
        std::array<float, 3> setColour (size_t ri)
        {
            std::array<float, 3> clr = { 0.0f, 0.0f, 0.0f };
            if (this->cm.numDatums() == 3) {
                if constexpr (std::is_integral<std::decay_t<F>>::value) {
                    // Differs from above as we divide by 255 to get value in range 0-1
                    clr = this->cm.convert (this->dcolour[ri]/255.0f, this->dcolour2[ri]/255.0f, this->dcolour3[ri]/255.0f);
                } else {
                    clr = this->cm.convert (this->dcolour[ri], this->dcolour2[ri], this->dcolour3[ri]);
                }
            } else if (this->cm.numDatums() == 2) {
                // Use vectorData
                clr = this->cm.convert (this->dcolour[ri], this->dcolour2[ri]);
            } else {
                clr = this->cm.convert (this->dcolour[ri]);
            }
            return clr;
        }

        //! Compute a triangle from 3 arbitrary corners
        void computeTriangle (vec<float> c1, vec<float> c2, vec<float> c3, std::array<float, 3> colr)
        {
            // v is the face normal
            vec<float> u1 = c1-c2;
            vec<float> u2 = c2-c3;
            vec<float> v = u1.cross(u2);
            v.renormalize();
            // Push corner vertices
            this->vertex_push (c1, this->vertexPositions);
            this->vertex_push (c2, this->vertexPositions);
            this->vertex_push (c3, this->vertexPositions);
            // Colours/normals
            for (unsigned int i = 0; i < 3U; ++i) {
                this->vertex_push (colr, this->vertexColors);
                this->vertex_push (v, this->vertexNormals);
            }
            this->indices.push_back (this->idx++);
            this->indices.push_back (this->idx++);
            this->indices.push_back (this->idx++);
        }

        //! Compute the triangulization
        void initializeVertices()
        {
            unsigned int ncoords = this->dataCoords == nullptr ? 0 : this->dataCoords->size();
            if (ncoords == 0) { return; }
            unsigned int ndata = this->scalarData == nullptr ? 0 : this->scalarData->size();
            // If we have vector data, then manipulate colour accordingly.
            unsigned int nvdata = this->vectorData == nullptr ? 0 : this->vectorData->size();

            if (ndata > 0 && ncoords != ndata) {
                std::cout << "ArbSurfaceVisual Error: ncoords ("<<ncoords<<") != ndata ("<<ndata<<"), return (no model)." << std::endl;
                return;
            }
            if (nvdata > 0 && ncoords != nvdata) {
                std::cout << "ArbSurfaceVisual Error: ncoords ("<<ncoords<<") != nvdata ("<<nvdata<<"), return (no model)." << std::endl;
                return;
            }

            this->setupScaling (this->scalarData->size());

            // Compute 2.5D Voronoi diagram
            std::vector<jcv_point> coords2d (ncoords);
            for (unsigned int i = 0; i < ncoords; ++i) {
                coords2d[i] = { (*this->dataCoords)[i][0], (*this->dataCoords)[i][1],  (*this->dataCoords)[i][2] };
            }
            jcv_diagram diagram;
            memset (&diagram, 0, sizeof(jcv_diagram));
            jcv_diagram_generate (ncoords, coords2d.data(), 0, 0, &diagram);

            // To draw triangles iterate over the 'sites' and get the edges
            const jcv_site* sites = jcv_diagram_get_sites (&diagram);
            // Note: The order of sites in the jcv_diagram is *not* same as original coordinate order...
            for (int i = 0; i < diagram.numsites; ++i) {
                const jcv_site* site = &sites[i];
                const jcv_graphedge* e = site->edges;
                while (e) {
                    morph::vec<float> t0 = { site->p.x, site->p.y, site->p.z };
                    morph::vec<float> t1 = { e->pos[0].x, e->pos[0].y, e->pos[0].z };
                    morph::vec<float> t2 = { e->pos[1].x, e->pos[1].y, e->pos[1].z };
                    // ...but site->index is the index into the original data
                    this->computeTriangle (t0.as_float(), t1.as_float(), t2.as_float(), this->setColour(site->index));
                    e = e->next;
                }
            }

            // Draw edges for debug
#if 0
            const jcv_edge* edge = jcv_diagram_get_edges (&diagram);
            while (edge) {
                morph::vec<float> ep0 = { edge->pos[0].x, edge->pos[0].y, edge->pos[0].z };
                morph::vec<float> ep1 = { edge->pos[1].x, edge->pos[1].y, edge->pos[1].z };
                this->computeTube (ep0, ep1, morph::colour::springgreen, morph::colour::crimson, 0.02f, 6);
                edge = jcv_diagram_get_next_edge(edge);
            }
#else
            // Draw half edges of one site for debug
            for (int i = 0; i < diagram.numsites; ++i) {
                const jcv_site* site = &sites[i];
                if (site->index == 4) { // our central site
                    const jcv_graphedge* edge = site->edges;
                    while (edge) {

                        morph::vec<float> ep0 = { edge->pos[0].x, edge->pos[0].y, edge->pos[0].z };
                        morph::vec<float> ep1 = { edge->pos[1].x, edge->pos[1].y, edge->pos[1].z };

                        std::cout << "This edge ["<<ep0<<"->"<<ep1<<"] is between coord " << edge->edge->sites[0]->index << " at ("
                                  << edge->edge->sites[0]->p.x << "," << edge->edge->sites[0]->p.y << "," << edge->edge->sites[0]->p.z
                                  << ") and coord " << edge->edge->sites[1]->index << " at ("
                                  << edge->edge->sites[1]->p.x << "," << edge->edge->sites[1]->p.y << "," << edge->edge->sites[1]->p.z << ")\n";

                        this->computeTube (ep0, ep1, morph::colour::springgreen, morph::colour::crimson, 0.02f, 6);
                        edge = edge->next;
                    }
                }
            }
#endif


            // At end free
            jcv_diagram_free( &diagram );

            // Add some spheres at the original data points for debugging
            for (unsigned int i = 0; i < ncoords; ++i) {
                this->computeSphere ((*this->dataCoords)[i], morph::colour::black, 0.03f);
            }
        }

        //! A copy of the scalarData which can be transformed suitably to be the z value of the surface
        std::vector<float> dcopy;
        //! A copy of the scalarData (or first field of vectorData), scaled to be a colour value
        std::vector<float> dcolour;
        std::vector<float> dcolour2;
        std::vector<float> dcolour3;

        // Do we add index labels?
        bool labelIndices = false;
        morph::vec<float, 3> labelOffset = { 0.04f, 0.0f, 0.0f };
        float labelSize = 0.03f;
    };

} // namespace morph
