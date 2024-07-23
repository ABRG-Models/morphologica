---
title: morph::histo
parent: Core maths classes
grand_parent: Reference
layout: page
permalink: /ref/coremaths/histo
nav_order: 11
---
```c++
#include <morph/histo.h>
```
Header file: [morph/histo.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/histo.h).

`morph::histo` is a simple histogram class. You pass in a container of
data values and the number of bins you want to sort it into.

`morph::histo` takes two template arguments. The first is the type of
the data that will be sorted. This could be any numeric type (`int`,
`float`, `double`, etc). The second is the floating point type for bin
locations, bin edges, proportions and the bin width.

You simply construct a histo object passing in a const reference to
your data, along with the number of bins you want to place it into,
then access histo member attributes for the results. Here's an example:

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
    std::size_t csum = (_counts - morph::vvec<std::size_t>{ 2, 1, 4 }).sum();
    morph::vvec<float> _proportions = h.proportions;
    std::cout << "Proportions are: " << _proportions << std::endl;

    return 0;
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

You can graph your histograms with [`morph::GraphVisual`](/ref/visual/graphvisual). See any of the examples [graph_histo.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/graph_histo.cpp), [randvec.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/randvec.cpp) or [bootstrap.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/bootstrap.cpp).