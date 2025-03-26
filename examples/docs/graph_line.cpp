// A line graph showing how line segments work nicely
// Also demonstrates crossing points

#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>
#include <format>

int main()
{
    // Set up a morph::Visual 'scene environment'.
    morph::Visual v(1280, 575, "Made with morph::GraphVisual");

    // Create a GraphVisual object (obtaining a unique_ptr to the object) with a spatial offset within the scene of 0,0,0
    auto gv = std::make_unique<morph::GraphVisual<double>> (morph::vec<float>{ 0.0f, 0.0f, 0.0f });

    // This mandatory line of boilerplate code sets the parent pointer in GraphVisual and binds some functions
    v.bindmodel (gv);

    // Data for the x axis. A vvec is like std::vector, but with built-in maths methods
    morph::vvec<double> x;

    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    x.linspace (0, 10, 11);

    // Hand chosen numbers
    morph::vvec<double> y = { 5, 8, 2, 9, 1, 2, 4, 5, 8, 3, 1 };

    // Choose a line graph by creating a lines stylepolicy datasetstyle.
    morph::DatasetStyle ds (morph::stylepolicy::lines);
    ds.linecolour = morph::colour::crimson;

    // For this graph, set manual y axis limits
    gv->setlimits_y (0, 10);

    // Now set the data
    gv->setdata (x, y, ds);

    // A second DatasetStyle is used to specify a colour and linewidth for a horizontal line at y=7.
    morph::DatasetStyle ds_horz (morph::stylepolicy::lines);
    ds_horz.linecolour = morph::colour::grey68;
    ds_horz.linewidth = ds.linewidth * 0.6f;

#ifdef GRAPH_LINES_SIMPLE
    gv->add_y_crossing_lines (x, y, 7, ds);
#else
    // Find, and annotate with vertical lines, the locations where the graph crosses
    // y=7. The x values of the crossing points are returned.
    morph::vvec<double> xcross = gv->add_y_crossing_lines (x, y, 7, ds, ds_horz);

    // Use results in xcross to annotate the graph
    size_t n = xcross.size();
    std::stringstream ss;
    if (n > 0) {
        // Loop through the elements of xcross, formatting a string
        for (size_t i = 0; i < n; ++i) {
            ss << std::format ("{}{}{:.2f}", ((i == 0 || i == n - 1) ? "" : ", "), (i == (n - 1) ? " and " : ""), xcross[i]);
        }
    } else {
        ss << "[no values]";
    }
    // Add a label at location {.05, .05, 0} with fontsize 0.03
    gv->addLabel (std::format("y=7 at x = {:s}", ss.str()), { 0.05f, 0.05f, 0.0f }, morph::TextFeatures(0.03f));
#endif

    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();

    // Add the GraphVisual OpenGL model to the Visual scene, transferring ownership of the unique_ptr
    v.addVisualModel (gv);

    // Render the scene on the screen until user quits with 'Ctrl-q'
    v.keepOpen();

    // Because v owns the unique_ptr to the GraphVisual, its memory will be deallocated when v goes out of scope.
    return 0;
}
