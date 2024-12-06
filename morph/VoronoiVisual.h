/*!
 * Visualize an arbitrary surface defined by values at points in 3D space (similar to
 * ScatterVisual). Using code from https://github.com/JCash/voronoi compute a 2D Voronoi
 * diagram around the data points (using their x/y values) to create the 'panels' to
 * colourize. interpolate the z values of the data points to determine the z values of
 * the edges in the Voronoi diagram. Colourize the panels based on the scalarData or
 * vectorData of the underlying VisualDataModel.
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

    //! The template argument F is the type of the data which this VoronoiVisual
    //! will visualize.
    template <typename F, int glver = morph::gl::version_4_1>
    class VoronoiVisual : public VisualDataModel<F, glver>
    {
    public:
        VoronoiVisual (const vec<float> _offset)
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
                std::cout << "VoronoiVisual Error: ncoords ("<<ncoords<<") != ndata ("<<ndata<<"), return (no model)." << std::endl;
                return;
            }
            if (nvdata > 0 && ncoords != nvdata) {
                std::cout << "VoronoiVisual Error: ncoords ("<<ncoords<<") != nvdata ("<<nvdata<<"), return (no model)." << std::endl;
                return;
            }

            this->setupScaling (this->scalarData->size());

            // Compute 2.5D Voronoi diagram using code adapted from
            // https://github.com/JCash/voronoi. The adaptation is to add a third
            // dimension to jcv_point.

            // FIrst make a vector of jcv_point objects as input
            std::vector<jcv_point> coords2d (ncoords);
            for (unsigned int i = 0; i < ncoords; ++i) {
                coords2d[i] = { (*this->dataCoords)[i][0], (*this->dataCoords)[i][1],  (*this->dataCoords)[i][2] };
            }

            // Generate the 2D Voronoi diagram
            jcv_diagram diagram;
            memset (&diagram, 0, sizeof(jcv_diagram));
            jcv_rect domain = {jcv_point{-1,-1,0}, jcv_point{2,2,0}};
            jcv_diagram_generate (ncoords, coords2d.data(), &domain, 0, &diagram);

            // We obtain access the the Voronoi cell sites:
            const jcv_site* sites = jcv_diagram_get_sites (&diagram);

            // Need a vec comparison function for a set of morph::vec. See:
            // https://abrg-models.github.io/morphologica/ref/coremaths/vvec/#using-morphvvec-as-a-key-in-stdmap-or-within-an-stdset
            auto _veccmp = [](morph::vec<float> a, morph::vec<float> b) { return a.lexical_lessthan(b); };

            // Now scan through the Voronoi cell 'sites' and 'edges' to re-assign z
            // values in the edges. This is not going to be particularly efficient, but
            // it will be fine for grids of the size that we generally visualize.
            //
            // For each site, we have a set of edges around that site. We examine each
            // edge in turn, considering the sites that cluster around each end of the
            // edge. We assign the mean z of the sites in the cluster to the edge's
            // postion at that end of the edge.
            for (int i = 0; i < diagram.numsites; ++i) {

                // We have the current edge_1, the next edge_2 and the previous edge_0
                const jcv_site* site = &sites[i];
                jcv_graphedge* edge_first = site->edges; // The very first edge
                jcv_graphedge* edge_1 = edge_first;
                jcv_graphedge* edge_2 = edge_first;
                jcv_graphedge* edge_0 = edge_first;
                while (edge_0->next) { edge_0 = edge_0->next; }

                while (edge_1) {

                    std::set<morph::vec<float>, decltype(_veccmp)> cellcentres_1 (_veccmp);
                    std::set<morph::vec<float>, decltype(_veccmp)> cellcentres_0 (_veccmp);

                    edge_2 = edge_1->next ? edge_1->next : edge_first;
                    // edge_0 already set

                    // populate cellcentres
                    for (unsigned int j = 0; j < 2; ++j) {
                        if (edge_1->edge->sites[j]) {
                            // Both cellcentres get edge_1 sites
                            cellcentres_1.insert ({edge_1->edge->sites[j]->p.x, edge_1->edge->sites[j]->p.y, edge_1->edge->sites[j]->p.z});
                            cellcentres_0.insert ({edge_1->edge->sites[j]->p.x, edge_1->edge->sites[j]->p.y, edge_1->edge->sites[j]->p.z});
                        }
                        // By definition, cellcentres_1 also gets edge_2 sites...
                        if (edge_2->edge->sites[j]) {
                            cellcentres_1.insert ({edge_2->edge->sites[j]->p.x, edge_2->edge->sites[j]->p.y, edge_2->edge->sites[j]->p.z});
                        }
                        // and cellcentres_0 gets edge_0 sites
                        if (edge_0->edge->sites[j]) {
                            cellcentres_0.insert ({edge_0->edge->sites[j]->p.x, edge_0->edge->sites[j]->p.y, edge_0->edge->sites[j]->p.z});
                        }
                    }

                    // Find the mean of the cell centres associated with edge_1 and edge_2
                    float zsum_1= 0.0f;
                    morph::vec<float, 2> mean_cc_1 = {0.0f};
                    for (auto cce : cellcentres_1) {
                        zsum_1 += cce[2];
                        mean_cc_1 += cce.less_one_dim();
                    }

                    // Find mean associated with the other end of edge_1 - the cell centres associated with edge_1 and edge_0
                    float zsum_0 = 0.0f;
                    morph::vec<float, 2> mean_cc_0 = {0.0f};
                    for (auto cce : cellcentres_0) {
                        zsum_0 += cce[2];
                        mean_cc_0 += cce.less_one_dim();
                    }

                    edge_1->pos[1].z = (zsum_1 / cellcentres_1.size());
                    edge_1->pos[0].z = (zsum_0 / cellcentres_0.size());

                    edge_0 = edge_1;
                    edge_1 = edge_1->next;
                }
            } // finished reassignment of z values

            // To draw triangles iterate over the 'sites' and get the edges
            // Note: The order of sites in the jcv_diagram is *not* same as original coordinate order...
            for (int i = 0; i < diagram.numsites; ++i) {
                const jcv_site* site = &sites[i];
                const jcv_graphedge* e = site->edges;
                while (e) {
                    morph::vec<float> t0 = { site->p.x, site->p.y, site->p.z };
                    morph::vec<float> t1 = { e->pos[0].x, e->pos[0].y, e->pos[0].z };
                    morph::vec<float> t2 = { e->pos[1].x, e->pos[1].y, e->pos[1].z };
                    // ...but site->index is the index into the original data 3 indices.
                    // NB: There are 3 each of pos/col/norm vertices (and 3 indices) per
                    // triangle. Could be reduced in principle. For a random map, it
                    // comes out as about about 17*4 vertices per coordinate.
                    this->computeTriangle (t0, t1, t2, this->setColour(site->index));
                    e = e->next;
                }
            }

            if (this->debug_edges) {
                // Now scan through the edges drawing tubes for debug
                for (int i = 0; i < diagram.numsites; ++i) {
                    const jcv_site* site = &sites[i];
                    const jcv_graphedge* e = site->edges;
                    while (e) {
                        this->computeTube ({ e->pos[0].x, e->pos[0].y, e->pos[0].z }, { e->pos[1].x, e->pos[1].y, e->pos[1].z },
                                           morph::colour::royalblue, morph::colour::goldenrod2, 0.01f, 6);
                        e = e->next;
                    }
                }
            }

            if (this->show_voronoi2d) {
                // Show the 2D Voronoi diagram's edges at z=0
                for (int i = 0; i < diagram.numsites; ++i) {
                    const jcv_site* site = &sites[i];
                    const jcv_graphedge* e = site->edges;
                    while (e) {
                        this->computeTube ({ e->pos[0].x, e->pos[0].y, 0.0f }, { e->pos[1].x, e->pos[1].y, 0.0f },
                                           morph::colour::black, morph::colour::black, 0.01f, 6);
                        e = e->next;
                    }
                }
            }

            // At end free
            jcv_diagram_free (&diagram);

            if (this->debug_dataCoords) {
                // Add some spheres at the original data points for debugging
                for (unsigned int i = 0; i < ncoords; ++i) {
                    this->computeSphere ((*this->dataCoords)[i], morph::colour::black, 0.03f);
                }
            }
        }

        //! A copy of the scalarData which can be transformed suitably to be the z value of the surface
        std::vector<float> dcopy;
        //! A copy of the scalarData (or first field of vectorData), scaled to be a colour value
        std::vector<float> dcolour;
        std::vector<float> dcolour2;
        std::vector<float> dcolour3;

        //! If true, show 2.5D Voronoi edges
        bool debug_edges = false;
        //! If true, show 2D Voronoi edges
        bool show_voronoi2d = false;
        //! If true, show black spheres at dataCoord locations
        bool debug_dataCoords = false;

        // Do we add index labels?
        bool labelIndices = false;
        morph::vec<float, 3> labelOffset = { 0.04f, 0.0f, 0.0f };
        float labelSize = 0.03f;
    };

} // namespace morph
