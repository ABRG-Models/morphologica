/*
 * Graphing styles - a number of enumerated classes.
 */

#pragma once

namespace morph {

    //! What shape for the graph markers?
    enum class markerstyle
    {
        none,
        triangle,
        uptriangle,
        downtriangle,
        square,
        diamond,
        pentagon, // A polygon has a flat top edge, the 'uppolygon' has a vertex pointing up
        uppentagon,
        hexagon,
        uphexagon,
        heptagon,
        upheptagon,
        octagon,
        upoctagon,
        circle,
        bar, // Special. For a bar graph.
        quiver, // Special. For a quiver plot. Drawn ON coord.
        quiver_fromcoord,
        quiver_tocoord,
        numstyles
    };

    //! How does a quiver go? Does it start at the coordinate? If so, it goes 'from'
    //! the coordinate; FromCoord. Does it instead sit on top of the coord (OnCoord)?
    //! Alternatively, it could go 'to' the coordinate; ToCoord. Used in QuiverVisual.
    //! This information is absorbed into markerstyle for GraphVisual.
    enum class QuiverGoes
    {
        FromCoord,
        ToCoord,
        OnCoord
    };

    //! line styles are going to be tricky to implement
    enum class linestyle
    {
        solid,
        dotted,
        dashes,
        shortdash,
        longdash,
        dotdash,
        custom,
        numstyles
    };

    enum class tickstyle
    {
        ticksin,
        ticksout,
        numstyles
    };

    //! Different axis styles
    enum class axisstyle
    {
        L,          // just left and bottom axis bars (3D: exactly 3 x/y/z axis bars)
        box,        // left, right, top and bottom bars, ticks only on left and bottom bars
        boxfullticks, // left, right, top and bottom bars, with ticks all round
        panels,     // For 3D: like a 2D 'box' making a floor, and 2 side 'walls'
        cross,      // a cross of bars at the zero axes
        boxcross,   // A box AND the zero axes
        twinax,     // A box which has two y axes, the first on the left and the second on the right
        numstyles
    };

    //! When generating a graph, should we generate marker-only graphs, line-only graphs
    //! or marker+line graphs? Each DatasetStyle contains a stylepolicy.
    enum class stylepolicy
    {
        markers,        // coloured markers, with differing shapes
        lines,          // coloured lines
        both,           // coloured markers, black lines
        allcolour,      // coloured markers and lines
        bar,            // bar graph. marker colour is the bar colour, linecolour is the outline colour (if used)
        numstyles
    };

    //! When autoscaling data, do we purely autoscale from the data, using the data's
    //! min and max values, or do we autoscale from a fixed value (such as 0) to the
    //! data's max, or do we scale the graph to two fixed values, set by the user?
    enum class scalingpolicy
    {
        autoscale,      // full autoscaling to data min and max
        manual_min,     // Use the GraphVisual's abs/datamin_y attribute when scaling, and data's max
        manual_max,     // Unlikely to be used, but use data's min and abs/datamax_y
        manual,         // Use abs/datamin_y and abs/datamax_y for scaling - i.e. full manual scaling
        numstyles
    };

    //! Which side of the (twin) axes should the dataset relate to?
    enum class axisside
    {
        left,
        right,
        numstyles
    };

} // namespace morph
