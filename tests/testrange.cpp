#include <iostream>
#include <morph/range.h>
#include <morph/mathconst.h>

int main()
{
    int rtn = 0;

    morph::range<float> r(2.0f, 4.0f);
    if (r.update (1.0f) == false) { --rtn; } // Update with 1 should change the range and return true
    if (r.update (5.0f) == false) { --rtn; } // Update with 5 should change the range and return true
    if (r.update (3.0f) == true) { --rtn; } // Update with 3 should not change the range

    std::cout << "Test " << (rtn == 0 ? "Passed" : "Failed") << std::endl;
    return rtn;
}
