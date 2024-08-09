/*
 * Use cartgridvisuals to illustrate use of ColourMapType::HSV colourmap.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/HSVWheelVisual.h>

// In this example, I'll create a special visual to show hsv colours.
struct SquareGridVisual : public morph::VisualModel<>
{
    SquareGridVisual(const morph::vec<float> _offset, const float hue_rotn = 0.0f, const bool rev = false)
        : morph::VisualModel<> (_offset)
    {
        // In the constructor set up the colour map
        this->colourMap.setType (morph::ColourMapType::HSV);

        // I think it's easier to think of rotating the hues by some number of radians
        this->colourMap.setHueRotation (hue_rotn);

        this->colourMap.setHueReverse (rev);
    }

    // A colourMap member attribute
    morph::ColourMap<float> colourMap;

    // initializeVertices is the standard function that we override when extending
    // VisualModel. Here, we're going to draw a grid of squares, with colour chosen
    // using their x and y coordinates
    void initializeVertices() override
    {
        // Some compile-time values used in the loop. The first says draw a square polygon with 4 sides. Change to 5 and see what happens.
        static constexpr int square_has_four_segments = 4;
        // Whats the distance from the centre of one square in the grid to the next?
        static constexpr float element_to_element_distance = 1.0f;
        // In a 'flat polygon', The radius is defined as the distance to a vertex, so,
        // for example, to get a square of width 2, set the radius to sqrt(2). Here, I use 0.97f to ensure a thin gap between squares
        static constexpr float square_centre_to_vertex = 0.97f * (element_to_element_distance / 2.0f) * morph::mathconst<float>::root_2;
        // Polygons have a vertex pointing 'up' by default, so we have to rotate
        static constexpr float square_needs_rotation = morph::mathconst<float>::pi_over_4;
        // How many squares along a side of the grid?
        static constexpr int num_elements_on_side = 12;
        // That number - 1:
        static constexpr float num_elements_less_one = num_elements_on_side - 1.0f;

        // Loop through, creating a grid of squares:
        for (int x = 0; x < num_elements_on_side; x++) {
            for (int y = 0; y < num_elements_on_side; y++) {

                // Create grid element position from x and y.
                morph::vec<float> element_pos = { static_cast<float>(x), static_cast<float>(y), 0.0f };
                element_pos *= element_to_element_distance;

                // We call the 2 argument overload of ColourMap::convert, making sure
                // that the range of each argument is [0,1] (hence the
                // divisions). Inside ColourMap::convert, the two numbers are treated as
                // coordinates and turned into an angle about (0.5,0.5) and a
                // radius. The angle becomes the hue and the radius the saturation. The
                // 'value' for the hsv colour is 1 by default. This can be modified by
                // calling ColourMap::setVal(). The starting colour for the hue is red,
                // which can also be modified, using setHue or setHueRotation.
                std::array<float, 3> element_colour = this->colourMap.convert (x/num_elements_less_one, y/num_elements_less_one);

                // We use a 'flat poly' primitive to draw a square. ux and uy are unit vectors that control the orientation of the polygon
                this->computeFlatPoly (element_pos, this->ux, this->uy, element_colour,
                                       square_centre_to_vertex, square_has_four_segments, square_needs_rotation);
            }
        }
    }
};

int main()
{
    // The main function is simple. Create a morph::Visual, add a single SquareGridVisual and then 'keep it open'
    morph::Visual v(1600, 1000, "The HSV colour map with 2D inputs", {-0.8,-0.8}, {.05,.05,.05}, 2.0f, 0.0f);
    v.backgroundBlack();
    v.setSceneTrans (-5.60868263,-5.17123413,-29.2000771); // numbers obtained by pressing 'z' and seeing stdout

    morph::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // TextFeatures is a nice way to specify font size, colour (and other things) for your addLabel() calls.
    morph::TextFeatures tf (0.5f, morph::colour::white);

    // Grid 1
    float hue_rotn = 0.0f;
    auto hsv_vis = std::make_unique<SquareGridVisual>(offset, hue_rotn);
    v.bindmodel (hsv_vis);
    //
    std::string lbl("hue rotation = 0");
    hsv_vis->addLabel (lbl, morph::vec<float>({0,-1,0}), tf);
    //
    hsv_vis->finalize();
    auto hsv_visp = v.addVisualModel (hsv_vis);

    // HSVWHeel for Grid1
    morph::vec<float, 3> woffset = offset;
    woffset[0] += 5.5f;
    woffset[1] -= 6.0f;
    auto hsvw_vis = std::make_unique<morph::HSVWheelVisual<float>>(woffset);
    v.bindmodel (hsvw_vis);
    hsvw_vis->setColour (morph::colour::white);
    hsvw_vis->radius = 3.5f;
    hsvw_vis->tf.fontsize = 0.4f;
    hsvw_vis->twodimensional = false;
    hsvw_vis->cm = hsv_visp->colourMap;
    hsvw_vis->finalize();
    v.addVisualModel (hsvw_vis);

    offset[0] = -14.0f;
    hue_rotn = -morph::mathconst<float>::pi_over_2;
    auto hsv_vis2 = std::make_unique<SquareGridVisual>(offset, hue_rotn);
    v.bindmodel (hsv_vis2);
    //
    using morph::unicode;
    std::string lbl2("hue rotation = ");
    lbl2 += std::to_string (hue_rotn/morph::mathconst<float>::pi) + unicode::toUtf8(unicode::pi);
    hsv_vis2->addLabel (lbl2, morph::vec<float>({0,-1,0}), tf);
    //
    hsv_vis2->finalize();
    auto hsv_vis2p = v.addVisualModel (hsv_vis2);

    // HSVwheel
    woffset = offset;
    woffset[0] += 5.5f;
    woffset[1] -= 6.0f;
    hsvw_vis = std::make_unique<morph::HSVWheelVisual<float>>(woffset);
    v.bindmodel (hsvw_vis);
    hsvw_vis->setFrameColour (morph::colour::teal);
    hsvw_vis->setTextColour (morph::colour::white);
    hsvw_vis->framelinewidth = 0.1f;
    hsvw_vis->radius = 3.5f;
    hsvw_vis->tf.fontsize = 0.4f;
    hsvw_vis->twodimensional = false;
    hsvw_vis->cm = hsv_vis2p->colourMap;
    hsvw_vis->finalize();
    v.addVisualModel (hsvw_vis);


    offset[0] = 14.0f;
    hue_rotn = 0.0f;
    auto hsv_vis3 = std::make_unique<SquareGridVisual>(offset, hue_rotn, true);
    v.bindmodel (hsv_vis3);
    //
    std::string lbl3("hue rotation = 0; direction reversed");
    hsv_vis3->addLabel (lbl3, morph::vec<float>({0,-1,0}), tf);
    //
    hsv_vis3->finalize();
    auto hsv_vis3p = v.addVisualModel (hsv_vis3);

    woffset = offset;
    woffset[0] += 5.5f;
    woffset[1] -= 6.0f;
    hsvw_vis = std::make_unique<morph::HSVWheelVisual<float>>(woffset);
    v.bindmodel (hsvw_vis);
    hsvw_vis->setColour (morph::colour::white);
    hsvw_vis->labels = {"Fwds", "FL", "Left", "BL", "Back", "BR", "Right", "FR"};
    hsvw_vis->framelinewidth = 0.2f;
    hsvw_vis->radius = 3.5f;
    hsvw_vis->tf.fontsize = 0.4f;
    hsvw_vis->twodimensional = false;
    hsvw_vis->cm = hsv_vis3p->colourMap;
    hsvw_vis->finalize();
    v.addVisualModel (hsvw_vis);

    v.keepOpen();
    return 0;
}
