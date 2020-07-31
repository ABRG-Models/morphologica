#include "tools.h"
#include <utility>
#include <iostream>
#include <unistd.h>

#include "HexGrid.h"
#include "ReadCurves.h"

#include "display.h"

using namespace morph;
using namespace std;

int main()
{
    if (XOpenDisplay(NULL) == (Display*)0) {
        cout << "No display, can't run test. Return 0\n";
        return 0;
    }

    int rtn = 0;
    try {

        float hhd = 0.02
        HexGrid hg (hhd, 35.0f*hhd, 0, morph::HexDomainShape::Boundary);
        hg.setEllipticalBoundary (this->ellipse_a, this->ellipse_b);
        hg.computeDistanceToBoundary();

        cout << hg.extent() << endl;
        cout << "Number of hexes in grid:" << hg.num() << endl;
        cout << "Last vector index:" << hg.lastVectorIndex() << endl;

        if (hg.num() != 2088 && hg.num() != 2087) {
            cerr << "hg num (" << hg.num() << ") not equal to 2087/2088..." << endl;
            rtn = -1;
        }

        morph::Visual v1 (win_width, win_height, "Large hex grid test");
        unsigned int idx = v1.addVisualModel (new morph::HexGridVisual<FLT> (v1.shaderprog,
                                                                                 RD.hg,
                                                                                 spatOff,
                                                                                 &RD.a[i],
                                                                                 zscale,
                                                                                 cscale,
                                                                                 morph::ColourMapType::Monochrome,
                                                                                 (float)i/(float)RD.N)); // hue

    } catch (const exception& e) {
        cerr << "Caught exception reading svg: " << e.what() << endl;
        cerr << "Current working directory: " << Tools::getPwd() << endl;
        rtn = -1;
    }

    return rtn;
}
