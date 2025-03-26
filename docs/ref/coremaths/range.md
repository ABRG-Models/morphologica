---
title: morph::range
parent: Core maths classes
grand_parent: Reference
layout: page
permalink: /ref/coremaths/range
nav_order: 4
---
```c++
#include <morph/range.h>
```
Header file: [morph/range.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/range.h).

`morph::range` is a useful little class for specifying a mathematical closed interval. It simply contains two numbers indicating the minimum and maximum of the interval.

It is used as a return object for the `vec::range` and `vvec::range` methods and gives semantic meaning to the two values `min` and `max`, which are public and accessible directly by client code (if a 2 element array were used for a range, the client coder would have to remember if element 0 was min or max).

The range object can participate in the process of determining the range of values in a data container and it can test for a value being within the interval.

`morph::range` takes one template argument, specifying the type of the values.
```c++
namespace morph {
    template <typename T>
    struct range
    {
```

`morph::range` is `constexpr` capable.

**Construct**
```c++
morph::range<T> r;                  // Default range has min == max == T{0}
morph::range<float> r(0.0f, 10.0f); // Construct with a defined range
```

**Update** the range to include a value
```c++
morph::range<int> r; // range initially 0 to 0
bool changed1 = r.update (100);      // range now 0 to 100
bool changed2 = r.update (-100);     // range now -100 to 100
bool changed3 = r.update (50);       // range unchanged; still -100 to 100
```

`update` returns a `bool` which will be true if the range was changed and false if the range is not changed. In the example above, `changed1` and changed2` will both be `true`, but `changed3` will contain `false`.

**Set** the range manually in a single function call
```c++
morph::range<int> r; // range initially 0 to 0
r.set (-100, 100);   // range now -100 to 100
```

**Test a value** to see if the range **includes** this value
```c++
r.includes (45);     // would return bool true, following on from previous example
r.includes (-450);   // would return bool false
```
To query the max or min of the range, just access the `max` or `min` members:
```c++
std::cout << "range maximum is " << r.max << " and its minimum is " << r.min << std::endl;
```

You can determine if one range fits inside another with `range::contains()`:
```c_++
morph::range<int> r1 = { 1, 100 };
morph::range<int> r2 = { 10, 90 };
morph::range<int> r3 = { -1, 2 };
std::cout << "range " << r1 << (r1.contains(r2) ? " contains " : " doesn't contain ") << r2 << std::endl;
std::cout << "range " << r1 << (r1.contains(r3) ? " contains " : " doesn't contain ") << r3 << std::endl;
```

You can **stream** the range to get both at once:
```c++
std::cout << r << std::endl;
```
This would output `[-100,100]` in our example.

There's a helper function to get `range.max - range.min`:
```c++
std::cout << "The range 'spans': " << r.span() << std::endl;
```

**Determine** a range **from data**. Here, we initialize a range with min taking the *maximum* possible value for the type and max taking the *minimum* possible value. This is done with a call to `range::search_init`. We then run through the data container, calling update for each element. For example:

```c++
morph::vvec<double> data;
data.randomize();
range<double> r;
r.search_init();
for (auto d : data) { r.update (d); }
std::cout << "The range of values in data was: " << r << std::endl;
```

To save a line of code, there's a constructor for setting up a range ready-configured for search:
```c++
morph::vvec<double> data;
data.randomize();
range<double> r (morph::range_init::for_search);
for (auto d : data) { r.update (d); }
std::cout << "The range of values in data was: " << r << std::endl;
```
