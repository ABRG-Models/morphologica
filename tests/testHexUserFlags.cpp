#include "morph/Hex.h"
#include <iostream>
#include <utility>

using namespace morph;
using namespace std;

int main()
{
    int rtn = 0;

    int r = 0;
    int g = 0;
    float d = 2.0f;
    unsigned int idx = 0;
    Hex h(idx, d, r, g);

    cout << "User flags 0-3: " << h.getUserFlag(0) << "," << h.getUserFlag(1)
         << "," << h.getUserFlag(2) << "," << h.getUserFlag(3) << " (init)" << endl;
    if (h.getUserFlag(2) == true) {
        rtn -= 1;
    }

    h.setUserFlag(2);
    cout << "User flags 0-3: " << h.getUserFlag(0) << "," << h.getUserFlag(1)
         << "," << h.getUserFlag(2) << "," << h.getUserFlag(3) << " (set)" << endl;
    if (h.getUserFlag(2) == false) {
        rtn -= 1;
    }

    h.unsetUserFlag(2);
    cout << "User flags 0-3: " << h.getUserFlag(0) << "," << h.getUserFlag(1)
         << "," << h.getUserFlag(2) << "," << h.getUserFlag(3) << " (unset)" << endl;
    if (h.getUserFlag(2) == true) {
        rtn -= 1;
    }

    h.unsetUserFlag(2);
    cout << "User flags 0-3: " << h.getUserFlag(0) << "," << h.getUserFlag(1)
         << "," << h.getUserFlag(2) << "," << h.getUserFlag(3)  << " (unset again)" << endl;
    if (h.getUserFlag(2) == true) {
        rtn -= 1;
    }

    h.setUserFlags (HEX_USER_FLAG_0 | HEX_USER_FLAG_3);
    cout << "User flags 0-3: " << h.getUserFlag(0) << "," << h.getUserFlag(1)
         << "," << h.getUserFlag(2) << "," << h.getUserFlag(3) << endl;
    if (h.getUserFlag(0) == false || h.getUserFlag(3) == false) {
        rtn -= 1;
    }

    h.resetUserFlags();
    cout << "User flags 0-3: " << h.getUserFlag(0) << "," << h.getUserFlag(1)
         << "," << h.getUserFlag(2) << "," << h.getUserFlag(3) << endl;
    if (h.getUserFlag(0) == true || h.getUserFlag(3) == true) {
        rtn -= 1;
    }

    if (rtn != 0) {
        cout << "FAIL" << endl;
    }
    return rtn;
}
