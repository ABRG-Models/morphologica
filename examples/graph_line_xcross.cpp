// A line graph showing how line segments work nicely
// Also demonstrates crossing points

#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>
#include <format>

int main()
{
    // Set up a morph::Visual 'scene environment'.
    morph::Visual v(1024, 768, "Made with morph::GraphVisual");

    // Create a GraphVisual object (obtaining a unique_ptr to the object) with a spatial offset within the scene of 0,0,0
    auto gv = std::make_unique<morph::GraphVisual<double>> (morph::vec<float>{ 0.0f, 0.0f, 0.0f });

    // This mandatory line of boilerplate code sets the parent pointer in GraphVisual and binds some functions
    v.bindmodel (gv);

    // Data for the x axis. A vvec is like std::vector, but with built-in maths methods
    morph::vvec<double> y;

    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    y.linspace (0, 10, 11);

    // Hand chosen numbers
    morph::vvec<double> x = { 5, 8, 2, 9, 1, 2, 4, 5, 8, 3, 1 };

    // Choose a line graph by creating a lines stylepolicy datasetstyle.
    morph::DatasetStyle ds (morph::stylepolicy::lines);
    ds.linecolour = morph::colour::crimson;

    // For this graph, set manual y axis limits
    gv->setlimits_y (0, 10);

    // Now set the data
    gv->setdata (x, y, ds);

    // A second DatasetStyle is used to specify a colour and linewidth for a vertical line at x=3.3
    morph::DatasetStyle ds_vert (morph::stylepolicy::lines);
    ds_vert.linecolour = morph::colour::grey68;
    ds_vert.linewidth = ds.linewidth * 0.6f;

    // Find, and annotate with vertical lines, the locations where the graph crosses
    // x=3.3. The y values of the crossing points are returned.
    morph::vvec<double> ycross = gv->add_x_crossing_lines (x, y, 3.3, ds, ds_vert);

    size_t n = ycross.size();
    std::stringstream ss;
    if (n > 0) {
        // Loop through the elements of ycross, formatting a string
        for (size_t i = 0; i < n; ++i) {
            ss << std::format ("{}{}{:.2f}", ((i == 0 || i == n - 1) ? "" : ", "), (i == (n - 1) ? " and " : ""), ycross[i]);
        }
    } else {
        ss << "[no values]";
    }
    // Add a label at location {.05, .05, 0} with fontsize 0.03
    gv->addLabel (std::format("At x=3.3, y = {:s}", ss.str()), { 0.05f, 0.05f, 0.0f }, morph::TextFeatures(0.03f));

    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();

    // Add the GraphVisual OpenGL model to the Visual scene, transferring ownership of the unique_ptr
    v.addVisualModel (gv);

    // Render the scene on the screen until user quits with 'Ctrl-q'
    v.keepOpen();

    // Because v owns the unique_ptr to the GraphVisual, its memory will be deallocated when v goes out of scope.
    return 0;
}
