#ifndef _SHAPEANALYSIS_H_
#define _SHAPEANALYSIS_H_

#include <vector>
#include <list>
#include <set>
#include <limits>
#include "morph/Hex.h"
#include "morph/HexGrid.h"
#include "morph/DirichVtx.h"

using std::vector;
using std::list;
using std::set;
using std::numeric_limits;

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

        /*!
         * A method to test the hex give by @h, which must live on the
         * HexGrid pointed to by @hg, to see if it is a Dirichlet
         * vertex. If so, a vertex should be greated in @vertices.
         */
        static void
        vertex_test (HexGrid* hg, vector<Flt>& f,
                     list<Hex>::iterator h, list<DirichVtx<Flt> >& vertices) {

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
                            vertices.push_back (
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
                                vertices.push_back (
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
                            vertices.push_back (
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
                                vertices.push_back (
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

        // Need to return some sort of reference to the vertex that we find at the end of the walk.
        static pair<Flt, Flt>
        walk_common (HexGrid* hg,
                     vector<Flt>& f,
                     DirichVtx<Flt>& v,
                     list<pair<Flt, Flt>>& path,
                     pair<Flt, Flt>& edgedoms,
                     Flt& next_neighb_dom) {

            // Really, we only have coordinates to return.
            pair<Flt, Flt> next_one = {numeric_limits<Flt>::max(), numeric_limits<Flt>::max()};

            list<Hex>::iterator hexit = v.hi;
            list<Hex>::iterator hexit_next = v.hi;
            list<Hex>::iterator hexit_neighb = v.hi;
            list<Hex>::iterator hexit_last = v.hi;

            // Set true when we find the partner vertex.
            bool partner_found = false;

            // Find the initial direction of the edge:
            int i = 0;
            for (i = 0; i<6; ++i) {
                if (hexit->get_vertex_coord(i) == v.v) {
                    // Then the neighbours are either side of vertex direction i.
                    #if 0
                    cout << "initial vertex in direction " << i << endl;
                    #endif
                    // I'll track the hex in neighb.first.
                    if (hexit->has_neighbour ((i+1)%6)) {
                        #if 0
                        cout << "adjoining hex in direction i+1 = " << ((i+1)%6)
                             << " has f=" << f[hexit->get_neighbour ((i+1)%6)->vi] << endl;
                        #endif
                        // The next hex is this:
                        hexit = hexit->get_neighbour ((i+1)%6);

                        if (f[hexit->vi] == edgedoms.first) {
                            cout << "Good" << endl;
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
            cout << "After determining initial direction, i=" << i << ": "
                 << Hex::neighbour_pos(i) << endl;

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
                            cout << "Insert coordinates, hex vi="
                                 << hexit->vi << ", j=" << j << endl;
                            edgehexes.insert (*hexit);
                            if (partner_found) {
                                cout << "NOTE: Inserting after finding partner" << endl;
                            }
                            path.push_back (hexit->get_vertex_coord (j));
                            path.push_back (hexit->get_vertex_coord (((j>0)?(j-1):5)));

                            // Given neighbour *is* over the edge, check for the next hex in the
                            // adjoining neighbours
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
                                list<DirichVtx<Flt> > onevertex;
                                vertex_test (hg, f, hexit, onevertex);
                                cout << "Got " << onevertex.size()
                                     << " vertices from vertex_test()" << endl;
                                // (maybe: double check if this vertex exists in dv) then put it in v.
                                for (auto ovi : onevertex) {
                                    v.vn = ovi.v;
                                    cout << "Set partner to coordinate ("
                                         << v.vn.first << "," << v.vn.second << ")" << endl;
                                    v.neighbn = ovi.neighb;
                                    next_one = v.vn;
                                    // How to find the value of the
                                    // identity of the hex which ends
                                    // this edge?
                                    cout << "HACK of next_neighb_dom just to get some code compiling" << endl;
                                    next_neighb_dom = v.neighbn.first; // FIXME THIS IS WRONG
                                    partner_found = true;
                                }
//#define DEBUG__END 1
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
            return next_one;
        }

        /*!
         * Walk out to the next vertex from vertx @v on HexGrid @hg
         * for which identities are in @f.
         *
         * @hg The HexGrid on which the action takes place
         *
         * @f The identity variable.
         *
         * @v The Dirichlet Vertex that we're walking from, and into
         * which we're going to write the path to the next neighbour
         *
         * @next_neighb_dom The identity of the next domain neighbour
         * when we find the vertex neighbour.
         */
        static pair<Flt, Flt>
        walk_to_next (HexGrid* hg, vector<Flt>& f, DirichVtx<Flt>& v, Flt& next_neighb_dom) {

            // Starting from hex v.hi, find neighbours whos f
            // values are v.f/v.neighb.first. Record
            // (in v.path_to_next) a series of coordinates that make up
            // the path between that vertex and the next vertex in the domain.
            pair<Flt, Flt> edgedoms;
            edgedoms.first = v.f;
            edgedoms.second = v.neighb.first;

            pair<Flt, Flt> next_one = walk_common (hg, f, v, v.pathto_neighbour, edgedoms, next_neighb_dom);
            cout << "Next vertex coord: " << next_one.first << "," << next_one.second << endl;
            return next_one;
        }

        /*!
         * Walk out to a neighbour from vertex @v.
         *
         * @hg The HexGrid on which the action takes place
         *
         * @f The identity variable.
         *
         * @v The Dirichlet Vertex that we're walking from, and into
         * which we're going to write the path to the next neighbour
         *
         * @next_neighb_dom The identity of the next domain neighbour
         * when we find the vertex neighbour.
         */
        static void
        walk_to_neighbour (HexGrid* hg, vector<Flt>& f, DirichVtx<Flt>& v, Flt& next_neighb_dom) {

            // Don't set neighbours for the edge vertices (though
            // edge vertices *can be set* as neighbours for other
            // vertices).
            if (v.neighb.first == -1.0f || v.neighb.second == -1.0f) {
                return;
            }

            pair<Flt, Flt> edgedoms = v.neighb;
            pair<Flt, Flt> neighbour_coord = walk_common (hg, f, v, v.pathto_next, edgedoms, next_neighb_dom);
            cout << "Neighbour coord: " << neighbour_coord.first << "," << neighbour_coord.second << endl;
        }

        /*!
         * Given an iterator into the list of DirichVtxs @vertices,
         * find the next vertex in the domain, along with the vertex
         * neighbours, and recurse until @domain has been populated
         * with all the vertices that define it.
         *
         * Return true for success, false for failure, and leave dv
         * pointing to the next vertex in vertices so that @domain can
         * be stored, reset and the next Dirichlet domain can be
         * found.
         */
        static bool
        process_domain (HexGrid* hg, vector<Flt>& f,
                        typename list<DirichVtx<Flt>>::iterator dv,
                        list<DirichVtx<Flt>>& vertices,
                        list<DirichVtx<Flt>>& domain,
                        DirichVtx<Flt> first_vtx) {

            // Domain ID is set in dv as dv->f;
            DirichVtx<Flt> v = *dv;

            // On the first call, first_vtx should have been set to vertices.end()
            if (first_vtx.unset()) {
                // Mark the first vertex in our domain
                first_vtx = v;
            }

            // Find the neighbour of this vertex, if possible. Can't
            // do this if it's a boundary vertex, but nothing happens
            // in that case.
            Flt next_neighb_dom = numeric_limits<Flt>::max();
            walk_to_neighbour (hg, f, v, next_neighb_dom);

            // Walk to the next vertex
            next_neighb_dom = numeric_limits<Flt>::max();
            pair<Flt, Flt> next_vtx = walk_to_next (hg, f, v, next_neighb_dom);

            // Erase the vertex and push back v
            dv = vertices.erase (dv);
            domain.push_back (v);

            if (first_vtx.compare (next_vtx) == false) {
                // Find dv which matches next_vtx.
                while (dv != vertices.end()) {
                    // Instead of: dv = next_vtx;, do:
                    if (dv->compare (next_vtx) == true) {
                        // vertex has correct coordinate. Check it has correct neighbours.
                        if (dv->f == v.f
                            && dv->neighb.first == v.neighb.second
                            && dv->neighb.second == next_neighb_dom) {
                            // Match for current dv
                            break;
                        }
                        ++dv;
                    }
                }
            } else {
                cout << "Walk to next arrived back at the first vertex." << endl;
                return true;
            }

            // Now delete dv and move on to the next, recalling
            // process_domain recursively, or exiting if we got to the
            // start of the domain perimeter.
            return (process_domain (hg, f, dv, vertices, domain, first_vtx));
        }

        /*!
         * Determine the locations of the vertices on a Hex grid which
         * are surrounded by three different values of @f. @f is
         * indexed by the HexGrid @hg. Return a list containing lists
         * of the vertices, each of which define a domain.
         */
        static list<list<DirichVtx<Flt> > >
        dirichlet_vertices (HexGrid* hg, vector<Flt>& f) {

            // 1. Go though and find a list of all vertices, in no
            // particular order.  This will lead to duplications
            // because >1 domain for a given ID, f, is possible early
            // in simulations. From this list, I can find vertex sets,
            // whilst deleting from the list until it is empty, and
            // know that I will have discovered all the domain vertex
            // sets.
            list<DirichVtx<Flt> > vertices;
            list<Hex>::iterator h = hg->hexen.begin();
            while (h != hg->hexen.end()) {
                vertex_test (hg, f, h, vertices);
                // Move on to the next Hex in hexen
                ++h;
            }

            // 2. Delete from the list<DirichVtx> and construct a
            // list<list<DirichVtx>> of all the domains. The
            // list<DirichVtx> for a single domain should be ordered
            // so that the perimeter of the domain is traversed. I
            // have to do Dirichlet domain boundary walks to achieve
            // this (to disambiguate between vertices from separate,
            // but same-ID domains).
            list<list<DirichVtx<Flt>>> dirich_domains;
            typename list<DirichVtx<Flt>>::iterator dv = vertices.begin();
            while (dv != vertices.end()) {
                list<DirichVtx<Flt>> one_domain;
                DirichVtx<Flt> first_vtx;
                bool success = process_domain (hg, f, dv, vertices, one_domain, first_vtx);
                if (success) {
                    dirich_domains.push_back (one_domain);
                } else {
                    cout << "UH OH, process_domain failed!" << endl;
                }
            }

            return dirich_domains;
        }

        /*!
         * Save all the information contained in a set of dirichlet
         * vertices to HDF5 into the HdfData @data. The set(list?) of
         * Dirichlet vertices is for one single Dirichlet domain.
         */
        static void
        dirichlet_save_vertex_set (HdfData& data /* container of domains */) {

        }

    }; // ShapeAnalysis

} // namespace morph

#endif // SHAPEANALYSIS
