---
title: morph::GraphVisual
parent: VisualModel classes
grand_parent: Reference
permalink: /ref/visualmodels/graphvisual
layout: page
nav_order: 10
---
# GraphVisual: 2D graphs
{: .no_toc}

```c++
#include <morph/GraphVisual.h>
```

**Table of contents**

- TOC
{:toc}

# Overview

`morph::GraphVisual` is a class that draws 2D graphs.

A short example of a program that draws a graph with `GraphVisual` is (omitting the `#include` lines for `<morph/Visual.h>`, `<morph/GraphVisual.h>` and `<morph/vvec.h>`):
```c++
int main() {
    morph::vvec<double> x = { -7,-6,-5,-4,-3,-2,-1,-0.5, 0, 0.5, 1, 2, 3, 4, 5, 6, 7  }; // data
    morph::Visual v(1024, 768, "Made with morph::GraphVisual"); // Visual scene/window

    // 5 lines of C++ code to create a 2D Graph
    auto gv = std::make_unique<morph::GraphVisual<double>> (morph::vec<float>({0,0,0}));
    v.bindmodel (gv);
    gv->setdata (x, x.pow(3));
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();
}
```
As usual, we create the `VisualModel` with a call to `std::make_unique<>`, and then call the boilerplate `bindmodel` function to wire up some callbacks.

In this example, there's just one line setting up the `GraphVisual`: `gv->setdata (x, x.pow(3));`.
This passes data into the GraphVisual instance.
The first argument to setdata copies the values in `x` to the abscissa (x axis) of the graph and `x.pow(3)` is copied to the ordinate (y axis).

The GraphVisual is then finalized (creating OpenGL vertices and
indices) and added to the `Visual` scene giving this result:

![Screenshot of a default GraphVisual graph of y = x^3](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/graph_basic1.png?raw=true)

The screenshot shows a graph of the function, which has the default dimensions of 1x1 units in model space.
GraphVisual has auto-scaled the data into model units so that it fits inside the axes of the graph and it has automatically determined suitable tick values to show.
It has used the default font ([VisualFont](/morphologica/ref/visualint/visualface#available-font-faces)::DVSans) and font size (0.05) for the axis labels and tick labels.
The data markers have the default colour of `morph::colour::royalblue` and lines of a default thickness (0.03) are drawn between the markers in black.

The rest of this page describes how you can choose different defaults do draw a variety of differently styled graphs.

## Setting data

The first example showed only a single dataset in the graph.
The call to `setdata` omitted to specify a *dataset index*, and because this was the first dataset, its index was set automatically to 0.
You can add multiple datasets to a GraphVisual.
Each call to setdata will increment the dataset index.
We could add a second dataset to the previous example:
```c++
    // ...
    gv->setdata (x, x.pow(3), "x cubed");                  // dataset index 0
    gv->setdata (x, 5.0 * x.pow(2), "5 times x squared");  // dataset index 1

    gv->finalize();
    // etc...
```
The result is shown in the left of these two graphs:

![Screenshot of two GraphVisual graphs, each showing two datasets](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/graph_basic2.png?raw=true)

Notice that we gave a third argument to `setdata` which defines the dataset's *datalabel*.
This is automatically displayed in a legend above the graph axes.
With the two setdata calls, the GraphVisual now displays two sets of data points, giving the second dataset a new marker shape ('uptriangle') and colour.
The scaling from the first set of data points is applied to second and subsequent datasets.
To illustrate, look at the right hand graph.
This also has two datasets, with the second dataset now showing 10x<sup>2</sup>.
Some of the elements of this dataset extend beyond the boundary of the axes and are not drawn.
If you are allowing GraphVisual to autoscale the data, remember that it is the first dataset for which the scaling is computed.
To work around this, you can manually define the limits of your axes (see [Controlling the data limits](#controlling-the-data-limits)).

### Setting data with a `DatasetStyle`

Each dataset in a GraphVisual has an associated `morph::DatasetStyle`.
This defines how the dataset should be drawn on the graph.
It allows you to choose whether data is represented by markers, lines or both.
It lets you set the line width and colour and the marker shape, size and colour.

When you make a call to `setdata(x, y)` or `setdata(x, y, "datalabel")`, a default DatasetStyle is created and then the `setdata (x, y, DatasetStyle)` overload is called.
For manual control over the style of your data, simply create a DatasetStyle before you call `setdata` as in this example:

```c++
    morph::DatasetStyle ds; // Equivalent: morph::DatasetStyle ds (morph::stylepolicy::both);
    ds.datalabel = "x cubed";
    ds.markercolour = morph::colour::springgreen3;
    ds.markersize = ds.markersize * 1.5;
    ds.markerstyle = morph::markerstyle::pentagon;
    gv->setdata (x, x.pow(3), ds);
```
Create a `DatasetStyle` instance, `ds`, then change some of its members. Here, we set the datalabel, changed the [colour](/morphologica/ref/visual/colour) for the markers and increased the marker size. Here's the resulting graph:

![Screenshot of two GraphVisual graphs, each showing two datasets](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/graph_datasetstyle.png?raw=true)

You can modify the `DatasetStyle` object and use it for later `setdata` calls. It need not stay in scope after `setdata` returns.

You can find DatasetStyle in the header [morph/DatasetStyle.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/DatasetStyle.h).

The public member attributes of `morph::DatasetStyle` that you can modify are:

* `morph::stylepolicy stylepolicy` Should we plot markers, lines or both? Options are `markers`, `lines`, `both` (coloured markers, black lines) `allcolour` (coloured markers and lines) or `bar` (bar graph). The default is `both`; if you want an alternative, pass it to the DatasetStyle constructor.
* `float linewidth` Thickness of lines in model units. Default is 0.007.
* `std::array<float, 3> linecolour` The [colour](/morphologica/ref/visual/colour) of the dataset line. RGB values in range [0,1].
* `float markersize` Diameter of the corners of the marker shape. Default is 0.03.
* `float markergap` A space between the markers and the lines. Default is 0.03.
* `morph::markerstyle markerstyle` The shape of the data markers. `square`, `triangle`, `hexagon`, etc. Options in [graphstyles.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/graphstyles.h).
* `std::array<float, 3> markercolour` The [colour](/morphologica/ref/visual/colour) of the datum markers. RGB values in range [0,1].

### Line graph examples
To make a line graph, create a DatasetStyle with `stylepolicy::lines` passed to the constructor. Set the `linewidth` and `linecolour` to your liking:
```c++
    // Left hand graph
    morph::DatasetStyle ds(morph::stylepolicy::lines); // initialize for line graphs
    ds.datalabel = "x cubed";
    ds.linewidth = 0.014f;
    gv->setdata (x, x.pow(3), ds); // line will be the default colour of black

    // Right hand graph. Note we re-use ds, simply changing some of its parameters
    ds.datalabel = "5 times x squared";
    ds.linewidth = 0.0035f;
    ds.linecolour = morph::colour::crimson;
    gv2->setdata (x, 5.0 * x.pow(2), ds);

```
The lines are drawn made up from straight line segments between the data points (any curve fitting is left for the client code to carry out).
![Screenshot of two GraphVisual graphs, with different line style options](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/graph_lines.png?raw=true)

### Marker-only graph examples
Create a `DatasetStyle` with `morph::stylepolicy::markers` and then change the defaults for `markersize`, `markerstyle` and `markercolour`.
```c++
    // Left hand graph
    morph::DatasetStyle ds(morph::stylepolicy::markers); // initialize for markers only
    ds.datalabel = "x cubed";
    ds.markersize = 0.06f;
    ds.markerstyle = morph::markerstyle::diamond;
    ds.markercolour = morph::colour::purple2;
    gv->setdata (x, x.pow(3), ds); // line will be the default colour of black

    // Right hand graph (re-using ds)
    ds.datalabel = "5 times x squared";
    ds.markersize = 0.014f;
    ds.markerstyle = morph::markerstyle::uphexagon;
    ds.markercolour = morph::colour::magenta;
    gv2->setdata (x, 5.0 * x.pow(2), ds);
```
When you choose `morph::stylepolicy::markers`, your datasets are plotted with a shaped marker at each datum. The left hand example has purple squares, the right had has magenta hexagons.

![Screenshot of two GraphVisual graphs, with different marker style options](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/graph_markers.png?raw=true)

### Marker plus line graph examples

There are six options for a marker-and-line graph:

```c++
    // Left hand graph
    morph::DatasetStyle ds(morph::stylepolicy::both); // equivalent to: morph::DatasetStyle ds;
    ds.datalabel = "x cubed";
    ds.linewidth = 0.0035f;
    ds.linecolour = morph::colour::purple2;
    ds.markersize = 0.06f;
    ds.markerstyle = morph::markerstyle::diamond;
    ds.markercolour = morph::colour::magenta;
    ds.markergap = 0.04;
    gv->setdata (x, x.pow(3), ds);

    // Right hand graph (re-using ds)
    ds.datalabel = "5 times x squared";
    ds.linewidth = 0.012f;
    ds.linecolour = morph::colour::navy;
    ds.markersize = 0.014f;
    ds.markerstyle = morph::markerstyle::uphexagon;
    ds.markercolour = morph::colour::dodgerblue;
    ds.markergap = 0.02;
    gv2->setdata (x, 5.0 * x.pow(2), ds);
```
![Screenshot of two GraphVisual graphs, with different line and marker style options](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/graph_both.png?raw=true)

### Data annotation lines

In general, `GraphVisual` is intended only to display your data and *not* to compute any additional statistics.
However, there is one statistical computation which is helpful enough to be integrated into the class.
This is the determination of locations where your graphed data cross a particular value for x or y, so that vertical and horizontal annotation lines can be drawn on the graph.

After calling `setdata`, it is possible to annotate your graph with `GraphVisual::add_y_crossing_lines` (for adding lines where the data crosses a y value) and the similarly named `GraphVisual::add_x_crossing_lines` to annotate data crossing an x value.

The example [graph_line.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/graph_line.cpp) has well annotated code that shows how to use these functions. The simplest code is:

```c++
    // A style for the dataset
    morph::DatasetStyle ds (morph::stylepolicy::lines);
    ds.linecolour = morph::colour::crimson;

    gv->setdata (x, y, ds); // Set the data in our GraphVisual, gv

    // Find, and annotate with vertical lines, the locations where the graph crosses
    // y=7. The x values of the crossing points are returned.
    morph::vvec<double> xcross = gv->add_y_crossing_lines (x, y, 7.0, ds);
```

Note that we call `add_y_crossing_lines` using the same data (`x` and `y`) and the same DatasetStyle as we used in the call to `setdata`.
`add_y_crossing_lines` will extract a colour from the DatasetStyle for the annotation lines and make their width a proportion of the original linewidth.
Only vertical lines that indicate the x crossing locations will be drawn by this overload of `add_y_crossing_lines`.

`add_y_crossing_lines` returns a `vvec` containing the crossing locations. This may be empty or contain any number of crossings.

To additionally draw a horizontal line indicating the value 7, you can add a second DatasetStyle to your call to `add_y_crossing_lines`.
The following example also shows an example of creating a graph label from the returned crossings.

```c++
    morph::DatasetStyle ds (morph::stylepolicy::lines);
    ds.linecolour = morph::colour::crimson;
    gv->setdata (x, y, ds); // Set the data in our GraphVisual, gv

    // A second DatasetStyle is used to specify a colour and linewidth for a horizontal line at y=7.
    morph::DatasetStyle ds_horz (morph::stylepolicy::lines);
    ds_horz.linecolour = morph::colour::grey68;
    ds_horz.linewidth = ds.linewidth * 0.6f;

    // Annotate, including a horizontal line
    morph::vvec<double> xcross = gv->add_y_crossing_lines (x, y, 7, ds, ds_horz);

    // Use results in xcross to build a label
    size_t n = xcross.size();
    std::stringstream ss;
    if (n > 0) {
        for (size_t i = 0; i < n; ++i) {
            ss << std::format ("{}{}{:.2f}", ((i == 0 || i == n - 1) ? "" : ", "), (i == (n - 1) ? " and " : ""), xcross[i]);
        }
    } else {
        ss << "[no values]";
    }
    gv->addLabel (std::format("y=7 at x = {:s}", ss.str()), { 0.05f, 0.05f, 0.0f }, morph::TextFeatures(0.03f));

```

## Unicode for special characters

You can incorporate unicode characters into your `DatasetStyle` datalabels and axis labels.
Use `morph::unicode::toUtf8()` to generate a UTF-8 string, as documented in [Unicode text handling](/morphologica/ref/visual/unicode).
Any of the `string` attributes that you can set can be passed a string containing UTF-8 character codes. For example:

```c++
    using uc = morph::unicode; // I usually shorten morph::unicode for readable code
    ds.datalabel = "x cubed is usually written as x" + uc::toUtf8 (uc::ss3); // ss3: superscript 3
    gv->setdata (x, x.pow(3), ds);

```
![Screenshot of a GraphVisual graphs with a unicode datalabel](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/graph_withunicode.png?raw=true)
Most of the codes are intuitively named; `unicode::alpha`, `unicode::beta`, `unicode::gamma` and so on. Consult the unicode header file [unicode.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/unicode.h) for the full list.

## Appending or updating data

If you need to change all the data points in a graph, you'll want to use the `update` function.

### Prepping a dataset

Your program may need to start with a graph that has an empty dataset and add to it with the `append` method. In this case, you must first prepare the graphs with as many datasets as you will use.

## The axes

You can choose from a few different axis styles.
The default style is `morph::axisstyle::box`, as shown in the previous examples.
This draws a box around the graph, with ticks drawn on the x and y axes only.
Other options include `axisstyle::boxfullticks`, `axisstyle::L` and `axisstyle::cross`.
Examples of these look like this:
![Screenshot of four graphs with different axis styles](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/graph_fouraxes.png?raw=true)

The code to setup the top left graph starts like this:
```c++
    auto gv = std::make_unique<morph::GraphVisual<float>>(morph::vec<float>({0,0,0}));
    v.bindmodel (gv);
    gv->axisstyle = morph::axisstyle::L; // That's all you have to do
    // ...rest of the setup
```
If you want to change the axis ticks then it's this:
```c++
    //...
    gv->axisstyle = morph::axisstyle::L;
    gv->tickstyle = morph::tickstyle::ticksin; // tickstyle::ticksout is the default
    // ...rest of the setup
```
You can alter the colour of the axes by changing axiscolour...

```c++
    //...
    gv->axisstyle = morph::axisstyle::L;
    gv->tickstyle = morph::tickstyle::ticksin;
    gv->axiscolour = morph::colour::cobalt;
    // ...rest of the setup
```
... and the axes line thickness with `gv->axislinewidth`:
```c++
    //...
    gv->axisstyle = morph::axisstyle::L;
    gv->tickstyle = morph::tickstyle::ticksin;
    gv->axiscolour = morph::colour::cobalt;
    gv->axislinewidth = 0.013f;
    // ...rest of the setup
```
Here are the four graphs again with thicker, coloured axes:
![Screenshot of four graphs with coloured axes](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/graph_fouraxes_clr.png?raw=true)
Note that the axis colour is applied to the axis box/cross/L, the axis ticks, axis tick labels and axis labels.

### Controlling the size of the axes

### Controlling the data limits

### The fonts

### The axis ticks

### Axis labels

## Special graph types

### Bar graph

### Histogram

### Quiver plots