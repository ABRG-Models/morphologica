/*
 * An interactive version of the colourbar program (the code is more involved)
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/HexGridVisual.h>
#include <morph/HexGrid.h>
#include <morph/ColourBarVisual.h>

// This is the standard way to incorporate key-operations into a morphologica program. See also myvisual.cpp.
struct myvisual final : public morph::Visual<>
{
    myvisual (int width, int height, const std::string& title) : morph::Visual<> (width, height, title)
    {
        // Set up HexGrid
        this->hg = std::make_unique<morph::HexGrid>(0.01f, 3.0f, 0.0f);
        this->hg->setCircularBoundary (0.6f);
        std::cout << "Number of pixels in grid:" << this->hg->num() << std::endl;
        // Initialize the function
        this->wobbly_function();
        this->rebuild_visualmodels();
    }

protected:
    // A multiplier that we'll apply to the data shown in the hexgrid and hence in the colourbar
    float multiplier = 0.0625f;

    // A function offset
    float function_zero = 0.0f;

    // Data container for a function
    morph::vvec<float> data;

    // A HexGrid to show in the scene.
    std::unique_ptr<morph::HexGrid> hg;

    // A colourmap for the wobbly function
    static constexpr morph::ColourMapType colour_map_type = morph::ColourMapType::Inferno;

    bool old_labels = false;

    // VisualModel pointers are used as identifiers to allow removal and then replacement from the Visual
    morph::HexGridVisual<float>* hgvp = nullptr;        // "HexGridVisual Pointer"
    morph::ColourBarVisual<float>* cbvp_vert = nullptr; // "ColourBarVisual Pointer (Vertical)"
    morph::ColourBarVisual<float>* cbvp_horz = nullptr; // "ColourBarVisual Pointer (Horizontal)"

    // Make the usual wobbly surface for display
    void wobbly_function()
    {
        this->data.resize (this->hg->num(), 0.0f);
        for (unsigned int ri = 0u; ri < this->hg->num(); ++ri) {
            // Multipliers this small: 0.0000000001f up to 0.0000001f give tick spacing near float's epsilon.
            this->data[ri] = this->function_zero + this->multiplier * std::sin (20.0f * this->hg->d_x[ri]) * std::sin (10.0f * this->hg->d_y[ri]) ; // Range 0->1
        }
    }

    // When the wobbly function changes, we remove our three visualmodels and then
    // completely rebuild them. The computational cost of this is not an issue in this
    // demo program.
    void rebuild_visualmodels()
    {
        if (this->hgvp != nullptr) { this->removeVisualModel (hgvp); }
        if (this->cbvp_vert != nullptr) { this->removeVisualModel (cbvp_vert); }
        if (this->cbvp_horz != nullptr) { this->removeVisualModel (cbvp_horz); }

        // Add a HexGridVisual to display the HexGrid within the morph::Visual scene
        morph::vec<float, 3> offset = { 0.0f, -0.05f, 0.0f };
        auto hgv = std::make_unique<morph::HexGridVisual<float>>(this->hg.get(), offset);
        this->bindmodel (hgv);
        hgv->cm.setType (this->colour_map_type); // This is how we set the colour map type in HexGridVisual
        hgv->setScalarData (&this->data);
        hgv->hexVisMode = morph::HexVisMode::HexInterp;
        hgv->finalize();
        this->hgvp = this->addVisualModel (hgv);

        // Add the colour bar
        offset = {0.8f, -0.3f, 0.0f};
        auto cbv =  std::make_unique<morph::ColourBarVisual<float>>(offset);
        this->bindmodel (cbv);
        cbv->orientation = morph::colourbar_orientation::vertical;
        cbv->tickside = morph::colourbar_tickside::right_or_below;
        cbv->number_of_ticks_range = morph::range<float>{4, 6};
        // Copy colourmap and scale to colourbar visual
        cbv->cm = this->hgvp->cm;
        cbv->old_labels = this->old_labels;
        cbv->scale = this->hgvp->colourScale;
        // Now build it
        cbv->finalize();
        this->cbvp_vert = this->addVisualModel (cbv);

        // Add a horizontal colourbar, too
        offset = {-0.3f, -1.0f, 0.0f};
        cbv =  std::make_unique<morph::ColourBarVisual<float>>(offset);
        this->bindmodel (cbv);
        cbv->orientation = morph::colourbar_orientation::horizontal;
        cbv->tickside = morph::colourbar_tickside::left_or_above;
        cbv->cm = this->hgvp->cm;
        cbv->number_of_ticks_range = morph::range<float>{2, 3};
        cbv->scale = this->hgvp->colourScale;
        cbv->old_labels = this->old_labels;
        std::string lbl = "ColourMapType: " + morph::ColourMap<float>::colourMapTypeToStr (this->colour_map_type);
        cbv->addLabel (lbl, morph::vec<float>{0.0f, -0.08f, 0.0f}, morph::TextFeatures(0.05f));
        cbv->finalize();
        this->cbvp_horz = this->addVisualModel (cbv);
    }

    // Override the extra keys function from morph::Visual
    void key_callback_extra (int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) override
    {
        if (key == morph::key::up && (action == morph::keyaction::press || action == morph::keyaction::repeat)) {
            multiplier *= 2.0f;
            std::cout << "multiplier is now " << multiplier << std::endl;
            this->wobbly_function();
            this->rebuild_visualmodels();
        }
        if (key == morph::key::down && (action == morph::keyaction::press || action == morph::keyaction::repeat)) {
            multiplier /= 2.0f;
            std::cout << "multiplier is now " << multiplier << std::endl;
            this->wobbly_function();
            this->rebuild_visualmodels();
        }
        if (key == morph::key::right && (action == morph::keyaction::press || action == morph::keyaction::repeat)) {
            this->function_zero += 0.0103f;
            std::cout << "function's zero is now " << function_zero << std::endl;
            this->wobbly_function();
            this->rebuild_visualmodels();
        }
        if (key == morph::key::left && (action == morph::keyaction::press || action == morph::keyaction::repeat)) {
            this->function_zero -= 0.0103f;
            std::cout << "function's zero is now " << function_zero << std::endl;
            this->wobbly_function();
            this->rebuild_visualmodels();
        }
        if (key == morph::key::o && action == morph::keyaction::press) {
            this->old_labels = true;
            this->wobbly_function();
            this->rebuild_visualmodels();
        }
        if (key == morph::key::n && action == morph::keyaction::press) {
            this->old_labels = false;
            this->wobbly_function();
            this->rebuild_visualmodels();
        }
        if (key == morph::key::h && action == morph::keyaction::press) {
            std::cout << "Up: Double multiplier\n";
            std::cout << "Down: Halve multiplier\n";
            std::cout << "Left: shift zero down\n";
            std::cout << "Right: shift zero up\n";
            std::cout << "o: old labels\n";
            std::cout << "n: new labels\n";
        }
    }
};

int main()
{
    myvisual v(1200, 1000, "Colour bars");
    // Position the scene within the window
    v.setSceneTrans (morph::vec<float,3>({-0.140266f, 0.237435f, -3.5f}));
    v.keepOpen();
    return 0;
}
