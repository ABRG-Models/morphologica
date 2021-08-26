## SVG defined boundaries

The morph::HexGrid class can have its boundary set from a specially defined .svg drawing.

Create an svg file. I used Inkscape, but the code has been tested with Adobe Illustrator-generated files and these should work.

In your drawing, draw a boundary using the Bezier curve tool (Shift-F6 in Inkscape - the little Rotring pen icon). Open the properties for the object (Ctrl-Shift-O in Inscape) and set its 'id' to 'cortex' (The press the 'Set' button in Inkscape).

Now draw a line to act as the scale bar with the Freehand line tool (F6 in Inkscape - the little pencil icon). Make the line's length equal to a known distance in millimetres. Open the properties for the line object and set 'id' based on the length. For example, if the line represents 1 mm, set the 'id' to '1x0_mm'. Note that 'x' has to be used instead of '.'. If the line represents 0.25 mm, then you would set 'id' to '0x25_mm'.

Note that it doesn't matter if there are other objects in your drawing, the HexGrid code will simply ignore these. For example, you can have a background bitmap image that you're tracing your boundary around in the .svg file.

Finally, save your svg. That should be it!

The program examples/show_svg_boundary can be used to test your svg. It takes up to 3 arguments. The first is your svg file path. The second is the overall span of your boundary in mm and the third is the hex diameter, also in mm. For example:

```
./build/examples/show_svg_boundary myboundary.svg 10 0.1
```