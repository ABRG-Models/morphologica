---
title: morph::scale
parent: Core maths classes
grand_parent: Reference
layout: page
permalink: /ref/coremaths/scale
nav_order: 5
---
# morph::scale
{: .no_toc}

```c++
#include <morph/scale.h>
```
Header file: [morph/scale.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/scale.h). Test code:  [tests/testScale](https://github.com/ABRG-Models/morphologica/blob/main/tests/testScale.cpp)  [tests/testScaleVector](https://github.com/ABRG-Models/morphologica/blob/main/tests/testScaleVector.cpp)

**Table of Contents**

- TOC
{:toc}

## Summary

In any computer visualization system, you'll soon need a way to scale numbers. If you're graphing a data set, you'll need to scale the values, whose [range](/morphologica/ref/coremaths/range) may be from 0 to 10<sup>6</sup>, so that they fit into a graph in your 3D environment whose height is 1. Likewise, all the colour maps in morphologica take input in the range [0,1] and if you want to make a quiver plot, you're likely to need to scale the length of the vectors you're going to render.

Scaling values is not inherently complex, but to create a class which can linearly or logarithmically scale both scalar and vector values is not trivial either! `morph::scale` handles all these cases and is an important part of morphologica which you may find useful in your own code as well.

## Design

The templated class `morph::scale` derives from `morph::scale_impl`:
```c++
namespace morph {
    template <typename T, typename S=T>
    struct scale : public scale_impl<number_type<T>::value, T, S> {};
```

Template param `T` is the type of the numbers or vectors that will be
scaled. Param `S` is the type of the output numbers (or for vectors,
their elements).

A suitable implementation of `scale_impl` is chosen at compile time, depending on whether the type `T` is a scalar such as `float`, `double` or `int` or an array or vector type such as `morph::vec<double, 3>`. `number_type` is a trait testing class that determines which implementation to compile. The `scale_impl` base class defines the API for `morph::scale`.

In future, other scaling implementations could be introduced for other mathematical objects.

`morph::scale` and friends make use of an enumerated scale function class:
```c++
namespace morph {
    enum class scaling_function { Linear, Logarithmic };
```
At present only these two scaling functions are supported, but this could be extended if needed.

## Usage

### Transforming data

If you have a `morph::scale` object that is parameterized, then you can scale individual values or collections of values.
```c++
morph::scale<float> s;
std::vector<float> input (10, 0.0f);
std::vector<float> output (10, 0.0f);
// snip code to find scaling parameters and initialise input
s.transform (input, output);                // Applies scaling to all values in the input container
float transformed = s.transform_one (2.0f); // Transforms a single value using the scaling
```
#### Inverse transform

You can also apply the inverse scaling to individual values:
```c++
float inverse_transformed = s.inverse_one (10.0f);
```
and you can inverse-transform whole containers of data as in this example:
```c++
morph::scale<double> s;
s.do_autoscale = true;
morph::vvec<double> input = { 10, 20, 30, 40 };
morph::vvec<double> output (input.size());
morph::vvec<double> inputcpy (input.size());
s.transform (input, output);  // one way...
s.inverse (output, inputcpy); // ...and back
std::cout << input << " transforms to " << output << " and inverses to " << inputcpy << std::endl;
```
output:
```
(10,20,30,40) transforms to (0,0.33333333333333331,0.66666666666666674,1) and inverses to (10,20,30,40)
```

### Automatically finding the scaling parameters

Perhaps the most common use of `morph::scale` is to linearly scale a collection of data values so that they have the range [0,1]. This means that the two parameters of the linear model y = *m*x + *c* must be found and used to transform the dataset. I've called this 'autoscaling from the data'. Here's the simplest example:
```c++
morph::scale<float> s; // A linear scaling from float values to float values
s.do_autoscale = true; // Tell the scale object we will need to autoscale
morph::vvec<float> vf = {1,2,3,4,5,8,9,18}; // Input data. std::vector<> would be fine too
morph::vvec<float> result(vf);              // Output/transformed data
s.transform (vf, result);        // transform() will first determine parameters then apply
```

The `scale` class is linear by default, and its default `output_range` is [0,1]. It does not autoscale by default, so this has been explicitly set. Its output type defaults to being the same as the input type, so the declaration above is equivalent to `morph::scale<float, float>`.

If your input data are integer values, but your scaled values would be more appropriate in a floating point type, then adjust your choice of `morph::scale` template parameters:
```c++
morph::scale<int, float> s;               // int inputs scaled to float outputs
s.do_autoscale = true;
morph::vvec<int> vi = {1,2,3,4,5,8,9,18}; // Input data in int type
// The rest is the same:
morph::vvec<float> result(vi.size());     // Output/transformed data in float type
s.transform (vi, result);
```

You can autoscale from a data set, finding the scaling parameters, without also transforming the data with `compute_scaling_from_data`:

```c++
morph::scale<int, float> s;
s.do_autoscale = true;
morph::vvec<int> vi = {1,2,3,4,5,8,9,18};
s.compute_scaling_from_data (vi); // s.transform (anydata) can now be called
```

Note that subsequent calls to `transform` will use the scaling parameters obtained from the first call! If you want to autoscale each new data set, then you have to reset `do_autoscale` back to true with `reset`:

```c++
morph::scale<int, float> s;
s.do_autoscale = true;
morph::vvec<int> vi = {1,2,3,4,5,8,9,18};           // A first data set
morph::vvec<int> vi2 = {10,20,30,40,50,80,90,180};  // A second data set
morph::vvec<float> result(vi);

s.transform (vi, result);   // Autoscales first data set to range [0,1]
s.reset();                  // Reset so that autoscaling will be re-computed
s.transform (vi2, result);  // Autoscales second data set
```

### Changing the output range

You may not always wish to scale your input data to the range [0,1]. To modify this, set the `output_range` for the scale object, which is an object of type `morph::range<S>` if `S` is a scalar type and of type `morph::range<S_el>` if `S` is a vector type. `S_el` is defined in scale.h to be the element type of `S`. To configure the `range` object, set its `min` and `max` attributes:

```c++
morph::scale<int, float> s;
s.output_range.min = 1.0f;
s.output_range.max = 2.0f;
s.do_autoscale = true;      // transform() will scale output to range [1,2] by calling compute_scaling_from_data()
```
or set it with brace initializers:
```c++
morph::scale<int, float> s;
s.output_range = { 1.0f, 2.0f };
```

### Manually setting the scaling parameters

The scaling parameters are stored in a member `morph::vvec` object called `params`. Both linear and logarithmic scaling functions require two parameters. You can set the params with
```c++
void setParams (S p0, S p1);
```
For a linear scaling, param 0 is gradient (or 'm') and param 1 is offset (or 'c'). Note that the type of the params is the *output* type, `S`.

There are corresponding getters for the params (these are from the scalar scale implementation):
```c++
S getParams (size_t idx) { return this->params[idx]; }
morph::vvec<S> getParams() { return this->params; }
```

It is the scaling parameters that determine if the `morph::scale` object is `ready()` (returns true if params size is > 1) and `reset()` simply calls `clear()` on `scale::params`.

You can only set the params if you already know the gradient and offset for your scaling. If you only know the expected *range* of input values for your scaling, you can use these to compute the scaling for the output range:

```c++
// Set up linear scaling so that numbers in range [-10, 10] get scaled to [0,5]
morph::scale<int, float> s;
s.output_range = { 0.0f, 5.0f };
s.compute_scaling (-10, 10);
```

You can also pass the input range to `scale<>::compute_scaling` and set the `output_range` with explicit `morph::range<>` objects:

```c++
morph::scale<int, float> s;
s.output_range = morph::range<float>{0, 5};
s.compute_scaling (morph::range<int>{-10, 10});
```

You can trigger the computation of the scaling function if you have a container of data by using `compute_scaling_from_data`, which is the function that is automatically called by `transform` when `do_autoscale` is `true`.

```c++
morph::scale<int, float> s;
std::vector<int> input_data = { 1, 2, 3, 6, 100 };
s.compute_scaling_from_data (input_data);
```

#### Identity scaling

If you need your `scale` object to perform the identity scaling, set its parameters with
```c++
morph::scale<double> sid;
sid.identity_scaling();
std::cout << 2.0 << " = " << sid.transform_one (2.0) << std::endl; // "2.0 = 2.0"
```

#### Null scaling

If you need your `scale` object to always output 0, you can set it to 'null scaling'.
```c++
morph::scale<double> sid;
sid.null_scaling();
std::cout << 2.0 << " null-scales to " << sid.transform_one (2.0) << std::endl; // "2.0 null-scales to 0.0"
```

`scale::null_scaling()` is equivalent to calling `scale::setParams (0, 0)`.

### Logarithmic scaling

Set logarithmic scaling by calling `setlog`.

```c++
morph::scale<double, float> ls;
ls.do_autoscale = true;
ls.setlog();

morph::vvec<double> input = {0.01, 0.1, 1.0, 10.0};
morph::vvec<float> output (input.size(), 0.0f);

if (input.has_zero()) {
   std::cout << "Warning: you can't log scale input containing zeros...\n";
   // To workaround, replace zeros in the input with NaNs (these will still be NaN after scaling)
   input.search_replace (0.0, std::numeric_limits<double>::quiet_NaN());
}

try {
    ls.transform (input, output);
} catch (const std::exception& e) {
    // Common reason for exception is that input contains zeros
    std::cout << "Caught error: " << e.what() << std::endl;
}
```

### Vector scaling

The vector implementation of `morph::scale` will scale the *lengths* of vectors, preserving their direction. This is exactly what you want if you are rendering quiver plots.

The API is essentially the same, except for the fact that the output_range (which is a range of vector lengths) has the type of the vector elements.

```c++
    morph::scale< morph::vec<float,4> > vec_scale;
    vec_scale.do_autoscale = true; // Vector lengths will be scaled to the range [0,1]

    // If you set the output_range, use the type of the vec *elements*
    // vec_scale.output_range.min = 1.0f; // float, not vec<float,4>
    // vec_scale.output_range.max = 2.0f;
    // or
    // vec_scale.output_range = { 1.0f, 2.0f };

    std::deque<morph::vec<float,4>> vec_input; // An input container
    vec_input.push_back ({1,1,2,1});
    vec_input.push_back ({2,2,2,3});
    vec_input.push_back ({3,3,4,1});
    vec_input.push_back ({4,4,4,4});
    std::deque<morph::vec<float,4>> scaled_vectors (vec_input.size()); // Output container

    // Perform autoscale and then tranformation:
    vec_scale.transform (vec_input, scaled_vectors);
```