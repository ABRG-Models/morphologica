/*
 * How to create your own morph::Visual to either add additional keypress actions, or to
 * override the default actions.
 */

#include <morph/Visual.h>

struct myvisual final : public morph::Visual
{
    // Boilerplate constructor (just copy this):
    myvisual (int width, int height, const std::string& title,
              const morph::vec<float, 2> caOffset = {-0.8,-0.8},
              const morph::vec<float> caLength = {.05,.05,.05},
              const float caThickness = 2.0f, const float caEm = 0.0f)
        : morph::Visual (width, height, title, caOffset, caLength, caThickness, caEm) {}
    // Some attributes that you might need in your myvisual scene:
    bool moving = false;
protected:
    // Optionally, override key_callback() with a much sparser function:
    void key_callback (GLFWwindow* _window, int key, int scancode, int action, int mods) override
    {
        // Here, I've omitted all the normal keypress actions in Visual::key_callback,
        // except for one to close the program and one for help output:
        if (key == GLFW_KEY_X && action == GLFW_PRESS) {
            std::cout << "User requested exit.\n";
            this->readyToFinish = true;
        }
        if (key == GLFW_KEY_H && action == GLFW_PRESS) {
            std::cout << "Help:\n";
            std::cout << "x: Exit program\n";
            std::cout << "h: This help\n";
        }
        // Then call the 'extra function', defined below
        this->key_callback_extra (_window, key, scancode, action, mods);
    }
    // Also optionally, add actions for extra keys:
    void key_callback_extra (GLFWwindow* window, int key, int scancode, int action, int mods) override
    {
        // 'm' key means toggle the 'moving' attribute
        if (key == GLFW_KEY_F && action == GLFW_PRESS) { this->moving = this->moving ? false : true; }

        if (key == GLFW_KEY_H && action == GLFW_PRESS) {
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
        glfwWaitEventsTimeout (0.018);
        if (v.moving == true) {
            std::cout << "Keep on moving...\n";
            v.moving = false;
        }
        v.render();
    }
    return 0;
}
