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
    scale1.compute_autoscale (0, 1); // Simply maps 0->1 to 0->1!

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

    cmap_types.clear();
    cmap_types.push_back (morph::ColourMapType::Monochrome);
    cmap_types.push_back (morph::ColourMapType::Monochrome);
    cmap_types.push_back (morph::ColourMapType::Monochrome);
    cmap_types.push_back (morph::ColourMapType::Monochrome);
    cmap_types.push_back (morph::ColourMapType::Monoval);
    cmap_types.push_back (morph::ColourMapType::Monoval);

    std::array<float, 3> purple = morph::vec<int, 3>({0x68, 0x31, 0x92}).as_float()/255.0f;
    std::array<float, 3> orange = morph::vec<int, 3>({0xdf, 0x5e, 0x26}).as_float()/255.0f;
    std::array<float, 3> blue = morph::vec<int, 3>({0x2a, 0x37, 0x91}).as_float()/255.0f;
    std::array<float, 3> green = morph::vec<int, 3>({0x5b, 0x89, 0x3d}).as_float()/255.0f;
    //std::array<float, 3> purple2 = morph::vec<int, 3>({0xa4, 0x84, 0xbc}).as_float()/255.0f;
    std::array<float, 3> orange2 = morph::vec<int, 3>({0xee, 0x9f, 0x7d}).as_float()/255.0f;

    std::vector<std::array<float, 3>> clrs;
    clrs.push_back (purple);
    clrs.push_back (orange);
    clrs.push_back (green);
    clrs.push_back (blue);
    clrs.push_back (orange);
    clrs.push_back (orange2);

    int j = 0;
    for (auto cmap_type : cmap_types) {
        ++i;
        cm1.setType (cmap_type);
        cm1.setRGB (clrs[j++]);
        auto cbv =  std::make_unique<morph::ColourBarVisual<float>>(offset);
        v.bindmodel (cbv);
        cbv->orientation = morph::colourbar_orientation::vertical;
        cbv->tickside = morph::colourbar_tickside::right_or_below;
        cbv->cm = cm1;
        cbv->scale = scale1;
        // morph::ColourMap<float>::colourMapTypeToStr (cmap_type)
        cbv->addLabel (std::format("hue={:.2f}", cbv->cm.getHue()), {0, -0.1, 0}, morph::TextFeatures(0.05f));
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
