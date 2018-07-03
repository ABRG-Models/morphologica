#include "ReadCurves.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;
using morph::ReadCurves;

#define DEBUG 1
#define DBGSTREAM std::cout
#include "MorphDbg.h"

int main()
{
    int rtn = -1;

    try {
        ReadCurves r("../tests/trial.svg");
        r.save (2.3f);
        rtn = 0;

    } catch (const exception& e) {
        cerr << "Caught exception reading trial.svg: " << e.what() << endl;
        rtn = -1;
    }

    return rtn;
}
