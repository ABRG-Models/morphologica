#include <morph/Visual.h>
int main()
{
    morph::Visual<morph::gl::version_4_1> v(600, 400, "Hello World!");
    v.addLabel ("Hello World!", {0,0,0});
    v.keepOpen();
    return 0;
}
