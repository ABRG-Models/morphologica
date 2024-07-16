/*
 * Visualize a Grating
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/GratingVisual.h>
#include <morph/vec.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <stdexcept>
#include <string>

struct myvisual final : public morph::Visual<>
{
    // Boilerplate constructor (just copy this):
    myvisual (int width, int height, const std::string& title,
              const morph::vec<float, 2> caOffset = {-0.8,-0.8},
              const morph::vec<float> caLength = {.05,.05,.05},
              const float caThickness = 2.0f, const float caEm = 0.0f)
        : morph::Visual<> (width, height, title, caOffset, caLength, caThickness, caEm) {}

    // Angle of the bands
    float angle = 45.0f;
    // Time
    unsigned long long int t = 0;
    // Flag if we should reinit the model
    bool needs_reinit = false;

protected:
    void key_callback_extra (int key, [[maybe_unused]]int scancode, int action, [[maybe_unused]]int mods) override
    {
        if (action == morph::keyaction::press || action == morph::keyaction::repeat) {
            if (key == morph::key::w) {
                this->angle += 1.0f;
                this->needs_reinit = true;
            } else if (key == morph::key::s) {
                this->angle -= 1.0f;
                this->needs_reinit = true;
            } else if (key == morph::key::a) {
                this->t = this->t > 0 ? this->t - 1 : 0;
                this->needs_reinit = true;
            } else if (key == morph::key::d) {
                this->t = this->t + 1;
                this->needs_reinit = true;
            }
            std::cout << "Keyboard update: Angle: " << angle << " time: " << this->t << std::endl;
        }
    }
};


int main (int ac, char** av)
{
    int rtn = -1;
    myvisual v(1024, 768, "Grating");
    v.setSceneTrans (morph::vec<float,3>({-0.990124f, -0.452241f, -3.6f}));

    if (ac > 1) { v.t = std::atoi (av[1]); }
    if (ac > 2) { v.angle = std::atoi (av[2]); }

    constexpr bool interactive = true;

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };

        auto rvm = std::make_unique<morph::GratingVisual<>> (offset);
        v.bindmodel (rvm);
        rvm->v_front = { 0.02f, 0.02f };
        rvm->t = v.t;
        rvm->lambda = .1;
        rvm->alpha = v.angle;
        rvm->finalize();
        auto rvmp = v.addVisualModel (rvm);

        if constexpr (interactive) {
            while (!v.readyToFinish) {
                v.waitevents (0.018);
                v.render();
                if (v.needs_reinit == true) {
                    rvmp->t = v.t;
                    rvmp->alpha = v.angle;
                    rvmp->reinit();
                    v.needs_reinit = false;
                }
            }
        } else {
            v.keepOpen();
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
