/*
 * Visualize a single vector
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/VectorVisual.h>
#include <morph/vec.h>
#include <morph/Quaternion.h>
#include <morph/TransformMatrix.h>
#include <iostream>
#include <array>
#include <stdexcept>
#include <string>

int main()
{
    morph::Visual v(1024, 768, "morph::VectorVisual", {0,0}, {.5,.5,.5}, 1.0f, 0.05f);
    v.lightingEffects();
    v.showCoordArrows = true;
    v.coordArrowsInScene = true;

    morph::vec<float> offset = {1,0,0};

    auto vvm = std::make_unique<morph::VectorVisual<float, 3>>(offset);
    v.bindmodel (vvm);
    vvm->thevec = {1,1,1};
    vvm->fixed_colour = true;
    vvm->single_colour = morph::colour::crimson;
    vvm->addLabel ("Rotn by Quaternion", {-0.8, -0.5, 0}, morph::TextFeatures(0.1f));
    vvm->finalize();
    auto ptr = v.addVisualModel (vvm);

    vvm = std::make_unique<morph::VectorVisual<float, 3>>(-offset);
    v.bindmodel (vvm);
    vvm->thevec = {1,1,1};
    vvm->fixed_colour = true;
    vvm->single_colour = morph::colour::royalblue;
    vvm->addLabel ("Rotn by TransformMatrix", {-0.8, -0.5, 0}, morph::TextFeatures(0.1f));
    vvm->finalize();
    auto ptr2 = v.addVisualModel (vvm);

    float angle_per_frame = 0.05f;
    morph::vec<float> axis = {0,1,0.4};

    // Quaternion way
    // Also demo quaternion rotation.
    // Set up a rotation about the z axis
    morph::Quaternion<float> qr (axis, angle_per_frame);

    morph::TransformMatrix<float> tf;
    tf.rotate (axis, angle_per_frame);

    while (!v.readyToFinish) {
        v.render();
        v.wait (0.01);

        ptr->thevec = (qr * ptr->thevec);
        ptr2->thevec = (tf * ptr2->thevec).less_one_dim();

        ptr->reinit();
        ptr2->reinit();
    }

    return 0;
}
