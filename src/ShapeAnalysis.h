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

                // Now find the other vertex which divides "me" and "neighbour"
                typename set<DirichVtx<Flt> >::iterator dvi2 = dv.begin();

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

                    // if dvi2 contains the two floats in v.neighb, then it's a match.
                    pair<bool, bool> matches = { false, false };
                    if ((dvi2->f == v.neighb.first)
                        || (dvi2->neighb.first == v.neighb.first)
                        || (dvi2->neighb.second == v.neighb.first)) {
                        matches.first = true;
                    }
                    if ((dvi2->f == v.neighb.second)
                        || (dvi2->neighb.first == v.neighb.second)
                        || (dvi2->neighb.second == v.neighb.second)) {
                        matches.second = true;
                    }

                    if (matches.first && matches.second) {
                        // Test distance:
                        if ( (dvi2->v.first - v.v.first)*(dvi2->v.first - v.v.first)
                             + (dvi2->v.second - v.v.second)*(dvi2->v.second - v.v.second) > 1e-7 ) {
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

        static void
        vertex_test (HexGrid* hg, vector<Flt>& f, list<Hex>::iterator h, set<DirichVtx<Flt> >& vertices) {
            // For each hex, examine its neighbours, counting number of different neighbours.
            set<Flt> n_ids;
            n_ids.insert (f[h->vi]);
            for (unsigned int ni = 0; ni < 6; ++ni) {
                if (h->has_neighbour(ni)) {
                    n_ids.insert (f[h->get_neighbour(ni)->vi]);
                }
            }

            if (h->boundaryHex == true && n_ids.size() == 2) { // 1. Test for boundary vertices

                // Here, I need to set a vertex where two hexes join and
                // we're on the boundary. This provides
                // information to set the angles to discover the
                // best center for each domain (see Honda 1983).

                for (int ni = 0; ni < 6; ++ni) { // ni==0 is neighbour east. 1 is neighbour NE, etc.

                    // If there's a neighbour in neighbour direction ni and that neighbour has different ID:
                    if (h->has_neighbour(ni) && f[h->get_neighbour(ni)->vi] != f[h->vi]) {

                        // Change this - examine which direction
                        // DOESN'T have a neighbour and that will
                        // determine which hex vertex is the
                        // domain vertex.

                        // The first non-identical ID
                        Flt f1 = f[h->get_neighbour(ni)->vi];
                        int nii = (ni+1)%6;
                        if (!h->has_neighbour(nii)) {
                            // Then vertex direction is "vertex direction ni"
                            vertices.insert (
                                DirichVtx<Flt> (
                                    h->get_vertex_coord(ni),
                                    hg->getd(),
                                    f[h->vi],
                                    make_pair(-1.0f, f[h->get_neighbour(ni)->vi]),
                                    h)
                                );
                            break; // or set ni=6;

                        } else {

                            nii = ni>0 ? (ni-1) : 5;
                            if (!h->has_neighbour(nii)) {
                                // Then vertex direction is "vertex direction (ni-1) or 5", i.e. nii.
                                vertices.insert (
                                    DirichVtx<Flt> (
                                        h->get_vertex_coord(nii),
                                        hg->getd(),
                                        f[h->vi],
                                        make_pair(f[h->get_neighbour(ni)->vi], -1.0f),
                                        h)
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
                    if (h->has_neighbour(ni) && f[h->get_neighbour(ni)->vi] != f[h->vi]) {

                        // The first non-identical ID
                        Flt f1 = f[h->get_neighbour(ni)->vi];
                        int nii = (ni+1)%6;

                        if (h->has_neighbour(nii)
                            && f[h->get_neighbour(nii)->vi] != f[h->vi]
                            && f[h->get_neighbour(nii)->vi] != f1 // f1 already tested != f[h->vi]
                            ) {
                            // Then vertex is "vertex ni"
                            vertices.insert (
                                DirichVtx<Flt>(
                                    h->get_vertex_coord(ni),
                                    hg->getd(),
                                    f[h->vi],
                                    make_pair(f[h->get_neighbour(nii)->vi], f[h->get_neighbour(ni)->vi]),
                                    h)
                                );
                            break;

                        } else {
                            nii = ni>0 ? (ni-1) : 5;
                            if (h->has_neighbour(nii)
                                && f[h->get_neighbour(nii)->vi] != f[h->vi]
                                && f[h->get_neighbour(nii)->vi] != f1 // f1 already tested != f[h->vi]
                                ) {
                                vertices.insert (
                                    DirichVtx<Flt>(
                                        h->get_vertex_coord(nii),
                                        hg->getd(),
                                        f[h->vi],
                                        make_pair(f[h->get_neighbour(ni)->vi], f[h->get_neighbour(nii)->vi]),
                                        h)
                                    );
                                break;
                            }
                        }
                    }
                }
            }
        }

        /*!
         * This method takes the HexGrid, along with the ID map, and
         * uses information in each hex to traverse edges out from
         * each vertex to its neighbour.
         */
        static void
        dirichlet_set_neighbours (HexGrid* hg, vector<Flt>& f, set<DirichVtx<Flt> >& dv) {

            cout << __FUNCTION__ << "called" << endl;
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

#if 1
                // Select an f value for which to show debug data.
                bool dbg = (fabs(v.f-0.28f)<0.01);

                if (dbg) {
                    DBG ("I have f=" << v.f << " and neighbour f values "
                         << v.neighb.first << "/" << v.neighb.second);
                }
#endif

                // Starting from hex v.hi, find neighbours whos f
                // values are v.neighb.first/v.neighb.second. Record
                // (in v.edge) a series of coordinates that make up
                // the edge between that vertex and the neighbour.
                list<Hex>::iterator hexit = v.hi;
                list<Hex>::iterator hexit_next = v.hi;
                list<Hex>::iterator hexit_neighb = v.hi;
                list<Hex>::iterator hexit_last = v.hi;

                // Set true when we find the partner vertex.
                bool partner_found = false;
                pair<Flt, Flt> edgedoms = v.neighb;

                // Find the initial direction of the edge:
                int i = 0;
                for (i = 0; i<6; ++i) {
                    if (hexit->get_vertex_coord(i) == v.v) {
                        // Then the neighbours are either side of vertex direction i.
                        if (dbg) {
                            cout << "initial vertex in direction " << i << endl;
                        }

                        // I'll track the hex in neighb.first.
                        if (hexit->has_neighbour ((i+1)%6)) {

                            if (dbg) {
                                cout << "adjoining hex in direction i+1 = " << ((i+1)%6)
                                     << " has f=" << f[hexit->get_neighbour ((i+1)%6)->vi] << endl;
                            }

                            // The next hex is this:
                            hexit = hexit->get_neighbour ((i+1)%6);

                            if (f[hexit->vi] == edgedoms.first) {
                                if (dbg) { cout << "Good" << endl; }
                            } else {
                                cout << "Not good." << endl;
                            }

                            break;

                        } else {
                            cout << "No neighbour in direction " << ((i+1)%6)
                                 << ". What to do?" << endl;
                        }
                    }
                }

                // Here, we have in hexit, a hex with value
                // edgedoms.first. Find the neighbour hex with value
                // edgedoms.second and add two vertices to v.edge accordingly.
                //unsigned int icount = 0;
                set<Hex> edgehexes;
                while (!partner_found && hexit != hg->hexen.end()/* && icount++ < 100*/) {
                    for (int j = 0; j<6; ++j) {
                        // Look at hex neighbour in direction j
                        if (hexit->has_neighbour (j)) {
                            // If we have a neighbour, then check if it's on the other side of the edge.
                            hexit_neighb = hexit->get_neighbour(j);
                            if (f[hexit_neighb->vi] == edgedoms.second) {
                                // As it IS on the other side of the edge, add vertex j and
                                // vertex j-1 (or 5) to the list of coordinates in the edge.
                                cout << "Insert coordinates, hex vi=" << hexit->vi << ", j=" << j << endl;
                                edgehexes.insert (*hexit);
                                if (partner_found) {
                                    cout << "NOTE: Inserting after finding partner" << endl;
                                }
                                v.edge.insert (hexit->get_vertex_coord (j));
                                v.edge.insert (hexit->get_vertex_coord (((j>0)?(j-1):5)));

                                // Given neighbour *is* over the edge, check for the next hex in the adjoining neighbours
                                if (hexit->has_neighbour ((j>0)?(j-1):5)
                                    && !edgehexes.count(*(hexit_next = hexit->get_neighbour ((j>0)?(j-1):5)))
                                    && f[hexit_next->vi] == edgedoms.first) {
                                    cout << "1 hexit_next is " << hexit_next->vi << endl;
                                    hexit = hexit_next;
                                } else if (hexit->has_neighbour ((j+1)%6)
                                           && !edgehexes.count(*(hexit_next = hexit->get_neighbour ((j+1)%6)))
                                           && f[hexit_next->vi] == edgedoms.first) {
                                    cout << "2 hexit_next is " << hexit_next->vi << endl;
                                    hexit = hexit_next;
                                } else {
                                    cout << "Could not find next hex in edge. Are we at the end?" << endl;
                                    // Check for which vertex is surrounded by three different types.
                                    set<DirichVtx<Flt> > onevertex;
                                    vertex_test (hg, f, hexit, onevertex);
                                    cout << "Got " << onevertex.size() << " vertices from vertex_test()" << endl;
                                    // (maybe: double check if this vertex exists in dv) then put it in v.
                                    for (auto ovi : onevertex) {
                                        v.vn = ovi.v;
                                        cout << "Set partner to coordinate (" << v.vn.first << "," << v.vn.second<< ")" << endl;
                                        v.neighbn = ovi.neighb;
                                        partner_found = true;
                                    }
#define DEBUG__END 1
#ifdef DEBUG__END // Maybe useful, but store edge information first.
                                    if (!partner_found) {
                                        cout << "Should have found a partner for hex ["
                                             << hexit->ri << "," << hexit->gi << "," << hexit->bi << "]" << endl;
                                        cout << "which has neighbour in direction " << Hex::neighbour_pos(j)
                                             << " which has ID " << edgedoms.second << "=" << f[hexit_neighb->vi] << endl;
                                        cout << " neighbour in (j>0)?(j-1):5 dirn " << ((j>0)?(j-1):5) << "/" << Hex::neighbour_pos((j>0)?(j-1):5) << "?:"
                                             << (hexit->has_neighbour ((j>0)?(j-1):5) ? "yes":"no") << endl;
                                        cout << " neighbour in j+1%6 dirn " << ((j+1)%6) << "/" << Hex::neighbour_pos((j+1)%6) << "?:"
                                             << (hexit->has_neighbour ((j+1)%6) ? "yes":"no") << endl;
                                        partner_found = true; // To get us out of loop.
                                    }
#endif
                                }
                            }
                        }
                    }
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

            list<Hex>::iterator h = hg->hexen.begin();
            while (h != hg->hexen.end()) {
                vertex_test (hg, f, h, vertices);
                // Move on to the next Hex in hexen
                ++h;
            }

            // The last task, before returning vertices, is to process
            // through and populate the "neighbour" vertex coordinate
            // for each vertex. This can be computed based on the
            // three domains which each vertex divides.
            //dirichlet_set_neighbours (vertices);
            dirichlet_set_neighbours (hg, f, vertices);

            return vertices;
        }

    }; // ShapeAnalysis

} // namespace morph

#endif // SHAPEANALYSIS
