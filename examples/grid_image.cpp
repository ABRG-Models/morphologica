#include <iostream>
#include <vector>
#include <cmath>

#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/loadpng.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/GridVisual.h>
#include <morph/Grid.h>

int main()
{
    morph::Visual v(1600, 1000, "Demo of Grid showing an image");

    morph::vec<float, 2> dx = { 0.02f, 0.02f };
    morph::vec<float, 2> nul = { 0.0f, 0.0f };
    // Top left to bottom right order matches image loaded by loadpng and avoids the need for a
    // vec<bool, 2> flip arg to morph::loadpng.
    morph::Grid g1(256U, 65U, dx, nul, morph::GridDomainWrap::Horizontal, // Triangles, TLBR
                   morph::GridOrder::topleft_to_bottomright);

    morph::Grid g2(256U, 65U, dx, nul, morph::GridDomainWrap::Horizontal, // Triangles, BLTR
                   morph::GridOrder::bottomleft_to_topright);

    morph::Grid g3(256U, 65U, dx, nul, morph::GridDomainWrap::Horizontal, // RectInterp TLBR
                   morph::GridOrder::topleft_to_bottomright);

    morph::Grid g4(256U, 65U, dx, nul, morph::GridDomainWrap::Horizontal, // RectInterp, BLTR
                   morph::GridOrder::bottomleft_to_topright);

    // Load an image
    std::string fn = "../examples/bike256_65.png";
    morph::vvec<float> image_data_tlbr;
    morph::vec<unsigned int, 2> dims = morph::loadpng (fn, image_data_tlbr, morph::vec<bool, 2>{false,false});
    morph::vvec<float> image_data_bltr;
    dims = morph::loadpng (fn, image_data_bltr);
    std::cout << "Image dims: " << dims << std::endl;

    // Now visualise with a GridVisual
    auto gv1 = std::make_unique<morph::GridVisual<float>>(&g1, morph::vec<float>({0,0,0}));
    v.bindmodel (gv1);
    gv1->gridVisMode = morph::GridVisMode::Triangles;
    gv1->setScalarData (&image_data_tlbr);
    gv1->cm.setType (morph::ColourMapType::GreyscaleInv); // inverse greyscale is good for a monochrome image
    gv1->zScale.setParams (0, 1); // As it's an image, we don't want relief, so set the zScale to have a zero gradient
    gv1->finalize();
    v.addVisualModel (gv1);

    auto gv2 = std::make_unique<morph::GridVisual<float>>(&g2, morph::vec<float>({6,0,0}));
    v.bindmodel (gv2);
    gv2->gridVisMode = morph::GridVisMode::Triangles;
    gv2->setScalarData (&image_data_bltr);
    gv2->cm.setType (morph::ColourMapType::GreyscaleInv);
    gv2->zScale.setParams (0, 1);
    gv2->finalize();
    v.addVisualModel (gv2);

    auto gv3 = std::make_unique<morph::GridVisual<float>>(&g3, morph::vec<float>({0,1.6,0}));
    v.bindmodel (gv3);
    gv3->gridVisMode = morph::GridVisMode::RectInterp;
    gv3->setScalarData (&image_data_tlbr);
    gv3->cm.setType (morph::ColourMapType::GreyscaleInv);
    gv3->zScale.setParams (0, 1);
    gv3->finalize();
    v.addVisualModel (gv3);

    auto gv4 = std::make_unique<morph::GridVisual<float>>(&g4, morph::vec<float>({6,1.6,0}));
    v.bindmodel (gv4);
    gv4->gridVisMode = morph::GridVisMode::RectInterp;
    gv4->setScalarData (&image_data_bltr);
    gv4->cm.setType (morph::ColourMapType::GreyscaleInv);
    gv4->zScale.setParams (0, 1);
    gv4->finalize();
    v.addVisualModel (gv4);

    v.keepOpen();

    return 0;
}
