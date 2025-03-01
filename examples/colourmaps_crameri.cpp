/*
 * Showing the Crameri colourmaps
 */

#include <iostream>
#include <vector>
#include <string>
#include <morph/scale.h>
#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/ColourBarVisual.h>

int main()
{
    // Contructor args are width, height, title
    std::string title_str = "ColourMaps from Fabio Crameri";
    morph::Visual v(1000, 1400, title_str);
    v.setSceneTrans (morph::vec<float,3>{ float{-1.52137}, float{1.74665}, float{-9.60001} });

    morph::scale<float> scale1;
    scale1.compute_scaling (0, 1); // Simply maps 0->1 to 0->1!

    morph::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // 1D maps
    std::vector<morph::ColourMapType> cmap_types;
    cmap_types.push_back (morph::ColourMapType::Devon);
    cmap_types.push_back (morph::ColourMapType::NaviaW);
    cmap_types.push_back (morph::ColourMapType::BrocO);
    cmap_types.push_back (morph::ColourMapType::Acton);
    cmap_types.push_back (morph::ColourMapType::Batlow);
    cmap_types.push_back (morph::ColourMapType::Berlin);
    cmap_types.push_back (morph::ColourMapType::Tofino);
    cmap_types.push_back (morph::ColourMapType::Broc);
    cmap_types.push_back (morph::ColourMapType::CorkO);
    cmap_types.push_back (morph::ColourMapType::Lapaz);
    cmap_types.push_back (morph::ColourMapType::BamO);
    cmap_types.push_back (morph::ColourMapType::Vanimo);
    cmap_types.push_back (morph::ColourMapType::Lajolla);
    cmap_types.push_back (morph::ColourMapType::Lisbon);
    cmap_types.push_back (morph::ColourMapType::GrayC);
    cmap_types.push_back (morph::ColourMapType::Roma);
    cmap_types.push_back (morph::ColourMapType::Vik);
    cmap_types.push_back (morph::ColourMapType::Navia);
    cmap_types.push_back (morph::ColourMapType::Bilbao);
    cmap_types.push_back (morph::ColourMapType::Turku);
    cmap_types.push_back (morph::ColourMapType::Lipari);
    cmap_types.push_back (morph::ColourMapType::VikO);
    cmap_types.push_back (morph::ColourMapType::BatlowK);
    cmap_types.push_back (morph::ColourMapType::Oslo);
    cmap_types.push_back (morph::ColourMapType::Oleron);
    cmap_types.push_back (morph::ColourMapType::Davos);
    cmap_types.push_back (morph::ColourMapType::Fes);
    cmap_types.push_back (morph::ColourMapType::Managua);
    cmap_types.push_back (morph::ColourMapType::Glasgow);
    cmap_types.push_back (morph::ColourMapType::Tokyo);
    cmap_types.push_back (morph::ColourMapType::Bukavu);
    cmap_types.push_back (morph::ColourMapType::Bamako);
    cmap_types.push_back (morph::ColourMapType::BatlowW);
    cmap_types.push_back (morph::ColourMapType::Nuuk);
    cmap_types.push_back (morph::ColourMapType::Cork);
    cmap_types.push_back (morph::ColourMapType::Hawaii);
    cmap_types.push_back (morph::ColourMapType::Bam);
    cmap_types.push_back (morph::ColourMapType::Imola);
    cmap_types.push_back (morph::ColourMapType::RomaO);
    cmap_types.push_back (morph::ColourMapType::Buda);

    morph::ColourMap<float> cm1(morph::ColourMapType::Acton);

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
        if (i % 8 == 0) {
            offset[0] = 0.0f;
            offset[1] -= 1.0f;
        }
    }

    v.keepOpen();

    return 0;
}
