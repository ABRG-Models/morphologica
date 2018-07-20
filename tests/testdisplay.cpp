#include "display.h"
#include "tools.h"
#include <utility>
#include <iostream>
#include <unistd.h>

using namespace std;

int main()
{
    int rtn = 0;

    morph::Gdisplay d(600, "testdisplay", 0.0, 0.0, 0.0);
    vector<double> fix(3, 0.0);
    vector<double> eye(3, 0.0);
    eye[2] = -0.4;
    vector<double> rot(3, 0.0);

    d.resetDisplay (fix, eye, rot);

    array<float,3> cl_a = morph::Tools::getJetColorF (0.98);
    array<float,3> pos = { { 0, 0, 0} };
    d.drawHex (pos, 0.5, cl_a);

    d.redrawDisplay();

    cout << "Sleep a while before closing display..." << endl;
    unsigned int i = 0;
    while (i++ < 6) {
        usleep (1000000); // one second
    }

    d.closeDisplay();

    return rtn;
}
