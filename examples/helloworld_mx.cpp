#include <morph/VisualMX.h>
int main()
{
    morph::VisualMX<morph::gl::version_4_1> v(600, 400, "Hello World!");
    v.addLabel ("Hello World!", {0,0,0});
    v.keepOpen();
    return 0;
}
