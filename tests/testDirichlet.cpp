//
// Testing/debugging Dirichlet boundary code
//

#include <iostream>
#define DBGSTREAM std::cout
#define DEBUG 1
#include <morph/MorphDbg.h>

#include <morph/HexGrid.h>
#include <morph/ReadCurves.h>
#include <morph/tools.h>
#include <morph/ColourMap.h>
#include <vector>
#include <list>
#include <array>
#include <stdexcept>
#include <morph/ShapeAnalysis.h>
#include <morph/Visual.h>
#include <morph/PolygonVisual.h>

using namespace morph;
using namespace std;

int main()
{
    int rtn = 0;
    try {
        HexGrid hg(0.2, 1, 0);

        hg.setBoundaryOnOuterEdge();

        cout << hg.extent() << endl;

        cout << "Number of hexes in grid:" << hg.num() << endl;
        cout << "Last vector index:" << hg.lastVectorIndex() << endl;

        // Make up a variable.
        vector<float> f (hg.num(), 0.1f);

        // Set values in the variable so that it's an identity variable.
        auto hi = hg.hexen.begin();
        auto hi2 = hi;
        while (hi->has_nse()) {
            while (hi2->has_ne()) {
                f[hi2->vi] = 0.2f;
                hi2 = hi2->ne;
            }
            f[hi2->vi] = 0.2f;
            hi2 = hi->nse;
            hi = hi->nse;
        }
        f[hi2->vi] = 0.2f;

        hi = hg.hexen.begin()->nw;
        hi2 = hi;
        while (hi->has_nse()) {
            while (hi2->has_nw()) {
                f[hi2->vi] = 0.4f;
                hi2 = hi2->nw;
            }
            f[hi2->vi] = 0.4f;
            hi2 = hi->nse;
            hi = hi->nse;
        }
        f[hi2->vi] = 0.4f;

        hi = hg.hexen.begin();
        f[hi->vi] = 0.3f;
        f[hi->ne->vi] = 0.3f;
        f[hi->nse->vi] = 0.3f;

        // The code to actually test:
        list<morph::DirichVtx<float>> vertices;
        list<morph::DirichDom<float>> domains = morph::ShapeAnalysis<float>::dirichlet_vertices (&hg, f, vertices);

        // There should be 19 vertices, precisely.
        unsigned int reqd = 19;
        if (vertices.size() != reqd) {
            DBG ("Not correct number of vertices; " << vertices.size() << " instead of " << reqd);
            rtn -= 1;
        }

        // Expecting one domain
        if (domains.size() != 1) { rtn -= 1; }

        morph::Visual v(1600, 1000, "Dirichlet code");
        v.lightingEffects();
        morph::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };
        morph::vec<float, 3> offset2 = offset;
        offset2 += {0,0,0.002f};
        array<float,3> cl_b = morph::ColourMap<float>::jetcolour (0.78);
        float sz = hg.hexen.front().d;
        for (auto h : hg.hexen) {
            array<float,3> cl_a = morph::ColourMap<float>::jetcolour (f[h.vi]);
            array<float,3> p = h.position();
            vec<float,3> pv = { p[0], p[1], p[2] };
            vec<float,3> vtx = pv;
            vtx += vec<float, 3>({1,0,0});
            auto pvp = std::make_unique<morph::PolygonVisual<>> (offset, pv, vtx, sz/1.8f, 0.002f, cl_a, 6);
            v.bindmodel (pvp);
            pvp->finalize();
            v.addVisualModel (pvp);
            if (h.boundaryHex()) {
                auto pvp2 = std::make_unique<morph::PolygonVisual<>> (offset2, pv, vtx, sz/12.0f, 0.002f, cl_b, 6);
                v.bindmodel (pvp2);
                pvp2->finalize();
                v.addVisualModel (pvp2);
            }
        }

        array<float,3> cl_c = morph::ColourMap<float>::jetcolour (0.98);
        for (auto verti : vertices) {
            vec<float,3> posn = verti.v.plus_one_dim (0.002);
            vec<float,3> vtx = posn + vec<float, 3>({1,0,0});
            auto pvp = std::make_unique<morph::PolygonVisual<>> (offset, posn, vtx, sz/8.0f, 0.002f, cl_c, 60);
            v.bindmodel (pvp);
            pvp->finalize();
            v.addVisualModel (pvp);
        }

        offset += { 0, 0, 0.004 };
        array<float,3> cl_d = morph::ColourMap<float>::jetcolour (0.7);
        array<float,3> cl_e = morph::ColourMap<float>::jetcolour (0.01);
        for (auto dom_outer : domains) {
            for (auto dom_inner : dom_outer.vertices) {
                // Draw the paths
                for (auto path : dom_inner.pathto_next) {
                    vec<float,3> posn = path.plus_one_dim (0.0);
                    vec<float,3> vtx = posn + vec<float, 3>({1,0,0});
                    auto pvp = std::make_unique<morph::PolygonVisual<>> (offset, posn, vtx, sz/16.0f, 0.002f, cl_d, 6);
                    v.bindmodel (pvp);
                    pvp->finalize();
                    v.addVisualModel (pvp);
                }
                for (auto path : dom_inner.pathto_neighbour) {
                    vec<float,3> posn = path.plus_one_dim (0.0);
                    vec<float,3> vtx = posn + vec<float, 3>({1,0,0});
                    auto pvp = std::make_unique<morph::PolygonVisual<>> (offset, posn, vtx, sz/16.0f, 0.002f, cl_e, 6);
                    v.bindmodel (pvp);
                    pvp->finalize();
                    v.addVisualModel (pvp);
                }
            }
        }

        // Draw small hex at boundary centroid.
        vec<float,3> centroid = {hg.boundaryCentroid[0], hg.boundaryCentroid[1], 0.0f};
        vec<float,3> centroidv = centroid + vec<float,3> ({ 0.0f, 1.0f, 0.0f });
        auto pvp = std::make_unique<morph::PolygonVisual<>> (morph::vec<float>({0,0,0}), centroid, centroidv, sz/16.0f, 0.01f, morph::vec<float>({0,0,1}), 10);
        v.bindmodel (pvp);
        pvp->finalize();
        v.addVisualModel (pvp);
        // red hex at zero
        auto pvp2 = std::make_unique<morph::PolygonVisual<>> (morph::vec<float>({0,0,0.01f}), morph::vec<float>({0,0,0}), morph::vec<float>({0,1,0}), sz/20.0f, 0.01f, morph::vec<float>({1,0,0}), 8);
        v.bindmodel (pvp2);
        pvp2->finalize();
        v.addVisualModel (pvp2);

        v.keepOpen();

    } catch (const exception& e) {
        cerr << "Caught exception: " << e.what() << endl;
        cerr << "Current working directory: " << Tools::getPwd() << endl;
        rtn = -1;
    }
    return rtn;
}
