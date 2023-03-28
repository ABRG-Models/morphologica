#include <morph/CartGrid.h>
#include <morph/vvec.h>
#include <iostream>

int main()
{
    int rtn = 0;

    // A symmetric, zero-centered CartGrid
    morph::CartGrid cg(2.0f, 8.0f);
    cg.setBoundaryOnOuterEdge();
    std::cout << "Set up " << cg.num() << " pixels in a Cartesian grid of width/depth: " << cg.width() << "/" << cg.depth() << std::endl;

    for (auto a : cg.rects) {
        std::cout << a.outputCart() << std::endl;
    }
    if (cg.num() != 25) { --rtn; }

    // A CartGrid with 400 elements
    morph::CartGrid cg2(0.05f, 0.05f, 0.0f, 0.0f, 0.95f, 0.95f);
    cg2.setBoundaryOnOuterEdge();
    std::cout << "Set up " << cg2.num() << " pixels in a Cartesian grid of width/depth: " << cg2.width() << "/" << cg2.depth() << std::endl;
    //for (auto a : cg2.rects) {
    //    std::cout << a.outputCart() << std::endl;
    //}

    // A CartGrid with a few elements
    morph::CartGrid cg3(0.05f, 0.05f, 0.0f, 0.0f, 0.2f, 0.2f);
    cg3.setBoundaryOnOuterEdge();
    std::cout << "Set up " << cg3.num() << " pixels in a Cartesian grid of width/depth: " << cg3.width() << "/" << cg3.depth() << std::endl;
    for (auto a : cg3.rects) {
        std::cout << a.outputCart() << std::endl;
    }

    morph::vvec<float> vals = { 1, 2, 3, 2, 1,   4, 5, 6, 7, 4,   7, 4, 2, 1, 4,   8, 8, 6, 8, 3,   9, 8, 3, 2, 1  };

    morph::vvec<float> filtered (25, 0);

    morph::vvec<float> expect_result = { 17, 21, 25, 23, 19,  32, 34, 32, 30, 31,  47, 50, 47, 41, 46,  52, 55, 42, 30, 43,  37, 42, 35, 23, 31  };
    expect_result /= 9.0f;

    cg3.boxfilter_f<float, 3> (vals, filtered);

    std::cout << "Box filter result:\n" << filtered << std::endl;
    std::cout << "  expected result:\n" << expect_result << std::endl;

    if (filtered.sum() != expect_result.sum()) {
        std::cout << "filtered sum " << filtered.sum() << std::endl;
        std::cout << "expected sum " << expect_result.sum() << std::endl;
        --rtn;
    }

    std::cout << "At end rtn is " << rtn;

    return rtn;
}
