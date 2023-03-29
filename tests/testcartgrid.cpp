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

    // A CartGrid with a few elements. Note setting of Horizontal wrap. boxfilter_f will throw exception if CartGrid is not wrapped horiontally
    morph::CartGrid cg3(0.05f, 0.05f, 0.0f, 0.0f, 0.2f, 0.2f,
                        0.0f, morph::CartDomainShape::Rectangle, morph::CartDomainWrap::Horizontal);
    cg3.setBoundaryOnOuterEdge();
    std::cout << "Set up " << cg3.num() << " pixels in a Cartesian grid of width/depth: " << cg3.width() << "/" << cg3.depth() << std::endl;
    for (auto a : cg3.rects) {
        std::cout << a.outputCart() << std::endl;
    }

    morph::vvec<float> vals = { 1, 2, 3, 2, 1,   4, 5, 6, 7, 4,   7, 4, 2, 1, 4,   8, 8, 6, 8, 3,   9, 8, 3, 2, 1  };

    morph::vvec<float> filtered (25, 0);
    morph::vvec<float> filtered_slow (25, 0);

    morph::vvec<float> expect_result = { 17, 21, 25, 23, 19,  32, 34, 32, 30, 31,  47, 50, 47, 41, 46,  52, 55, 42, 30, 43,  37, 42, 35, 23, 31  };
    expect_result /= 9.0f;

    static constexpr bool boxsum_only = false;

    cg3.boxfilter_f<float, 3, boxsum_only> (vals, filtered);
    cg3.boxfilter<float, boxsum_only> (vals, filtered_slow, 3);

    std::cout << "\nvals:\n" << vals.str(5);
    std::cout << "\n\nSlow 3x3 Box filter result:\n" << filtered_slow.str(5) << std::endl;
    std::cout << "\nFast 3x3 Box filter result:\n" << filtered.str(5) << std::endl;
    std::cout << "\nexpected result:\n" << expect_result.str(5) << std::endl;
    if (filtered.sum() != expect_result.sum()) {
        std::cout << "filtered sum " << filtered.sum() << std::endl;
        std::cout << "expected sum " << expect_result.sum() << std::endl;
        --rtn;
    }
    if (filtered.sum() != filtered_slow.sum()) { --rtn; }
#if 1
    morph::vvec<float> vals8x10 = {
        1, 2, 3, 2, 1, 1, 2, 3, 2, 1,
        4, 5, 6, 7, 4, 4, 5, 6, 7, 4,
        7, 4, 2, 1, 4, 7, 4, 2, 1, 4,
        8, 8, 6, 8, 3, 8, 8, 6, 8, 3,
        9, 8, 3, 2, 1, 9, 8, 3, 2, 1,
        4, 5, 6, 7, 4, 4, 5, 6, 7, 4,
        1, 2, 3, 2, 1, 1, 2, 3, 2, 1,
        7, 4, 2, 1, 4, 7, 4, 2, 1, 4
    };
    morph::vvec<float> filtered_5x5 (80, 0);
    morph::vvec<float> filtered_5x5_slow (80, 0);
    morph::CartGrid cg4(0.05f, 0.05f, 0.0f, 0.0f, 0.45f, 0.35f,
                        0.0f, morph::CartDomainShape::Rectangle, morph::CartDomainWrap::Horizontal);
    cg4.setBoundaryOnOuterEdge();
    std::cout << "Set up " << cg4.num() << " pixels in a Cartesian grid of width/depth: " << cg4.widthnum() << "/" << cg4.depthnum() << std::endl;

    std::cout << "\nvals8x10:\n" << vals8x10.str(10);
    cg4.boxfilter_f<float, 5, boxsum_only> (vals8x10, filtered_5x5);
    cg4.boxfilter<float, boxsum_only> (vals8x10, filtered_5x5_slow, 5);
    std::cout << "\n\nSlow 5x5 Box filter result:\n" << filtered_5x5_slow.str(10) << std::endl;
    std::cout << "\nFast 5x5 Box filter result:\n" << filtered_5x5.str(10) << std::endl;

    if (filtered_5x5.sum() != filtered_5x5_slow.sum()) { --rtn; }
#endif

#if 1
    morph::vvec<float> filtered_7x7 (80, 0);
    morph::vvec<float> filtered_7x7_slow (80, 0);

    cg4.boxfilter_f<float, 7, boxsum_only> (vals8x10, filtered_7x7);
    cg4.boxfilter<float, boxsum_only> (vals8x10, filtered_7x7_slow, 7);
    std::cout << "\n\nSlow 7x7 Box filter result:\n" << filtered_7x7_slow.str(10) << std::endl;
    std::cout << "\nFast 7x7 Box filter result:\n" << filtered_7x7.str(10) << std::endl;

    if (filtered_7x7.sum() != filtered_7x7_slow.sum()) { --rtn; }
#endif
    std::cout << "At end rtn is " << rtn << std::endl;
    return rtn;
}
