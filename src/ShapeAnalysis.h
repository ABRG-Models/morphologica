#ifndef _SHAPEANALYSIS_H_
#define _SHAPEANALYSIS_H_

#include <vector>
#include <list>
#include <set>
#include "morph/Hex.h"
#include "morph/HexGrid.h"
#include "morph/DirichVtx.h"

using std::vector;
using std::list;
using std::set;

using morph::Hex;
using morph::HexGrid;
using morph::DirichVtx;

namespace morph {

    /*!
     * A helper class, containing pattern analysis code to analyse
     * patterns within HexGrids.
     */
    template <class Flt>
    class ShapeAnalysis
    {
    public:
        /*!
         * Obtain the contours (as a vector of list<Hex>) in the scalar
         * fields f, where threshold is crossed.
         */
        static vector<list<Hex> > get_contours (HexGrid* hg,
                                                vector<vector<Flt> >& f,
                                                Flt threshold) {

            unsigned int nhex = hg->num();
            unsigned int N = f.size();

            vector<list<Hex> > rtn;
            // Initialise
            for (unsigned int li = 0; li < N; ++li) {
                list<Hex> lh;
                rtn.push_back (lh);
            }

            Flt maxf = -1e7;
            Flt minf = +1e7;
            for (auto h : hg->hexen) {
                if (h.onBoundary() == false) {
                    for (unsigned int i = 0; i<N; ++i) {
                        if (f[i][h.vi] > maxf) { maxf = f[i][h.vi]; }
                        if (f[i][h.vi] < minf) { minf = f[i][h.vi]; }
                    }
                }
            }
            Flt scalef = 1.0 / (maxf-minf);

            // Re-normalize
            vector<vector<Flt> > norm_f;
            norm_f.resize (N);
            for (unsigned int i=0; i<N; ++i) {
                norm_f[i].resize (nhex, 0.0);
            }

            for (unsigned int i = 0; i<N; ++i) {
                for (unsigned int h=0; h<nhex; h++) {
                    norm_f[i][h] = (f[i][h] - minf) * scalef;
                }
            }

            // Collate
            for (unsigned int i = 0; i<N; ++i) {
                for (auto h : hg->hexen) {
                    if (h.onBoundary() == false) {
                        if (norm_f[i][h.vi] > threshold) {
                            if ( (h.has_ne && norm_f[i][h.ne->vi] < threshold)
                                 || (h.has_nne && norm_f[i][h.nne->vi] < threshold)
                                 || (h.has_nnw && norm_f[i][h.nnw->vi] < threshold)
                                 || (h.has_nw && norm_f[i][h.nw->vi] < threshold)
                                 || (h.has_nsw && norm_f[i][h.nsw->vi] < threshold)
                                 || (h.has_nse && norm_f[i][h.nse->vi] < threshold) ) {
                                rtn[i].push_back (h);
                            }
                        }
                    } else { // h.onBoundary() is true
                        if (norm_f[i][h.vi] > threshold) {
                            rtn[i].push_back (h);
                        }
                    }
                }
            }

            return rtn;
        }

        /*!
         * Take a set of variables, @f, for the given HexGrid
         * @hg. Return a vector of Flts (again, based on the HexGrid
         * @hg) which marks each hex with the outer index of the @f
         * which has highest value in that hex, scaled and converted to a
         * float.
         */
        static vector<Flt>
        dirichlet_regions (HexGrid* hg, vector<vector<Flt> >& f) {
            unsigned int N = f.size();

            // Single variable to return
            vector<Flt> rtn (f[0].size(), 0.0);

            // Mark regions first.
            for (auto h : hg->hexen) {

                Flt maxf = -1e7;
                for (unsigned int i = 0; i<N; ++i) {
                    if (f[i][h.vi] > maxf) {
                        maxf = f[i][h.vi];
                        Flt fi = 0.0f;
                        fi = (Flt)i;
                        rtn[h.vi] = (fi/N);
                    }
                }
            }

            return rtn;
        }

        static void
        dirichlet_set_neighbours (set<DirichVtx<Flt> >& dv) {

            typename set<DirichVtx<Flt> >::iterator dvi = dv.begin();

            while (dvi != dv.end()) {

                // Don't set neighbours for the edge vertices (though
                // edge vertices *can be set* as neighbours for other
                // vertices).
                if (dvi->neighb.first == -1.0f || dvi->neighb.second == -1.0f) {
                    ++dvi;
                    continue;
                }

                // Make a copy of the vertex at *dvi, as I want to modify it.
                DirichVtx<Flt> v = *dvi;
                dvi = dv.erase (dvi);

#if 0
                // Select an f value for which to show debug data.
                bool dbg = (fabs(v.f-0.28f)<0.01);
#endif
                // Now find the other vertex which divides "me" and "neighbour"
                typename set<DirichVtx<Flt> >::iterator dvi2 = dv.begin();

                // Two floats that define the edge we want to move out along.
                pair<float, float> edges = v.neighb;

                while (dvi2 != dv.end()) {

                    // Ignore vertices of the same domain
                    if (dvi2->f == v.f) { // v.f is "me"
                        ++dvi2;
                        continue;
                    }

                    // Ignore vertices that are equal in location
                    if (dvi2->v.first == v.v.first
                        && dvi2->v.second == v.v.second) {
                        ++dvi2;
                        continue;
                    }

                    // if dvi2 contains the two floats in edges, then it's a match.
                    pair<bool, bool> matches = { false, false };
                    if ((dvi2->f == edges.first)
                        || (dvi2->neighb.first == edges.first)
                        || (dvi2->neighb.second == edges.first)) {
                        matches.first = true;
                    }
                    if ((dvi2->f == edges.second)
                        || (dvi2->neighb.first == edges.second)
                        || (dvi2->neighb.second == edges.second)) {
                        matches.second = true;
                    }

                    if (matches.first && matches.second) {
                        // Test distance:
                        if ( (dvi2->v.first - v.v.first)*(dvi2->v.first - v.v.first)
                             + (dvi2->v.second - v.v.second)*(dvi2->v.second - v.v.second) > 1e-7 ) {
#if 0
                            if (dbg) {
                                cout << "===================\n"
                                     << "match neighbours, set v.vn = (" << dvi2->v.first << "," << dvi2->v.second
                                     << ", " << std::hex << &dvi2->v << std::dec << ")"
                                     << " as neighbour for me at (" << v.v.first << "," << v.v.second << ")"
                                     << endl;
                            }
#endif
                            v.vn = dvi2->v;
                            v.neighbn = dvi2->neighb;
                            break;
                        } // else don't match vertex in same location
                    }
                    ++dvi2;
                }
                dv.insert(v);
                // no need to ++dvi;
            }
        }

        /*!
         * Determine the locations of the vertices on a Hex grid which
         * are surrounded by three different values of @f. @f is
         * indexed by the HexGrid @hg. Return a set of the vertices.
         */
        static set<DirichVtx<Flt> >
        dirichlet_vertices (HexGrid* hg, vector<Flt>& f) {

            // A set of pairs of floats, with a comparison function that will
            // set points as equivalent if they're within a small difference
            // of each other.
            set<DirichVtx<Flt> > vertices;

            for (auto h : hg->hexen) {

                // For each hex, examine its neighbours, counting number of different neighbours.
                set<Flt> n_ids;
                n_ids.insert (f[h.vi]);
                for (unsigned int ni = 0; ni < 6; ++ni) {
                    if (h.has_neighbour(ni)) {
                        n_ids.insert (f[h.get_neighbour(ni)->vi]);
                    }
                }

                if (h.boundaryHex == true && n_ids.size() == 2) { // 1. Test for boundary vertices

                    // Here, I need to set a vertex where two hexes join and
                    // we're on the boundary. This provides
                    // information to set the angles to discover the
                    // best center for each domain (see Honda 1983).

                    for (int ni = 0; ni < 6; ++ni) { // ni==0 is neighbour east. 1 is neighbour NE, etc.

                        // If there's a neighbour in neighbour direction ni and that neighbour has different ID:
                        if (h.has_neighbour(ni) && f[h.get_neighbour(ni)->vi] != f[h.vi]) {

                            // Change this - examine which direction
                            // DOESN'T have a neighbour and that will
                            // determine which hex vertex is the
                            // domain vertex.

                            // The first non-identical ID
                            Flt f1 = f[h.get_neighbour(ni)->vi];
                            int nii = (ni+1)%6;
                            if (!h.has_neighbour(nii)) {
                                // Then vertex direction is "vertex direction ni"
                                vertices.insert (
                                    DirichVtx<Flt> (
                                        h.get_vertex_coord(ni),
                                        hg->getd(),
                                        f[h.vi],
                                        make_pair(-1.0f, f[h.get_neighbour(ni)->vi])
                                        )
                                    );
                                break; // or set ni=6;

                            } else {

                                nii = ni>0 ? (ni-1) : 5;
                                if (!h.has_neighbour(nii)) {
                                    // Then vertex direction is "vertex direction (ni-1) or 5", i.e. nii.
                                    vertices.insert (
                                        DirichVtx<Flt> (
                                            h.get_vertex_coord(nii),
                                            hg->getd(),
                                            f[h.vi],
                                            make_pair(f[h.get_neighbour(ni)->vi], -1.0f)
                                            )
                                        );
                                    break; // or set ni=6;
                                }
                            }
                        }
                    }

                } else if (n_ids.size() > 2) { // 2. Test for internal vertices

                    // >2 (i.e. 3) different types in self &
                    // neighbouring hexes, so now work out which of
                    // the Hex's vertices is the vertex of the domain.

                    for (int ni = 0; ni < 6; ++ni) { // ni==0 is neighbour east. 1 is neighbour NE, etc.

                        // If there's a neighbour in direction ni and that neighbour has different ID:
                        if (h.has_neighbour(ni) && f[h.get_neighbour(ni)->vi] != f[h.vi]) {

                            // The first non-identical ID
                            Flt f1 = f[h.get_neighbour(ni)->vi];
                            int nii = (ni+1)%6;

                            if (h.has_neighbour(nii)
                                && f[h.get_neighbour(nii)->vi] != f[h.vi]
                                && f[h.get_neighbour(nii)->vi] != f1 // f1 already tested != f[h.vi]
                                ) {
                                // Then vertex is "vertex ni"
                                vertices.insert (
                                    DirichVtx<Flt>(
                                        h.get_vertex_coord(ni),
                                        hg->getd(),
                                        f[h.vi],
                                        make_pair(f[h.get_neighbour(nii)->vi], f[h.get_neighbour(ni)->vi])
                                        )
                                    );
                                break;

                            } else {
                                nii = ni>0 ? (ni-1) : 5;
                                if (h.has_neighbour(nii)
                                    && f[h.get_neighbour(nii)->vi] != f[h.vi]
                                    && f[h.get_neighbour(nii)->vi] != f1 // f1 already tested != f[h.vi]
                                    ) {
                                    vertices.insert (
                                        DirichVtx<Flt>(
                                            h.get_vertex_coord(nii),
                                            hg->getd(),
                                            f[h.vi],
                                            make_pair(f[h.get_neighbour(ni)->vi], f[h.get_neighbour(nii)->vi])
                                            )
                                        );
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            // The last task, before returning vertices, is to process
            // through and populate the "neighbour" vertex coordinate
            // for each vertex. This can be computed based on the
            // three domains which each vertex divides.
            dirichlet_set_neighbours (vertices);

            return vertices;
        }

    }; // ShapeAnalysis

} // namespace morph

#endif // SHAPEANALYSIS
