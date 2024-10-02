/*
 * Make a healpix visual, showing the NEST index in a colour map
 */
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <morph/Visual.h>
#include <morph/HealpixVisual.h>

int main (int argc, char** argv)
{
    int ord = 7; // HEALPix order
    if (argc > 1) { ord = std::atoi (argv[1]); }

    morph::Visual v(1024, 768, "Healpix");

    auto hpv = std::make_unique<morph::HealpixVisual<float>> (morph::vec<float>{0,0,0});
    v.bindmodel (hpv);
    hpv->set_order (ord);
    hpv->cm.setType (morph::ColourMapType::Plasma);

    // The HealpixVisual has pixeldata, which is ordered with the NEST indexing
    // scheme. If we fill it with sequential values, then the colour map will show the
    // hierarchical nature of the HEALPix.
    for (int64_t p = 0; p < hpv->n_pixels(); ++p) {
        hpv->pixeldata[p] = static_cast<float>(p) / hpv->n_pixels();
    }

    std::stringstream ss;
    constexpr bool centre_horz = true;
    ss << ord << (ord == 1 ? "st" : (ord == 2 ? "nd" : (ord == 3 ? "rd" : "th")))
       << " order HEALPix with nside = " << hpv->get_nside()
       << " and " << hpv->n_pixels() << " pixels\n";
    hpv->addLabel (ss.str(), {0.0f, -1.2f , 0.0f }, morph::TextFeatures{0.08f, centre_horz});

    // Finalize and add
    hpv->finalize();
    v.addVisualModel (hpv);


    v.keepOpen();
    return 0;
}
