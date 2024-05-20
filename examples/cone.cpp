/*
 * Visualize a Cone
 */
#include <morph/Visual.h>
#include <morph/ConeVisual.h>
#include <morph/vec.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

int main()
{
    int rtn = 0;

    morph::Visual v(1024, 768, "A simple cone");
    v.lightingEffects(true);

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };

        // Draw several cones, demonstrating what 'ringoffset' does
        for (int i = 0; i < 6; ++i) {
            auto cvm = std::make_unique<morph::ConeVisual<>> (offset);
            v.bindmodel (cvm);
            cvm->ringoffset = 0.2f * i;
            cvm->clr = { (5-i) * 0.2f, 0.0f, i * 0.2f };
            std::stringstream ss;
            ss << "ringoffset = " << cvm->ringoffset;
            cvm->addLabel (ss.str(), { 0.0f, 0.3f, 0.0f }, morph::TextFeatures(0.05f));
            cvm->finalize();
            v.addVisualModel (cvm);

            offset[1] += 0.75f;
        }
        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
