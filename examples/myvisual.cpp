/*
 * How to create your own morph::Visual to either add additional keypress actions, or to
 * override the default actions.
 */

#include <morph/gl/version.h>
#include <morph/Visual.h>

// You can choose an OpenGL version to pass as template arg to morph::Visual
constexpr int my_gl_version = morph::gl::version_4_1;

struct myvisual final : public morph::Visual<my_gl_version>
{
    // Boilerplate constructor (just copy this):
    myvisual (int width, int height, const std::string& title,
              const morph::vec<float, 2> caOffset = {-0.8,-0.8},
              const morph::vec<float> caLength = {.05,.05,.05},
              const float caThickness = 2.0f, const float caEm = 0.0f)
        : morph::Visual<my_gl_version> (width, height, title, caOffset, caLength, caThickness, caEm) {}
    // Some attributes that you might need in your myvisual scene:
    bool moving = false;
protected:
    // Optionally, override key_callback() with a much sparser function:
    bool key_callback (int key, int scancode, int action, int mods) override
    {
        // Here, I've omitted all the normal keypress actions in Visual::key_callback,
        // except for one to close the program and one for help output:
        if (key == morph::key::x && action == morph::keyaction::press) {
            std::cout << "User requested exit.\n";
            this->readyToFinish = true;
        }
        if (key == morph::key::h && action == morph::keyaction::press) {
            std::cout << "Help:\n";
            std::cout << "x: Exit program\n";
            std::cout << "h: This help\n";
        }
        // Then call the 'extra function', defined below
        this->key_callback_extra (key, scancode, action, mods);

        return false; // No need to re-render the window for either option
    }
    // Also optionally, add actions for extra keys:
    static constexpr bool debug_callback_extra = false;
    void key_callback_extra (int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) override
    {
        if constexpr (debug_callback_extra) {
            std::cout << "myvisual::key_callback_extra called for key=" << key << " scancode="
                      << scancode << " action=" << action << " and mods=" << mods << std::endl;
        }
        // 'm' key means toggle the 'moving' attribute
        if (key == morph::key::f && action == morph::keyaction::press) { this->moving = this->moving ? false : true; }

        if (key == morph::key::h && action == morph::keyaction::press) {
            std::cout << "myvisual extra help:\n";
            std::cout << "m: Start moving\n";
        }
    }
};

int main()
{
    myvisual v(600, 400, "Custom Visual: myvisual");
    v.addLabel ("Hello World!", {0,0,0});
    while (!v.readyToFinish) {
        v.waitevents (0.018);
        if (v.moving == true) {
            std::cout << "Keep on moving...\n";
            v.moving = false;
        }
        v.render();
    }
    return 0;
}
