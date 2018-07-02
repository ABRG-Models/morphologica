#include "ReadCurves.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;
using morph::ReadCurves;

int main()
{
    int rtn = -1;

    try {
        ReadCurves r("../tests/trial.svg");
        rtn = 0;

    } catch (const exception& e) {
        cerr << "Caught exception reading trial.svg: " << e.what() << endl;
        rtn = -1;
    }

    return rtn;
}
