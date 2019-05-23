#include "Visual.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;
using morph::Visual;

int main()
{
    int rtn = -1;

    Visual v(800,600,"Test window");

    cout << "Enter key to end" << endl;

    int a;
    cin >> a;

    return rtn;
}
