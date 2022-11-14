/*
 * Test the code which determines distance to boundary
 */

#include "morph/HexGrid.h"
#include "morph/tools.h"
#include "morph/ReadCurves.h"
#include <iostream>

using namespace morph;
using namespace std;

int main()
{
    int rtn = 0;
    try {
        string pwd = Tools::getPwd();
        string curvepath = "../../tests/trial.svg";

        ReadCurves r(curvepath);

        HexGrid hg(0.02f, 7.0f, 0.0f);
        hg.setBoundary (r.getCorticalPath());

        cout << hg.extent() << endl;

        cout << "Number of hexes in grid:" << hg.num() << endl;
        cout << "Last vector index:" << hg.lastVectorIndex() << endl;

        hg.computeDistanceToBoundary();

        for (auto h : hg.hexen) {
            cout << "r is " << h.r << " and dist to boundary: " << h.distToBoundary << endl;
            if (h.distToBoundary == -1) {
                rtn = -1;
            }
        }

    } catch (const exception& e) {
        cerr << "Caught exception reading trial.svg: " << e.what() << endl;
        cerr << "Current working directory: " << Tools::getPwd() << endl;
        rtn = -1;
    }
    return rtn;
}
