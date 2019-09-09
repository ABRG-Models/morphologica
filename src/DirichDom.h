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
         * Using passed-in HexGrid (@hg) and identity map (@f), compute the area of this domain. Can
         * use the paths of the DirichVtx members to determine which hexes are inside and which are
         * outside the domain.
         */
        void compute_area (const HexGrid* hg, const vector<Flt>& f) {
#if 0
            // Start at one of the vertices. Follow the edge of one vertex, counting/marking hexes
            // as you go. Continue around the perimeter until you get back to the start. Now fill in
            // the region until all hexes in the domain are marked.
            typename list<DirichVtx<Flt>>::const_iterator dv = this->vertices.begin();
            pair<Flt, Flt> firstborder = dv->pathto_next.front();
            // Got a point. Now find a hex in hg that a) has this point on it as a vertex and b) has
            // the correct ID.
            typename list<Hex>::const_iterator hi = hg->hexen.begin();
            typename list<Hex>::const_iterator firsthex = hi;
            while (hi != hg->hexen.end()) {
                if (hi->contains_vertex (firstborder) && f[hi->vi] == this->f) {
                    // This hex is on the border of this domain. Now fill in the area, using identity.
                    firsthex = hi;
                    break;
                }
                ++hi;
            }

            set<list<Hex>::iterator> insideHexes;
            insideHexes.insert (firsthex);
            // Walk around hexes, setting a userflag in the Hex, as below
#if 0
            while (hi->has_nne) {
                hi = hi->nne;
            }
            //hi->boundaryHex = true;
            //hi->insideBoundary = true;

            while (hi->has_ne) {
                hi = hi->ne;
                //hi->boundaryHex = true;
                //hi->insideBoundary = true;
            }
            while (hi->has_nse) {
                hi = hi->nse;
                //hi->boundaryHex = true;
                //hi->insideBoundary = true;
            }
            while (hi->has_nsw) {
                hi = hi->nsw;
                //hi->boundaryHex = true;
                //hi->insideBoundary = true;
            }
            while (hi->has_nw) {
                hi = hi->nw;
                //hi->boundaryHex = true;
                //hi->insideBoundary = true;
            }
            while (hi->has_nnw) {
                hi = hi->nnw;
                //hi->boundaryHex = true;
                //hi->insideBoundary = true;
            }
            while (hi->has_nne) {
                hi = hi->nne;
                //hi->boundaryHex = true;
                //hi->insideBoundary = true;
            }
            while (hi->has_ne && hi->ne->boundaryHex == false) {
                hi = hi->ne;
                //hi->boundaryHex = true;
                //hi->insideBoundary = true;
            }
#endif
#endif
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

            pair<Flt, Flt> Pi_best; // Start out with a simplex
            Pi_best.first = mean_x / this->vertices.size();
            Pi_best.second = mean_y / this->vertices.size();
            DBG ("Pi_best starts as mean of vertices: ("
                 << Pi_best.first << "," << Pi_best.second << ")");

            // Start with an initial simplex consisting of the centroid of the vertices, and two
            // other randomly chosen? Opposite? vertices from the domain.

            // So I think a Nelder-Mead Simplex class will be a nice approach:
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
