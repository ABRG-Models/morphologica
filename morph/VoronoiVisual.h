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
#include <chrono>

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

        //! Compute 2.5D Voronoi diagram using code adapted from
        // https://github.com/JCash/voronoi. The adaptation is to add a third dimension
        // to jcv_point.
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

            this->setupScaling (ncoords); // should be same size as nvdata or data

            morph::quaternion<float> rq;
            if (this->data_z_direction != this->uz) {
                // Compute a rotation but for now:
                this->dcoords.resize (ncoords);
                morph::vec<float> r_axis = this->data_z_direction.cross (this->uz);
                float r_angle = this->data_z_direction.angle (this->uz);
                rq.rotate(r_axis, r_angle);
                for (size_t i = 0; i < ncoords; ++i) {
                    this->dcoords[i] = rq * (*this->dataCoords)[i];
                }
                this->dcoords_ptr = &this->dcoords;
            } else {
                this->dcoords_ptr = this->dataCoords;
            }

            // Use morph::range to find the extents of dataCoords. From these create a
            // rectangle to pass to jcv_diagram_generate.
            morph::range<float> rx, ry;
            rx.search_init();
            ry.search_init();
            for (unsigned int i = 0; i < ncoords; ++i) {
                rx.update ((*this->dcoords_ptr)[i][0]);
                ry.update ((*this->dcoords_ptr)[i][1]);
            }

            using namespace std::chrono;
            using sc = std::chrono::steady_clock;

            sc::time_point t0 = sc::now();
            // Generate the 2D Voronoi diagram
            jcv_diagram diagram;
            memset (&diagram, 0, sizeof(jcv_diagram));

            jcv_rect domain = {
                jcv_point{rx.min - this->border_width, ry.min - this->border_width, 0.0f},
                jcv_point{rx.max + this->border_width, ry.max + this->border_width, 0.0f}
            };
            jcv_diagram_generate (ncoords, this->dcoords_ptr->data(), &domain, 0, &diagram);

            // Time jcv_diagram_generate:
            sc::time_point t1 = sc::now();

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

                    // This is 'voronoi cell centres that are clustered around the 1st end of the edge'
                    std::set<morph::vec<float>, decltype(_veccmp)> cellcentres_1 (_veccmp);
                    // This is 'voronoi cell centres that are clustered around the 0th end of the edge'
                    std::set<morph::vec<float>, decltype(_veccmp)> cellcentres_0 (_veccmp);

                    edge_2 = edge_1->next ? edge_1->next : edge_first;
                    // edge_0 already set

                    bool had_all_sites = true; // used only if debug_border_edge_sites == true

                    // populate cellcentres. Known issue: In some cases, outer edges
                    // have only 1 site at 1 of their ends. This makes it a problem to
                    // compute the correct z value for the end with no site. The
                    // solution would be to modify the jcvoronoi algorithm to populate
                    // both ends of all edges with additional logic.
                    if constexpr (debug_border_edge_sites == false) {
                        for (unsigned int j = 0; j < 2; ++j) {
                            if (edge_1->edge->sites[j]) {
                                cellcentres_1.insert (edge_1->edge->sites[j]->p);
                                cellcentres_0.insert (edge_1->edge->sites[j]->p);
                            }
                            // By definition, cellcentres_1 also gets edge_2 sites...
                            if (edge_2->edge->sites[j]) { cellcentres_1.insert (edge_2->edge->sites[j]->p); }
                            // and cellcentres_0 gets edge_0 sites
                            if (edge_0->edge->sites[j]) { cellcentres_0.insert (edge_0->edge->sites[j]->p); }
                        }
                    } else {
                        for (unsigned int j = 0; j < 2; ++j) {
                            std::cout << "j = " << j << "..." << std::endl;
                            if (edge_1->edge->sites[j]) {
                                std::cout << "Both cellcentres get edge_1 sites. Insert " << edge_1->edge->sites[j]->p << " to _1 and _0\n";
                                cellcentres_1.insert (edge_1->edge->sites[j]->p);
                                cellcentres_0.insert (edge_1->edge->sites[j]->p);
                            } else {
                                std::cout << "Edge_1 from " << edge_1->edge->pos[0] << " to " << edge_1->edge->pos[1] << " has no sites["<<j<<"]\n";
                                had_all_sites = false;
                            }
                            // By definition, cellcentres_1 also gets edge_2 sites...
                            if (edge_2->edge->sites[j]) {
                                std::cout << "By definition, cellcentres_1 also gets edge_2 sites. Insert " << edge_2->edge->sites[j]->p << "\n";
                                cellcentres_1.insert (edge_2->edge->sites[j]->p);
                            } else {
                                std::cout << "Edge_2 from " << edge_2->edge->pos[0] << " to " << edge_2->edge->pos[1] << " has no sites["<<j<<"]\n";
                                had_all_sites = false;
                            }
                            // and cellcentres_0 gets edge_0 sites
                            if (edge_0->edge->sites[j]) {
                                std::cout << "By definition, cellcentres_0 also gets edge_0 sites. Insert " << edge_0->edge->sites[j]->p << "\n";
                                cellcentres_0.insert (edge_0->edge->sites[j]->p);
                            } else {
                                std::cout << "Edge_0 from " << edge_0->edge->pos[0] << " to " << edge_0->edge->pos[1] << " has no sites["<<j<<"]\n";
                                had_all_sites = false;
                            }
                        }
                    }
                    // Find the mean of the cell centres associated with edge_1 and edge_2
                    float zsum_1 = 0.0f;
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

                    if constexpr (debug_border_edge_sites == false) {
                        edge_1->pos[1][2] = (zsum_1 / cellcentres_1.size());
                        edge_1->pos[0][2] = (zsum_0 / cellcentres_0.size());
                    } else {
                        // Can't set edge 1 z positions, if we had a no sites situation above
                        if (had_all_sites) {
                            std::cout << "Edge 1 pos 1 at " << edge_1->pos[1] << " height is zsum_1/cellcentres_1.size() = "
                                      << zsum_1<<"/"<<cellcentres_1.size() << " = " << (zsum_1 / cellcentres_1.size()) << std::endl;
                            edge_1->pos[1][2] = (zsum_1 / cellcentres_1.size());
                            std::cout << "Edge 1 pos 0 at " << edge_1->pos[0] << "height is zsum_0/cellcentres_0.size() = "
                                      << zsum_1<<"/"<<cellcentres_1.size()<< " = " << (zsum_0 / cellcentres_0.size()) << std::endl;

                            edge_1->pos[0][2] = (zsum_0 / cellcentres_0.size());
                        } else {
                            // Will need to do more work to fine correct z
                            if (edge_1->neighbor) {
                                std::cout << "Need more work for edge_1 ("
                                          << edge_1->pos[0] << "--" << edge_1->pos[1] << "), neighbor site: (" << edge_1->neighbor->p << ")\n";
                            } else {
                                std::cout << "Need more work for edge_1 ("
                                          << edge_1->pos[0] << "--" << edge_1->pos[1] << ") which has no neighbor site\n";
                            }
                        }
                    }

                    // Prepare for next loop
                    edge_0 = edge_1;
                    edge_1 = edge_1->next;
                }
            } // finished reassignment of z values
            sc::time_point t1r = sc::now();

            // To draw triangles iterate over the 'sites' and get the edges
            this->triangle_counts.resize (ncoords, 0);
            this->site_indices.resize (ncoords, 0);
            this->triangle_count_sum = 0;

            if (this->data_z_direction != this->uz) {
                // inverse rotate
                morph::vec<float> t0 = {0.0f};
                morph::vec<float> t1 = {0.0f};
                morph::vec<float> t2 = {0.0f};
                morph::quaternion<float> rqinv = rq.invert();
                for (int i = 0; i < diagram.numsites; ++i) {
                    const jcv_site* site = &sites[i];
                    const jcv_graphedge* e = site->edges;
                    unsigned int site_triangles = 0;
                    while (e) {
                        // NB: There are 3 each of pos/col/norm vertices (and 3 indices) per
                        // triangle. Could be reduced in principle. For a random map, it
                        // comes out as about about 17*4 vertices per coordinate.
                        t0 = rqinv * site->p;
                        t1 = rqinv * e->pos[0];
                        t2 = rqinv * e->pos[1];
                        this->computeTriangle (t0, t1, t2, this->setColour(site->index));
                        ++site_triangles;
                        e = e->next;
                    }
                    //std::cout << "Voronoi cell " << i << " had " << site_triangles << " triangles\n";
                    this->triangle_counts[i] = site_triangles;
                    this->site_indices[i] = site->index;
                    this->triangle_count_sum += site_triangles;
                }
            } else {
                // sc1 and site_triangles_total used if debug_border_edge_sites == true only
                morph::scale<float> sc1;
                unsigned int site_triangles_total = 0;
                if constexpr (debug_border_edge_sites == true) { sc1.compute_scaling (0, 25); }
                // No need to inverse rotate
                for (int i = 0; i < diagram.numsites; ++i) {
                    const jcv_site* site = &sites[i];
                    const jcv_graphedge* e = site->edges;
                    unsigned int site_triangles = 0;
                    while (e) {
                        if constexpr (debug_border_edge_sites == false) {
                            this->computeTriangle (site->p, e->pos[0], e->pos[1], this->setColour(site->index));
                        } else {
                            // Colour by site index (for first 25)
                            this->computeTriangle (site->p, e->pos[0], e->pos[1], this->cm.convert (sc1.transform_one (site_triangles_total)));
                            ++site_triangles_total;
                        }
                        ++site_triangles;
                        e = e->next;
                    }
                    this->triangle_counts[i] = site_triangles;
                    this->site_indices[i] = site->index;
                    this->triangle_count_sum += site_triangles;
                }
            }
            if (static_cast<unsigned int>(diagram.numsites) != ncoords) {
                std::cout << "WARNING: numsites != ncoords ?!?!\n";
            }
            sc::time_point t1t = sc::now(); // time draw triangles

            // Draw optional objects
            if (this->debug_edges) {
                // Now scan through the edges drawing tubes for debug
                for (int i = 0; i < diagram.numsites; ++i) {
                    const jcv_site* site = &sites[i];
                    const jcv_graphedge* e = site->edges;
                    while (e) {
                        this->computeTube (e->pos[0], e->pos[1], morph::colour::royalblue, morph::colour::goldenrod2, 0.01f, 6);
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
                        this->computeTube ({ e->pos[0].x(), e->pos[0].y(), 0.0f }, { e->pos[1].x(), e->pos[1].y(), 0.0f },
                                           morph::colour::black, morph::colour::black, 0.01f, 6);
                        e = e->next;
                    }
                }
            }

            if (this->debug_dataCoords) {
                // Add some spheres at the original data points for debugging
                for (unsigned int i = 0; i < ncoords; ++i) {
                    this->computeSphere ((*this->dataCoords)[i], morph::colour::black, 0.03f);
                }
            }
            sc::time_point t1o = sc::now(); // time draw extra objects

            // At end free the Voronoi diagram memory
            jcv_diagram_free (&diagram);

            // Time the rest
            sc::time_point t2 = sc::now();

            sc::duration t_d = t1 - t0;
            std::cout << "jcv_diagram_generate: " << duration_cast<microseconds>(t_d).count() << " us\n";
            sc::duration t_d1 = t1r - t1;
            std::cout << "regenerate z values: " << duration_cast<microseconds>(t_d1).count() << " us\n";
            sc::duration t_d1t = t1t - t1r;
            std::cout << "Draw triangles: " << duration_cast<microseconds>(t_d1t).count() << " us\n";
            sc::duration t_d1o = t1o - t1t;
            std::cout << "Draw extra objects: " << duration_cast<microseconds>(t_d1o).count() << " us\n";

            sc::duration t_d2 = t2 - t0;
            std::cout << "Everything: " << duration_cast<microseconds>(t_d2).count() << " us\n";
        }

        void reinitColoursScalar()
        {
            if (this->colourScale.do_autoscale == true) { this->colourScale.reset(); }
            this->dcolour.resize (this->scalarData->size());
            this->colourScale.transform (*(this->scalarData), dcolour);

            // Replace elements of vertexColors
            unsigned int tcounts = 0;
            for (std::size_t i = 0u; i < this->triangle_counts.size(); ++i) {
                auto c = this->cm.convert (this->dcolour[this->site_indices[i]]);
                std::size_t d_idx = tcounts * 9; // 3 floats per vtx, 3 vtxs per tri
                for (std::size_t j = 0; j < 3 * this->triangle_counts[i]; ++j) {
                    // This is ONE colour vertex. Need 3 per triangle.
                    this->vertexColors[d_idx + 3 * j]     = c[0];
                    this->vertexColors[d_idx + 3 * j + 1] = c[1];
                    this->vertexColors[d_idx + 3 * j + 2] = c[2];
                }
                tcounts += this->triangle_counts[i];
            }

            // Lastly, this call copies vertexColors (etc) into the OpenGL memory space
            this->reinit_colour_buffer();
        }

        //! Called by reinitColours when vectorData is not null (vectors are probably RGB colour)
        void reinitColoursVector()
        {
            if (this->colourScale.do_autoscale == true) { this->colourScale.reset(); }

            for (unsigned int i = 0; i < this->vectorData->size(); ++i) {
                this->dcolour[i] = (*this->vectorData)[i][0];
                this->dcolour2[i] = (*this->vectorData)[i][1];
                this->dcolour3[i] = (*this->vectorData)[i][2];
            }
            if (this->cm.getType() != morph::ColourMapType::RGB) {
                this->colourScale.transform (this->dcolour, this->dcolour);
                this->colourScale2.transform (this->dcolour2, this->dcolour2);
                this->colourScale3.transform (this->dcolour3, this->dcolour3);
            } // else assume dcolour/dcolour2/dcolour3 are all in range 0->1 (or 0-255) already

            // Replace elements of vertexColors
            unsigned int tcounts = 0;
            for (std::size_t i = 0u; i < this->triangle_counts.size(); ++i) {
                std::array<float, 3> c = this->setColour (this->site_indices[i]);
                std::size_t d_idx = tcounts * 9; // 3 floats per vtx, 3 vtxs per tri
                for (std::size_t j = 0; j < 3 * this->triangle_counts[i]; ++j) {
                    // This is ONE colour vertex. Need 3 per triangle.
                    this->vertexColors[d_idx + 3 * j]     = c[0];
                    this->vertexColors[d_idx + 3 * j + 1] = c[1];
                    this->vertexColors[d_idx + 3 * j + 2] = c[2];
                }
                tcounts += this->triangle_counts[i];
            }

            // Lastly, this call copies vertexColors (etc) into the OpenGL memory space
            this->reinit_colour_buffer();
        }

        void reinitColours()
        {
            if (this->vertexColors.size() < this->triangle_count_sum * 3) {
                throw std::runtime_error ("vertexColors is not big enough to reinitColours()");
            }

            // now sub-call the scalar or vector reinit colours function
            if (this->scalarData != nullptr) {
                this->reinitColoursScalar();
            } else if (this->vectorData != nullptr) {
                this->reinitColoursVector();
            } else {
                throw std::runtime_error ("No data to reinitColours()");
            }
        }

        //! If true, show 2.5D Voronoi edges
        bool debug_edges = false;
        //! If true, show 2D Voronoi edges
        bool show_voronoi2d = false;
        //! If true, show black spheres at dataCoord locations
        bool debug_dataCoords = false;

        //! Set true to debug the issue with computing z for border edges on, say, a
        //! rectangular grid where some edges have only one site after the jcvoronoi
        //! algorithm.
        static constexpr bool debug_border_edge_sites = false;

        //! What direction should be considered 'z' when converting the data into a voronoi diagram?
        //! The data values will be rotated before the Voronoi pass, then rotated back.
        morph::vec<float> data_z_direction = this->uz;

        //! You can add a little extra to the rectangle that is auto-detected from the
        //! datacoordinate ranges. This defaults to epsilon to give the best possible
        //! surface with a rectangular grid.
        float border_width = std::numeric_limits<float>::epsilon();

        // Do we add index labels?
        bool labelIndices = false;
        morph::vec<float, 3> labelOffset = { 0.04f, 0.0f, 0.0f };
        float labelSize = 0.03f;

    protected:
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
                    if (!this->dcolour2.empty() && ! this->dcolour3.empty()) {
                        clr = this->cm.convert (this->dcolour[ri], this->dcolour2[ri], this->dcolour3[ri]);
                    }
                }
            } else if (this->cm.numDatums() == 2) {
                // Use vectorData
                if (!this->dcolour2.empty()) {
                    clr = this->cm.convert (this->dcolour[ri], this->dcolour2[ri]);
                }
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

        //! Have to record the number of triangles in each cell in order to update the colours
        morph::vvec<unsigned int> triangle_counts;
        //! Record the data index for each Voronoi cell index
        morph::vvec<unsigned int> site_indices;
        unsigned int triangle_count_sum = 0;

        //! A copy of the scalarData which can be transformed suitably to be the z value of the surface
        std::vector<float> dcopy;
        //! A copy of the scalarData (or first field of vectorData), scaled to be a colour value
        std::vector<float> dcolour;
        std::vector<float> dcolour2;
        std::vector<float> dcolour3;

        //! Internally owned version of dataCoords after rotation
        std::vector<morph::vec<float>> dcoords;
        //! A pointer either to dcoords or this->dataCoords
        const std::vector<morph::vec<float>>* dcoords_ptr;
    };

} // namespace morph
