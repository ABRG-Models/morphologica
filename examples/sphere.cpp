/*
 * Visualize a Sphere
 */
#include <morph/Visual.h>
#include <morph/VisualModel.h>
#include <morph/vec.h>
#include <morph/colour.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

// Quick visual that simply draws spheres
template <int glver = morph::gl::version_4_1>
class PrimitiveVisual : public morph::VisualModel<glver>
{
public:
    PrimitiveVisual(const morph::vec<float> _offset)
    {
        this->mv_offset = _offset;
        this->viewmatrix.translate (this->mv_offset);
    }

    void initializeVertices()
    {
        // This primitive computes a fan and rings to make a sphere
        float l = 1.1f;
        this->computeSphere (morph::vec<float>{-l,0,0}, morph::colour::royalblue, 1.0f, 12, 12);
        // These compute the sphere from a geodesic icosahedron. First with 2 iterations of the triangulation algorithm
        this->computeSphereGeo (morph::vec<float>{l,0,0}, morph::colour::maroon, 1.0f, 2);
        // This one with 3 iterations (meaning more triangles and a smoother sphere)
        this->computeSphereGeo (morph::vec<float>{0,l * std::tan(60*morph::mathconst<float>::deg2rad),0}, morph::colour::cyan3, 1.0f, 3);
    }
};

int main()
{
    int rtn = 0;

    morph::Visual v(1024, 768, "Sphere primitives");
    v.lightingEffects (true);

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };

        auto pvm = std::make_unique<PrimitiveVisual<>> (offset);
        v.bindmodel (pvm);
        pvm->finalize();
        v.addVisualModel (pvm);

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
