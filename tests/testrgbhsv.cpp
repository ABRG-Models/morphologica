#include <limits>
#include <array>
#include <iostream>
#include "morph/ColourMap.h"

int main ()
{
    int rtn = 0;
    morph::vec<float, 3> rgb_in = { 1, 0.727f, 0.339f };
    morph::vec<float, 3> hsv = morph::ColourMap<float>::rgb2hsv_vec (rgb_in);
    std::cout << "rgb: " << rgb_in << " maps to hsv: " << hsv << std::endl;
    return rtn;
}
