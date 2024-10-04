/*
 * Showing hue-variable 1D colour maps.
 */

#include <iostream>
#include <vector>
#include <string>
#include <format>
#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/ColourBarVisual.h>
#include <morph/Grid.h>
#include <morph/GridVisual.h>

int main()
{
    // Contructor args are width, height, title, coordinate arrows offset, cooridnate
    // arrows lengths, coord arrow thickness, coord arrow font size (0 means no labels)
    std::string title_str = "ColourMaps";
    morph::Visual v(1000, 1200, title_str);
    v.setSceneTrans (morph::vec<float,3>{ float{-1.11157}, float{0.762484}, float{-5.7} });

    morph::Scale<float> scale1;
    scale1.set_input_range (0, 1); // Simply maps 0->1 to 0->1!

    morph::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // Hue-variable 1D maps
    std::vector<morph::ColourMapType> cmap_types;
    cmap_types.push_back (morph::ColourMapType::Monochrome);
    cmap_types.push_back (morph::ColourMapType::Monochrome);
    cmap_types.push_back (morph::ColourMapType::Monochrome);
    cmap_types.push_back (morph::ColourMapType::Monochrome);
    cmap_types.push_back (morph::ColourMapType::Monochrome);
    cmap_types.push_back (morph::ColourMapType::Monochrome);
    cmap_types.push_back (morph::ColourMapType::Monoval);
    cmap_types.push_back (morph::ColourMapType::Monoval);
    cmap_types.push_back (morph::ColourMapType::Monoval);
    cmap_types.push_back (morph::ColourMapType::Monoval);
    cmap_types.push_back (morph::ColourMapType::Monoval);
    cmap_types.push_back (morph::ColourMapType::Monoval);
    cmap_types.push_back (morph::ColourMapType::HSV1D);
    cmap_types.push_back (morph::ColourMapType::HSV1D);
    cmap_types.push_back (morph::ColourMapType::HSV1D);
    cmap_types.push_back (morph::ColourMapType::HSV1D);
    cmap_types.push_back (morph::ColourMapType::HSV1D);
    cmap_types.push_back (morph::ColourMapType::HSV1D);

    morph::ColourMap<float> cm1;

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
        float hue = offset[1]*(-0.05555555555f) + offset[0] / 2.4f; // Use x value to set hue
        cbv->cm.setHue (hue);
        cbv->scale = scale1;
        // morph::ColourMap<float>::colourMapTypeToStr (cmap_type)
        cbv->addLabel (std::format("hue={:.2f}", hue), {0, -0.1, 0}, morph::TextFeatures(0.05f));
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
