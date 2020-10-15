#include <morph/display.h>
#include <morph/tools.h>
#include <morph/ColourMap.h>
#include <utility>
#include <iostream>
#include <unistd.h>

using namespace std;

int main()
{
    if (XOpenDisplay(NULL) == (Display*)0) {
        cout << "No display, can't run test. Return 0\n";
        return 0;
    }

    unsigned int sleep_seconds = 1;
    string pwd = morph::Tools::getPwd();
    if (pwd.substr(pwd.length()-11) == "build/tests") {
        sleep_seconds = 0;
    }

    int rtn = 0;
    try {
        morph::Gdisplay d(600, "testdisplay", 0.0, 0.0, 0.0);
        vector<double> fix(3, 0.0);
        vector<double> eye(3, 0.0);
        eye[2] = -0.4;
        vector<double> rot(3, 0.0);

        d.resetDisplay (fix, eye, rot);

        array<float,3> cl_a = morph::ColourMap<float>::jetcolour (0.98);
        array<float,3> pos = { { 0, 0, 0} };
        d.drawHex (pos, 0.5, cl_a);
        d.redrawDisplay();

        cout << "Sleep " << sleep_seconds << " s before closing display..." << endl;
        unsigned int i = 0;
        while (i++ < sleep_seconds) {
            usleep (1000000); // one second
        }

        d.closeDisplay();

    } catch (const exception& e) {
        cerr << "Caught exception: " << e.what() << endl;
        rtn = -1;
    }


    return rtn;
}
