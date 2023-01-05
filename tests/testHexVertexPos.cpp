#include "morph/Hex.h"
#include <iostream>
#include <utility>

using namespace morph;
using namespace std;

int main()
{
    int r = 0;
    int g = 0;
    float d = 2.0f;
    unsigned int idx = 0;
    Hex h(idx, d, r, g);

    pair<float, float> vN = h.get_vertex_coord (HEX_VERTEX_POS_N);
    pair<float, float> vNE = h.get_vertex_coord (HEX_VERTEX_POS_NE);
    pair<float, float> vSE = h.get_vertex_coord (HEX_VERTEX_POS_SE);
    pair<float, float> vS = h.get_vertex_coord (HEX_VERTEX_POS_S);
    pair<float, float> vSW = h.get_vertex_coord (HEX_VERTEX_POS_SW);
    pair<float, float> vNW = h.get_vertex_coord (HEX_VERTEX_POS_NW);

    // Output for user
    cout << "Hex centre: (" << h.x << "," << h.y << ")" << endl;
    cout << "Hex vertex N : (" << vN.first << "," << vN.second << ")" << endl;
    cout << "Hex vertex NE: (" << vNE.first << "," << vNE.second << ")" << endl;
    cout << "Hex vertex SE: (" << vSE.first << "," << vSE.second << ")" << endl;
    cout << "Hex vertex S : (" << vS.first << "," << vS.second << ")" << endl;
    cout << "Hex vertex SW: (" << vSW.first << "," << vSW.second << ")" << endl;
    cout << "Hex vertex NW: (" << vNW.first << "," << vNW.second << ")" << endl;


    float vto_ne = d/(2.0f * morph::mathconst<float>::sqrt_of_3);
    // Test the numbers (non-exhaustive)
    if (vN.first == 0.0f
        && vNE.second == vto_ne
        && vSW.second == -vto_ne) {
        cout << "PASS" << endl;
        return 0;
    }

    cout << "FAIL" << endl;
    return 1;
}
