#include <morph/version.h>
#include <morph/Visual.h>
#include <string>

int main()
{
    morph::Visual<morph::gl::version_3_1_es> v(600, 400, "Hello Version!");
    v.addLabel ("Hello World!", {0,-0.1,0}, morph::TextFeatures(0.1f, 48));
    v.addLabel (std::string("morphologica version ") + morph::version_string(), {0,-0.2,0}, morph::TextFeatures(0.04f, 32));
    v.keepOpen();
    return 0;
}
