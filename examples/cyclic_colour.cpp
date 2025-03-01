/*
 * Use cartgridvisuals to illustrate use of ColourMapType::HSV colourmap.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/CyclicColourVisual.h>

int main()
{
    // The main function is simple. Create a morph::Visual, add a single SquareGridVisual and then 'keep it open'
    std::string titlestr = "Cyclic colour map";
    morph::Visual v(1600, 1000, titlestr);
    v.backgroundBlack();

    morph::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // TextFeatures is a nice way to specify font size, colour (and other things) for your addLabel() calls.
    morph::TextFeatures tf (0.5f, morph::colour::white);

    // HSVWHeel for Grid1
    morph::vec<float, 3> woffset = offset;
    auto hsvw_vis = std::make_unique<morph::CyclicColourVisual<float>>(woffset);
    v.bindmodel (hsvw_vis);
    hsvw_vis->setColour (morph::colour::white);
    hsvw_vis->cm = morph::ColourMapType::CET_C6;// CET_C6
    hsvw_vis->finalize();
    v.addVisualModel (hsvw_vis);

    v.keepOpen();
    return 0;
}
