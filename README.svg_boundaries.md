# SVG defined boundaries

The **morph::HexGrid** class can have its boundary set from a specially defined scalable vector graphics drawing (svg). This readme describes how to create a suitable drawing.

Create an svg file. I used Inkscape, but the code has been tested with Adobe Illustrator-generated files and these should work.

In your drawing, draw a boundary using the Bezier curve tool (Shift-F6 in Inkscape - the little Rotring pen icon). Open the properties for the object (Ctrl-Shift-O in Inscape) and set its 'id' to 'cortex' (The press the 'Set' button in Inkscape). The id 'cortex' was used because the code was originally developed to define boundary shapes for cortical brain regions.

Now draw a line to act as the scale bar with the Freehand line tool (F6 in Inkscape - the little pencil icon). Make the line's length equal to a known distance in millimetres. Open the properties for the line object and set 'id' based on the length. For example, if the line represents 1 mm, set the 'id' to '1x0_mm'. Note that 'x' has to be used instead of '.'. If the line represents 0.25 mm, then you would set 'id' to '0x25_mm'.

Note that it doesn't matter if there are other objects in your drawing, the HexGrid code will simply ignore these. For example, you can have a background bitmap image that you're tracing your boundary around in the .svg file.

Finally, save your svg. It should now be ready to be applied to a HexGrid boundary.

The program **examples/show_svg_boundary** can be used to test your svg. It takes up to 3 arguments. The first is your svg file path. The second is the overall span of your boundary in mm and the third is the hex diameter, also in mm. For example (from the root morphologica directory):

```bash
./build/examples/show_svg_boundary myboundary.svg 10 0.1
```

There are some example svg files in the **boundaries** directory. Try this:

```bash
./build/examples/show_svg_boundary boundaries/whiskerbarrels.svg 3 0.01
```

or this:

```bash
./build/examples/show_svg_boundary boundaries/whiskerbarrels.svg 3 0.03
```

If opening your new svg file with show_svg_boundary works, then you can use the file in a HexGrid. Here's an example code snippet:

```c++
// These two includes are required
#include <morph/ReadCurves.h>
#include <morph/HexGrid.h>

// You read the SVG with a morph::ReadCurves object
morph::ReadCurves r("/path/to/myboundary.svg");
// Create a HexGrid, with suitable hex diameter, hex grid initial size,
// z value set to 0 and HexDomainShape set to HexDomainShape::Boundary
float hexd = 0.1f; float x_span = 2.0f; float z = 0.0f;
morph::HexGrid hg(hexd, x_span, z, morph::HexDomainShape::Boundary);
// Apply the curves as a boundary:
hg.setBoundary (r.getCorticalPath());
```
