---
title: morph::histo
parent: Core maths classes
grand_parent: Reference
layout: page
permalink: /ref/coremaths/histo
nav_order: 11
---
# morph::histo
{: .no_toc}

```c++
#include <morph/histo.h>
```
Header file: [morph/histo.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/histo.h).

**Table of Contents**

- TOC
{:toc}

## Summary

`morph::histo` is a simple histogram class. You pass in a container of
data values and the number of bins you want to sort it into.

`morph::histo` takes two template arguments:

```c++
namespace morph {
    template <typename H=float, typename T=float>
    struct histo
```

`H` is the type of the data that will be sorted. This could be any
numeric type (`int`, `float`, `double`, etc). The second, `T`, is the
floating point type for bin locations, bin edges, proportions and the
bin width.

You simply construct a `histo` object, passing in a const reference to
your data, along with the number of bins you want to place it
into. Once constructed, you access histo member attributes for the
results. Here's an example:

```c++
#include <morph/histo.h>
int main()
{
    morph::vvec<int> numbers = { 1, 1, 2, 3, 4, 4, 4 };
    morph::histo<int, float> h(numbers, 3);

    std::cout << "For data: " << numbers << " arranged into three bins:\n\n";
    // Data range in terms of first histo template param type:
    morph::range<int> _datarange = h.datarange;
    std::cout << "data range is: " << _datarange << std::endl;
    // Counts use the std::size_t type:
    std::size_t _datacount = h.datacount;
    std::cout << "data count is: " << _datacount << std::endl;
    // proportions, bin edges, bins, bin width are of type float:
    float _binwidth = h.binwidth;
    std::cout << "bin width is: " << _binwidth << std::endl;
    morph::vvec<float> _bins = h.bins;
    std::cout << "bin centres are: " << _bins << std::endl;
    morph::vvec<float> _binedges = h.binedges;
    std::cout << "bin edges are: " << _binedges << std::endl;
    morph::vvec<std::size_t> _counts = h.counts;
    std::cout << "Counts are: " << _counts << std::endl;
    morph::vvec<float> _proportions = h.proportions;
    std::cout << "Proportions are: " << _proportions << std::endl;
}
```

The output of this program is:
```
For data: (1,1,2,3,4,4,4) arranged into three bins:

data range is: [1, 4]
data count is: 7
bin width is: 1
bin centres are: (1.5,2.5,3.5)
bin edges are: (1,2,3,4)
Counts are: (2,1,4)
Proportions are: (0.285714298,0.142857149,0.571428597)
```

### Accessing statistics

The histogram statistics are available as:

* `histo::bins` A `vvec<T>` of the bin centres
* `histo::binedges` A `vvec<T>` of the bin edges
* `histo::counts` A `vvec<size_t>` of bin counts
* `histo::proportions` A `vvec<T>` of the counts as proportions

You can also obtain the proportion of counts above or below a position `x` on the bin axis using `histo::proportion_below(const T& x)`. Using the same example numbers:

```c++
#include <morph/histo.h>
int main()
{
    morph::vvec<int> numbers = { 1, 1, 2, 3, 4, 4, 4 };
    morph::histo<int, float> h(numbers, 3);
    std::cout << "The proportion of " << numbers << " falling below 2.5 is: "
              << h.proportion_below (2.5f) << std::endl;
}
```
outputs:
```
The proportion of (1,1,2,3,4,4,4) falling below 2.5 is: 0.357143
```

The function `histo::proportion_above(const T& x)` returns the proportion above x (it returns 1 - proportion_below(x)).

## Histogram graphs

You can graph your histograms with [`morph::GraphVisual`](/morphologica/ref/visual/graphvisual). See any of the examples [graph_histo.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/graph_histo.cpp), [randvec.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/randvec.cpp) or [bootstrap.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/bootstrap.cpp).

Very briefly:

```c++
// includes: vvec.h, histo.h, Visual.h, GraphVisual.h
int main()
{
    morph::vvec<int> numbers = { 1, 1, 2, 3, 4, 4, 4 };
    morph::histo<int, float> h(numbers, 3);

    morph::Visual v(1024, 768, "Histogram");

    // Create a new GraphVisual with offset within the scene of 0,0,0. Note the type for
    // the GraphVisual has to match the *second* template type for the histo.
    auto gv = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>{0,0,0});
    v.bindmodel (gv);

    // Here we simply pass in our morph::histo object, h, as the data to graph
    gv->setdata (h);

    gv->xlabel = "Bin";
    gv->ylabel = "Proportion";
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();
}
```