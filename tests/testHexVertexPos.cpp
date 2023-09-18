#include "morph/Hex.h"
#include <iostream>

using namespace morph;
using namespace std;

int main()
{
    int r = 0;
    int g = 0;
    float d = 2.0f;
    unsigned int idx = 0;
    Hex h(idx, d, r, g);

    morph::vec<float, 2> vN = h.get_vertex_coord (HEX_VERTEX_POS_N);
    morph::vec<float, 2> vNE = h.get_vertex_coord (HEX_VERTEX_POS_NE);
    morph::vec<float, 2> vSE = h.get_vertex_coord (HEX_VERTEX_POS_SE);
    morph::vec<float, 2> vS = h.get_vertex_coord (HEX_VERTEX_POS_S);
    morph::vec<float, 2> vSW = h.get_vertex_coord (HEX_VERTEX_POS_SW);
    morph::vec<float, 2> vNW = h.get_vertex_coord (HEX_VERTEX_POS_NW);

    // Output for user
    cout << "Hex centre: (" << h.x << "," << h.y << ")" << endl;
    cout << "Hex vertex N : " << vN << endl;
    cout << "Hex vertex NE: " << vNE << endl;
    cout << "Hex vertex SE: " << vSE << endl;
    cout << "Hex vertex S : " << vS << endl;
    cout << "Hex vertex SW: " << vSW << endl;
    cout << "Hex vertex NW: " << vNW << endl;


    float vto_ne = d/(2.0f * morph::mathconst<float>::sqrt_of_3);
    // Test the numbers (non-exhaustive)
    if (vN[0] == 0.0f
        && vNE[1] == vto_ne
        && vSW[1] == -vto_ne) {
        cout << "PASS" << endl;
        return 0;
    }

    cout << "FAIL" << endl;
    return 1;
}
