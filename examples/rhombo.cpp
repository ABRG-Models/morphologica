#include <morph/Visual.h>
#include <morph/RhomboVisual.h>
#include <morph/GridVisual.h>
#include <morph/vec.h>

int main()
{
    // Create a scene
    morph::Visual v(1024, 768, "A rhombohedron scene", {0.8,-0.8}, {.1,.1,.1}, 3.0f, 0.02f);
    v.showCoordArrows = true; // Please show the coord arrows by default
    v.ptype = morph::perspective_type::cylindrical; // compute cyl. projection for this scene
    v.coordArrowsInScene = true;
    v.fov = 90;
    v.lightingEffects();
    v.releaseContext();

    // *Show* cyl. proj of v on v2
    morph::Visual v2(768, 768, "Cylindrical projection viewer");
    v2.showCoordArrows = false;
    v2.lightingEffects();

    v2.releaseContext();
    v.setContext();

    // Parameters of the model
    morph::vec<float, 3> offset = { -1,  0,  0 };   // a within-scene offset
    morph::vec<float, 3> e1 = { 0.25,  0,  0 };
    morph::vec<float, 3> e2 = { 0.1,  0.25,  0 };
    morph::vec<float, 3> e3 = { 0,  0.0,  0.25 };
    morph::ColourMap<float> cmap(morph::ColourMapType::Rainbow);

    offset = { -2, 0, 0.05 };
    auto rv = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(1.0f));
    v.bindmodel (rv);
    rv->finalize();
    v.addVisualModel (rv);

    offset = { 2, 0, -1.7 };
    auto rv2 = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.5f));
    v.bindmodel (rv2);
    rv2->finalize();
    v.addVisualModel (rv2);

    offset = { 0, 2, 0.15 };
    auto rv3 = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.3333f));
    v.bindmodel (rv3);
    rv3->finalize();
    v.addVisualModel (rv3);

    offset = { 2, 2, 0.5 };
    auto rv4 = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.25f));
    v.bindmodel (rv4);
    rv4->finalize();
    v.addVisualModel (rv4);

    offset = { 0, -2.2, 0.9 };
    auto rv5 = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.2f));
    v.bindmodel (rv5);
    rv5->finalize();
    v.addVisualModel (rv5);

    offset = { 0, -1.8, 1.7 };
    auto rv6 = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.1f));
    v.bindmodel (rv6);
    rv6->finalize();
    v.addVisualModel (rv6);
    v.render();

    // context change
    v.releaseContext();
    v2.setContext();

    // This model goes in window 2
    offset = { 0, 0, 0 };
    auto gv = std::make_unique<morph::GridVisual<float>> (&v.cyl_view, offset);
    v2.bindmodel (gv);
    gv->twodimensional = true;
    gv->setVectorData (&v.cyl_data);
    gv->cm.setType (morph::ColourMapType::RGB);
    gv->zScale.setParams (0, 1);
    gv->finalize();
    auto gvp = v2.addVisualModel (gv);
    v2.render();
    v2.releaseContext();

    while (v.readyToFinish == false && v2.readyToFinish == false) {

        v.setContext();
        v.waitevents(0.01);
        v.render();
        v.releaseContext();

        v2.setContext();
        v2.waitevents(0.01);
        gvp->colourScale.reset();
        gvp->updateData (&v.cyl_data);
        v2.render();
        v2.releaseContext();
    }

    return 0;
}
