/*
 * Showing the CET colourmaps
 */

#include <iostream>
#include <vector>
#include <string>
#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/ColourBarVisual.h>

int main()
{
    // Contructor args are width, height, title, coordinate arrows offset, cooridnate
    // arrows lengths, coord arrow thickness, coord arrow font size (0 means no labels)
    std::string title_str = "ColourMaps from CET";
    morph::Visual v(1000, 1400, title_str);
    v.setSceneTrans (morph::vec<float,3>{ float{-1.17245}, float{1.24502}, float{-7.7} });

    morph::Scale<float> scale1;
    scale1.compute_scaling (0, 1); // Simply maps 0->1 to 0->1!

    morph::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // 1D maps
    std::vector<morph::ColourMapType> cmap_types;
    cmap_types.push_back (morph::ColourMapType::CET_L02);
    cmap_types.push_back (morph::ColourMapType::CET_L13);
    cmap_types.push_back (morph::ColourMapType::CET_C4);
    cmap_types.push_back (morph::ColourMapType::CET_D04);
    cmap_types.push_back (morph::ColourMapType::CET_L12);
    cmap_types.push_back (morph::ColourMapType::CET_C1s);
    cmap_types.push_back (morph::ColourMapType::CET_L01);
    cmap_types.push_back (morph::ColourMapType::CET_C5);
    cmap_types.push_back (morph::ColourMapType::CET_D11);
    cmap_types.push_back (morph::ColourMapType::CET_L04);
    cmap_types.push_back (morph::ColourMapType::CET_CBL2);
    cmap_types.push_back (morph::ColourMapType::CET_C4s);
    cmap_types.push_back (morph::ColourMapType::CET_L15);
    cmap_types.push_back (morph::ColourMapType::CET_L20);
    cmap_types.push_back (morph::ColourMapType::CET_CBD1);
    cmap_types.push_back (morph::ColourMapType::CET_D06);
    cmap_types.push_back (morph::ColourMapType::CET_I3);
    cmap_types.push_back (morph::ColourMapType::CET_D01A);
    cmap_types.push_back (morph::ColourMapType::CET_L16);
    cmap_types.push_back (morph::ColourMapType::CET_L06);
    cmap_types.push_back (morph::ColourMapType::CET_C2s);
    cmap_types.push_back (morph::ColourMapType::CET_I1);
    cmap_types.push_back (morph::ColourMapType::CET_C7s);
    cmap_types.push_back (morph::ColourMapType::CET_I2);
    cmap_types.push_back (morph::ColourMapType::CET_C6s);
    cmap_types.push_back (morph::ColourMapType::CET_C6);
    cmap_types.push_back (morph::ColourMapType::CET_L05);
    cmap_types.push_back (morph::ColourMapType::CET_D08);
    cmap_types.push_back (morph::ColourMapType::CET_L03);
    cmap_types.push_back (morph::ColourMapType::CET_L14);
    cmap_types.push_back (morph::ColourMapType::CET_C2);
    cmap_types.push_back (morph::ColourMapType::CET_R3);
    cmap_types.push_back (morph::ColourMapType::CET_D01);
    cmap_types.push_back (morph::ColourMapType::CET_C1);
    cmap_types.push_back (morph::ColourMapType::CET_D02);
    cmap_types.push_back (morph::ColourMapType::CET_CBC1);
    cmap_types.push_back (morph::ColourMapType::CET_D09);
    cmap_types.push_back (morph::ColourMapType::CET_L10);
    cmap_types.push_back (morph::ColourMapType::CET_R1);
    cmap_types.push_back (morph::ColourMapType::CET_C3);
    cmap_types.push_back (morph::ColourMapType::CET_CBL1);
    cmap_types.push_back (morph::ColourMapType::CET_C3s);
    cmap_types.push_back (morph::ColourMapType::CET_C5s);
    cmap_types.push_back (morph::ColourMapType::CET_L08);
    cmap_types.push_back (morph::ColourMapType::CET_R4);
    cmap_types.push_back (morph::ColourMapType::CET_R2);
    cmap_types.push_back (morph::ColourMapType::CET_L11);
    cmap_types.push_back (morph::ColourMapType::CET_D10);
    cmap_types.push_back (morph::ColourMapType::CET_D07);
    cmap_types.push_back (morph::ColourMapType::CET_L17);
    cmap_types.push_back (morph::ColourMapType::CET_D12);
    cmap_types.push_back (morph::ColourMapType::CET_CBC2);
    cmap_types.push_back (morph::ColourMapType::CET_D13);
    cmap_types.push_back (morph::ColourMapType::CET_D03);
    cmap_types.push_back (morph::ColourMapType::CET_C7);
    cmap_types.push_back (morph::ColourMapType::CET_L07);
    cmap_types.push_back (morph::ColourMapType::CET_L09);
    cmap_types.push_back (morph::ColourMapType::CET_L18);
    cmap_types.push_back (morph::ColourMapType::CET_L19);

    morph::ColourMap<float> cm1(morph::ColourMapType::Plasma);

    // Display 1D colour maps
    int i = 0;
    for (auto cmap_type : cmap_types) {
        ++i;
        cm1.setType (cmap_type);
        auto cbv =  std::make_unique<morph::ColourBarVisual<float>>(offset);
        v.bindmodel (cbv);
        cbv->orientation = morph::colourbar_orientation::vertical;
        cbv->tickside = morph::colourbar_tickside::right_or_below;
        cbv->cm = cm1;
        cbv->scale = scale1;
        cbv->addLabel (morph::ColourMap<float>::colourMapTypeToStr (cmap_type), {0, -0.1, 0}, morph::TextFeatures(0.05f));
        cbv->finalize();
        v.addVisualModel (cbv);
        // Update location
        offset[0] += 0.4f;
        if (i % 6 == 0) {
            offset[0] = 0.0f;
            offset[1] -= 1.0f;
        }
    }

    v.keepOpen();

    return 0;
}
