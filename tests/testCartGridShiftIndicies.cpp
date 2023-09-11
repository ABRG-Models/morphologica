#include <morph/CartGrid.h>
#include <morph/vvec.h>
#include <iostream>

int main()
{
    int rtn = 0;

    // A symmetric, zero-centered CartGrid
    morph::CartGrid cg(1.0f, 1.0f, 4.0f, 4.0f);     // dx, dy, span x, span y
    cg.setBoundaryOnOuterEdge();
    std::cout << "Set up " << cg.num() << " pixels in a Cartesian grid of width/depth: " << cg.width() << "/" << cg.depth() << std::endl;

    for (auto a : cg.rects) {
        std::cout << a.outputCart() << std::endl;
    }
    if (cg.num() != 25) { --rtn; }

    // // A CartGrid with 400 elements
    // morph::CartGrid cg2(0.05f, 0.05f, 0.0f, 0.0f, 0.95f, 0.95f);
    // cg2.setBoundaryOnOuterEdge();
    // std::cout << "Set up " << cg2.num() << " pixels in a Cartesian grid of width/depth: " << cg2.width() << "/" << cg2.depth() << std::endl;
    // //for (auto a : cg2.rects) {
    // //    std::cout << a.outputCart() << std::endl;
    // //}

    // // A CartGrid with a few elements. Note setting of Horizontal wrap. boxfilter_f will throw exception if CartGrid is not wrapped horiontally
    // morph::CartGrid cg3(0.05f, 0.05f, 0.0f, 0.0f, 0.2f, 0.2f,
    //                     0.0f, morph::CartDomainShape::Rectangle, morph::CartDomainWrap::Horizontal);
    // cg3.setBoundaryOnOuterEdge();
    // std::cout << "Set up " << cg3.num() << " pixels in a Cartesian grid of width/depth: " << cg3.width() << "/" << cg3.depth() << std::endl;
    // for (auto a : cg3.rects) {
    //     std::cout << a.outputCart() << std::endl;
    // }

    morph::vvec<float> vals = { 0, 1, 2, 3, 4,   5, 6, 7, 8, 9,   10, 11, 12, 13, 14,   15, 16, 17, 18, 19,   20, 21, 22, 23, 24  };
    morph::vvec<int> orig = {13, 14, 8, 9};
    
    // Move so that all destinations are within the cartgrid
    morph::vvec<int> actual_result = cg.shiftIndicies(orig, -2, 1);       // -2 in the x direction.  Plus 1 in the y direction.
    morph::vvec<int> expected_result = {16, 17, 11, 12};

    if (expected_result == actual_result){
        rtn = 0;
    } else {
        rtn -= 1;
        std::cout << "Expected result " << expected_result << " not equal to actual " << actual_result  << std::endl;
    }

    std::cout << "At end rtn is " << rtn << std::endl;
    return rtn;
}
