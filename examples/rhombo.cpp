#include <morph/Visual.h>
#include <morph/RhomboVisual.h>
#include <morph/GridVisual.h>
#include <morph/vec.h>

int main()
{
    // Create a scene
    morph::Visual v(1024, 768, "A rhombohedron");
    v.showCoordArrows = true; // Please show the coord arrows by default
    v.ptype = morph::perspective_type::cylindrical;
    v.coordArrowsInScene = true;
    v.lightingEffects();

    // Parameters of the model
    morph::vec<float, 3> offset = { -1,  0,  0 };   // a within-scene offset
    morph::vec<float, 3> e1 = { 0.25,  0,  0 };
    morph::vec<float, 3> e2 = { 0.1,  0.25,  0 };
    morph::vec<float, 3> e3 = { 0,  0.0,  0.25 };
    morph::ColourMap<float> cmap(morph::ColourMapType::Jet);
    std::array<float, 3> colour1 = cmap.convert(1.0f);
    std::array<float, 3> colour2 = cmap.convert(0.5f);
    std::array<float, 3> colour3 = cmap.convert(0.3333f);
    std::array<float, 3> colour4 = cmap.convert(0.25f);
    std::array<float, 3> colour5 = cmap.convert(0.2f);

    offset = { -2, 0, 0.05 };
    auto rv = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, colour1);
    v.bindmodel (rv);
    rv->finalize();
    v.addVisualModel (rv);

    offset = { 2, 0, 0.1 };
    auto rv2 = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, colour2);
    v.bindmodel (rv2);
    rv2->finalize();
    v.addVisualModel (rv2);

    offset = { 0, 2, 0.15 };
    auto rv3 = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, colour3);
    v.bindmodel (rv3);
    rv3->finalize();
    v.addVisualModel (rv3);

    offset = { 2, 2, 0.5 };
    auto rv4 = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, colour4);
    v.bindmodel (rv4);
    rv4->finalize();
    v.addVisualModel (rv4);

    offset = { 0, -2, 1 };
    auto rv5 = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, colour5);
    v.bindmodel (rv5);
    rv5->finalize();
    v.addVisualModel (rv5);

    offset = {0,-1.6,0};
    auto gv = std::make_unique<morph::GridVisual<float>> (&v.cyl_view, offset);
    v.bindmodel (gv);
    gv->twodimensional = true;
    gv->setScalarData (&v.cyl_data);
    gv->cm.setType (morph::ColourMapType::Jet);
    gv->zScale.setParams (0, 1);
    gv->finalize();
    auto gvp = v.addVisualModel (gv);

    while (v.readyToFinish == false) {
        v.waitevents(0.018);
        gvp->colourScale.reset();
        gvp->updateData (&v.cyl_data);
        v.render();
    }

    return 0;
}
