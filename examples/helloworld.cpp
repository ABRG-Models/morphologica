#include <morph/Visual.h>
int main()
{
    morph::Visual v(600, 400, "Hello World!");
    v.addLabel ("Hello World!", {0,0,0});
    while (v.readyToFinish == false) {
        glfwWaitEventsTimeout (0.018);
        v.render();
    }
    return 0;
}
