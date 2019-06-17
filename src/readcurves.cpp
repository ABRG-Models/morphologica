#include "ReadCurves.h"
#include <utility>
#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>
#include "BezCoord.h"

using namespace std;
using morph::ReadCurves;
using morph::BezCoord;
using std::vector;

#define DEBUG 1
#define DBGSTREAM std::cout
#include "MorphDbg.h"

int main(int argc, char** argv)
{
    int rtn = -1;

    if (argc < 2 && argc > 0) {
        cerr << "Usage: " << argv[0] << " ./path/to/curves.svg" << endl;
        return rtn;
    }
    // Check path in argv[1]?

    try {
        ReadCurves r(argv[1]);
        //r.save (0.001f);
        vector<BezCoord> pts = r.getCorticalPath().getPoints (0.01f);
        auto i = pts.begin();
        cout << "The cortical path list of points is: " << endl;
        while (i != pts.end()) {
            cout << *i << endl;
            ++i;
        }

    } catch (const exception& e) {
        cerr << "Caught exception reading " << argv[1] << ": " << e.what() << endl;
        rtn = -1;
    }

    return rtn;
}
