#ifndef _DIRICHDOM_H_
#define _DIRICHDOM_H_

#include <list>
using std::list;
#include <vector>
using std::vector;
#include <utility>
using std::pair;
using std::make_pair;
#include <sstream>
using std::stringstream;
#include "DirichVtx.h"
using morph::DirichVtx;
#include "NM_Simplex.h"
using morph::NM_Simplex;
using morph::NM_Simplex_State;
#include "HdfData.h"
using morph::HdfData;
#include "Hex.h"
using morph::Hex;
#include "HexGrid.h"
using morph::HexGrid;
#define DEBUG 1
#include "MorphDbg.h"

namespace morph {

    /*!
     * Dirichlet domain class.
     *
     * Holds a list of DirichVtx objects, and a number of statistical metrics for the domain.
     */
    template <class Flt>
    class DirichDom {
    public:
        //! The ordered list of vertices that make up this Dirichlet domain.
        list<DirichVtx<Flt>> vertices;

        //! The area of the domain
        Flt area = 0.0;

        //! The identity of the domain
        Flt f = 0.0;

        //! The Honda1983 Dirichlet metric for the domain
        Flt honda = 0.0;

        //! A metric of how much the edges deviate from the straight lines that are defined by the vertices.
        Flt edge_deviation = 0.0;

        //! Return the number of vertices
        unsigned int numVertices (void) const {
            return this->vertices.size();
        }

        /*!
         * Compute the perpendicular distance from point p to the line defined by points a and b.
         */
        static Flt compute_distance_to_line (const pair<Flt, Flt>& p,
                                             const pair<Flt, Flt>& a, const pair<Flt, Flt>& b) {
            // Find angle between Ai--Pi and Ai--p
            Flt angle = DirichVtx<Flt>::compute_angle (p, a, b, 1);
            // And distance from p to Ai
            Flt p_to_a = DirichVtx<Flt>::line_length (p, a);
            // Return projection of p onto line Ai--Pi
            return (p_to_a * sin (angle));
        }

        /*!
         * Compute the root of the mean of the sum of the squared distances of the edges from the
         * straight line segments that join the vertices of this domain.
         */
        void compute_edge_deviation (void) {

            DirichVtx<Flt> lastvtx = this->vertices.back();

            Flt d2sum = 0.0;
            Flt dcount = 0.0;
            for (DirichVtx<Flt> vtx : this->vertices) {
                // vtx.v is the current coord, lastvtx.v is the prev coord. These mark the two ends of the line.
                for (pair<Flt, Flt> xi : vtx.pathto_next) {
                    // Find perp. distance from xi to x0-x1 line.
                    Flt dist = DirichDom<Flt>::compute_distance_to_line (xi, vtx.v, lastvtx.v);
                    dist *= dist;
                    d2sum += dist;
                    dcount += 1.0;
                }
                // Update the last vtx
                lastvtx = vtx;
            }

            Flt d2mean = d2sum / dcount;
            this->edge_deviation = sqrt (d2mean);
            DBG ("Edge deviation is " << this->edge_deviation);
        }

        /*!
         * Using passed-in HexGrid (@hg) and identity map (@f), compute the area of this domain. Can
         * use the paths of the DirichVtx members to determine which hexes are inside and which are
         * outside the domain.
         *
         * The way to solve this is to use the solution I used in HexGrid::markHexesInside. This
         * means we go around the boundary, marking hexes in straight lines in all possible inward
         * directions from each boundary hex. Simples.
         */
        void compute_area (HexGrid* hg, const vector<Flt>& f) {

            // Start at one of the vertices. Follow the edge of one vertex, counting/marking hexes
            // as you go. Continue around the perimeter until you get back to the start. Now fill in
            // the region (with 'laser beams') until all hexes in the domain are marked.

            // Find a coordinate that is situated on the border of the domain
            typename list<DirichVtx<Flt>>::const_iterator dv = this->vertices.begin();
            pair<Flt, Flt> firstborder = dv->pathto_next.front();

            // Now find a hex in hg that a) has this coordinate on it as a vertex and b) has the
            // correct ID. This will be the first hex on the boundary.
            list<Hex>::iterator firsthex = hg->hexen.begin();
            while (firsthex != hg->hexen.end()) {
                if (firsthex->contains_vertex (firstborder) && f[firsthex->vi] == this->f) {
                    // This hex is on the border of this domain.
                    break;
                }
                ++firsthex;
            }

            // Now walk around the border, setting HEX_USER_FLAG_0/1 for every domain boundary hex
            // and HEX_USER_FLAG_0 for every domain hex.

            // Boundary hex iterator
            list<Hex>::iterator bhi = firsthex;
            // Previous boundary hex iterator
            list<Hex>::iterator bhi_prev = firsthex;
            // Neighbour hex iterator
            list<Hex>::iterator nhi = firsthex;
            // A vector of Hex iterators to be filled with the hexes on the domain boundary
            vector< list<Hex>::iterator > domBoundary;

            // Set flags on first hex and add it to domBoundary
            firsthex->setUserFlags(HEX_USER_FLAG_0 | HEX_USER_FLAG_1);
            domBoundary.push_back(firsthex);

            // Before the main while() loop, find a neighbouring hex that is also on the boundary
            for (unsigned int i = 0; i<6; ++i) {
                if (bhi->has_neighbour(i)) {
                    // Might be a boundary hex:
                    nhi = bhi->get_neighbour(i);
                    if (f[nhi->vi] == this->f) {
                        // Is a boundary hex if some of neighbours have id != f.
                        set<Flt> neighbid;
                        for (unsigned int j = 0; j<6; ++j) {
                            if (nhi->has_neighbour(j)) {
                                neighbid.insert (f[nhi->get_neighbour(j)->vi]);
                            }
                        }
                        if (neighbid.size() > 1 && nhi != bhi_prev) {
                            bhi_prev = bhi;
                            bhi = nhi;
                            break; // out of for
                        }
                    }
                }
            }

            // Now bhi_prev and bhi are set, should be able while through all the hexes on the
            // boundary of this domain, using the f ID to guide us...
            DBG2 ("while loop to find boundary...");
            bool gotnext = false;
            while (bhi->getUserFlag(1) == false && bhi != hg->hexen.end()) {
                DBG2 ("gotnext set to false, now do stuff");
                gotnext = false;
                for (unsigned int i = 0; i<6; ++i) {
                    if (bhi->has_neighbour(i)) {
                        DBG2 ("neighbour in " << Hex::neighbour_pos(i) << " dirn");
                        // Might be a boundary hex:
                        nhi = bhi->get_neighbour(i);
                        if (f[nhi->vi] == this->f) {
                            DBG2 ("this neighbour matches ID");
                            // nhi is also a boundary hex if some of its neighbours have id != f.
                            set<Flt> neighbid;
                            neighbid.insert(f[nhi->vi]);
                            for (unsigned int j = 0; j<6; ++j) {
                                if (nhi->has_neighbour(j)) {
                                    neighbid.insert (f[nhi->get_neighbour(j)->vi]);
                                }
                            }
                            DBG2 ("number of IDs next to this neighbour is " << neighbid.size());
                            if (neighbid.size() > 1 && nhi != bhi_prev && nhi->getUserFlag(1) == false) {
                                DBG2 ("Setting flags on bhi " << bhi->outputRG());
                                // FLAG_1 Marks the hex as being 'just inside' the domain boundary
                                // FLAG_0 Marks the hex as being inside the domain boundary
                                bhi->setUserFlags(HEX_USER_FLAG_0 | HEX_USER_FLAG_1);
                                domBoundary.push_back (bhi);
                                bhi_prev = bhi;
                                bhi = nhi;
                                DBG2 ("Next hex is " << bhi->outputRG());
                                gotnext = true;
                                break; // out of for, but not while
                            }
                        }
                    }
                }
                if (gotnext == false) {
                    break;
                }
            }

            // Mark the last one...
            DBG2 ("Mark last hex on boundary " << bhi->outputRG());
            bhi->setUserFlags (HEX_USER_FLAG_0 | HEX_USER_FLAG_1);
            domBoundary.push_back (bhi);

            // It's possible to miss out a hex on the boundary, when there are two hexes next to
            // each other which are both on the boundary and a third hex protruding out - a sort of
            // boundary pimple. So, run through domBoundary to catch these cases and ensure that the
            // area measurement is accurate.
            for (list<Hex>::iterator hi : domBoundary) {
                for (unsigned int i = 0; i<6; ++i) {
                    if (hi->has_neighbour(i)) {
                        nhi = hi->get_neighbour(i);
                        if (f[nhi->vi] == this->f && nhi->getUserFlag(0) == false) {
                            // Simply set neighbouring hexes that have the correct ID as being in the domain.
                            nhi->setUserFlag(0);
                        }
                    }
                }
            }

            DBG ("foreach hex in domBoundary");
            // Now the domain boundary should have been found.
            list<Hex>::iterator innerhex = hg->hexen.end();
            for (list<Hex>::iterator hi : domBoundary) {

                DBG2 ("boundary hex " << hi->outputRG());
                // Mark inwards in all possible directions from nh.
                unsigned short firsti = 0;
                for (unsigned short i = 0; i < 6; ++i) {
                    if (hi->has_neighbour(i)
                        && f[hi->get_neighbour(i)->vi] == this->f
                        && hi->get_neighbour(i)->getUserFlag(1) == false) {
                        innerhex = hi->get_neighbour(i);
                        innerhex->setUserFlag(0);
                        firsti = i;
                        break;
                    }
                }

                // It's possible that the starting hex has no "inner hex" next to it, so continue on
                // to the next hex on the boundary.
                if (innerhex == hg->hexen.end()) { continue; }

                DBG2 ("firsti is " << firsti);
                // mark in a straight line in direction firsti
                while (innerhex->has_neighbour(firsti)
                       && f[innerhex->get_neighbour(firsti)->vi] == this->f
                       && innerhex->get_neighbour(firsti)->getUserFlag(1) == false) {
                    innerhex = innerhex->get_neighbour(firsti);
                    innerhex->setUserFlag(0);
                }

                // First count upwards until we hit a boundary hex
                short diri = (firsti + 1) % 6;
                while (hi->has_neighbour (diri)
                       && diri != firsti
                       && f[hi->get_neighbour(diri)->vi] == this->f
                       && hi->get_neighbour(diri)->getUserFlag(1) == false) {
                    innerhex = hi->get_neighbour (diri);
                    innerhex->setUserFlag(0);
                    // mark in dirn diri
                    while (innerhex->has_neighbour(diri)
                           && f[innerhex->get_neighbour(diri)->vi] == this->f
                           && innerhex->get_neighbour(diri)->getUserFlag(1) == false) {
                        innerhex = innerhex->get_neighbour(diri);
                        innerhex->setUserFlag(0);
                    }
                    diri = (diri + 1) % 6;
                }
                // Then count downwards until we hit the other boundary hex
                diri = firsti>0 ? firsti-1 : 5;
                while (hi->has_neighbour (diri)
                       && diri != firsti
                       && f[hi->get_neighbour(diri)->vi] == this->f
                       && hi->get_neighbour(diri)->getUserFlag(1) == false) {
                    innerhex = hi->get_neighbour (diri);
                    innerhex->setUserFlag(0);
                    // mark the hexes...
                    while (innerhex->has_neighbour(diri)
                           && f[innerhex->get_neighbour(diri)->vi] == this->f
                           && innerhex->get_neighbour(diri)->getUserFlag(1) == false) {
                        innerhex = innerhex->get_neighbour(diri);
                        innerhex->setUserFlag(0);
                    }
                    diri = diri>0 ? diri-1 : 5;
                }

                // Find the next hex along the boundary.
            }

            // Now count the area up, resetting the flags as we go
            unsigned int hcount = 0;
            for (Hex& h : hg->hexen) {
                hcount += (h.getUserFlag(0) == true) ? 1 : 0;
                h.resetUserFlags();
            }
            DBG ("hcount = " << hcount);
            this->area = hg->getHexArea() * hcount;
            DBG ("Area = " << this->area);
        }

        //! This is the objective function for the gradient descent. Put it in DirichDom
        Flt compute_sos (const Flt& x, const Flt& y) const {
            typename list<DirichVtx<Flt>>::const_iterator dv = this->vertices.begin();
            Flt sos = 0.0;
            while (dv != this->vertices.end()) {
                // Compute sum of square distances to the lines.
                Flt dist = dv->compute_distance_to_line (make_pair(x, y));
                sos += dist * dist;
                ++dv;
            }
            return sos;
        }

        /*!
         * Take a set of Dirichlet vertices defining exactly one Dirichlet domain and compute a
         * metric for the Dirichlet-ness of the vertices after Honda1983. Fixme: move into DirichDom.
         */
        Flt dirichlet_analyse_single_domain (pair<Flt, Flt>& P) {

            typename list<DirichVtx<Flt>>::iterator dv = this->vertices.begin();
            typename list<DirichVtx<Flt>>::iterator dvnext = dv;
            typename list<DirichVtx<Flt>>::iterator dvprev = this->vertices.end();

            Flt mean_x = 0.0;
            Flt mean_y = 0.0;

            // Compute Pi lines for each vertex in the domain, and also (for later use) the mean
            // position of the vertices.
            while (dv != this->vertices.end()) {

                dvnext = ++dv;
                if (dvnext == this->vertices.end()) {
                    dvnext = this->vertices.begin();
                }

                pair<Flt, Flt> Aim1;
                if (dvprev == this->vertices.end()) {
                    dvprev--;
                    Aim1 = dvprev->v;
                    dvprev = this->vertices.begin();
                } else {
                    Aim1 = dvprev->v;
                    ++dvprev;
                }

                // Reset dv back one now we figured out dvnext and dvprev
                dv--;
                mean_x += dv->v.first;
                mean_y += dv->v.second;
                dv->compute_line_to_centre (Aim1, dvnext->v);
                ++dv;
            }

            // Ok, got the lines to Pi for each Dirichlet vertex. Can now find a Pi_best that
            // minimises the distance to each Pi line.
            //
            // This is amenable to a nice simple gradient descent. Will implement a Nelder-Mead
            // approach.

            // Start out with a simplex with a vertex at the centroid of the domain vertices, and
            // then two other vertices at the first domain vertex (v) and its neighbour (vn).
            pair<Flt, Flt> Pi_best;
            Pi_best.first = mean_x / this->vertices.size();
            Pi_best.second = mean_y / this->vertices.size();
            NM_Simplex<Flt> simp (Pi_best, this->vertices.begin()->v, this->vertices.begin()->vn);
            simp.termination_threshold = numeric_limits<Flt>::epsilon();

            while (simp.state != NM_Simplex_State::ReadyToStop) {

                if (simp.state == NM_Simplex_State::NeedToComputeThenOrder) {
                    // 1. apply objective to each vertex
                    for (unsigned int i = 0; i <= simp.n; ++i) {
                        simp.values[i] = this->compute_sos (simp.vertices[i][0], simp.vertices[i][1]);
                    }
                    simp.order();

                } else if (simp.state == NM_Simplex_State::NeedToOrder) {
                    simp.order();

                } else if (simp.state == NM_Simplex_State::NeedToComputeReflection) {
                    Flt val = this->compute_sos (simp.xr[0], simp.xr[1]);
                    simp.apply_reflection (val);

                } else if (simp.state == NM_Simplex_State::NeedToComputeExpansion) {
                    Flt val = this->compute_sos (simp.xe[0], simp.xc[1]);
                    simp.apply_expansion (val);

                } else if (simp.state == NM_Simplex_State::NeedToComputeContraction) {
                    Flt val = this->compute_sos (simp.xc[0], simp.xc[1]);
                    simp.apply_contraction (val);
                }
            }
            vector<Flt> vP = simp.best_vertex();
            Flt min_sos = simp.best_value();
            DBG ("FINISHED! Best approximation: (" << vP[0] << "," << vP[1] << ") has value " << min_sos);
            // We now have a P and a metric

            // Write P into the ref in the arg
            P.first = vP[0];
            P.second = vP[1];

            // Return the metric. In Honda 1983, this is $\Delta_j$
            Flt mean_sos_per_vertex = min_sos/static_cast<Flt>(this->numVertices());

            this->honda = mean_sos_per_vertex;

            return mean_sos_per_vertex;
        }

        //! Save this domain data in HdfData& @data under path @pathroot
        void save (HdfData& data, const string& pathroot) const {
            string p("");
            p = pathroot + "/f";
            data.add_val (p.c_str(), this->f);
            p = pathroot + "/area";
            data.add_val (p.c_str(), this->area);
            p = pathroot + "/honda";
            data.add_val (p.c_str(), this->honda);
            p = pathroot + "/edgedev";
            data.add_val (p.c_str(), this->edge_deviation);

            unsigned int vcount = 0;
            for (auto dv : this->vertices) {
                stringstream vname;
                vname << pathroot << "/vtx";
                vname.width(3);
                vname.fill('0');
                vname << vcount++;
                dv.save (data, vname.str());
            }
        }
    };

} // namespace morph

#endif // _DIRICHDOM_H_
