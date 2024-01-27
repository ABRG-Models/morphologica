---
layout: page
parent: Tutorials
title: morph::vvec
permalink: /tutorials/vvec/
nav_order: 3
---
# Using the variable vector class, morph::vvec

`vvec` is a dynamically re-sizable array which derives from `std::vector`.
It extends `std::vector` by providing maths methods.
It's a convenient and simple linear maths library, all in a single header.
The elements of a `vvec` take the template type for the class, which is commonly a scalar such as `int`, `float` or `double`.
However, it is also possible to create a `vvec` of vectors or coordinates.

Here's how the class is declared:

```c++
    template <typename S=float, typename Al=std::allocator<S>>
    struct vvec : public std::vector<S, Al>
    {
        //! We inherit std::vector's constructors like this:
        using std::vector<S, Al>::vector;
```

## Creating a `vvec`

Easy! Just do it as if it were an `std::vector`:
```c++
morph::vvec<int> vv = { 1, -4, 6, 8 };
```
That creates and initialises a vvec containing four int numbers. If you want to initialise every element to the same value (such as 0), you can instead (again, just like std::vector):
```c++
morph::vvec<int> vv (4, 0);
```
Note that you can also `resize()` a vvec and call `reserve()`, just as for `std::vector`.

We can place either of the initialisers above into a program, and then send the content of the vvec to stdout with the line:
```c++
std::cout << vv << std::endl;
```
A complete, short program example is:
```c++
#include <iostream>
#include <morph/vvec.h>
int main()
{
    morph::vvec<int> vv = { 1, -4, 6, 8 };
    std::cout << vv << std::endl;
    return 0;
}
```
When you compile and run, you should see this:
```
~/codeproject$ g++ -I. -o vvec vvec.cpp && ./vvec
(1,-4,6,8)
~/codeproject$
```
`vvec` has the ability to be streamed, and places brackets around the comma-separated numbers.

## Filling the `vvec` with Values

You can set the values in your vector with a number of functions. Anything you could do with an `std::vector` will work, such as:
```c++
morph::vvec<float> vf (4); // Create a vvec of size 4
vf[0] = 1.0f; // Set the first element to the value 1
vf[1] = 2.0f;
```
or:
```c++
float f = 0.0f;
for (auto& v : vf) {
    v = f;
    f += 1.0f;
}
```
There are also a number of `set_from` functions:
```c++
morph::vvec<float> vf;

// set values from an std::array
std::array<float, 5> arr1 = { 5.0f, 4.0f, 3.0f, 2.0f, 1.0f };
vf.set_from (arr1);
std::cout << vf << std::endl;

// set values from a single scalar
vf.set_from (7.5f);
std::cout << vf << std::endl;
```
which gives this output:
```
(5,4,3,2,1)
(7.5,7.5,7.5,7.5,7.5)
```
Note that when setting from a fixed size array, our vvec gets resized, but when set_from a single scalar is called, the vvec's size is maintained and every element is given the passed in value of `7.5f`.

You can set all elements of a `vvec` to zero or the max/lowest possible value of the element type with these functions:
```c++
morph::vvec<unsigned short> vus (10);
vus.zero();       // Set all elements to 0
vus.set_max();    // Set all elements to max for unsigned short (65535)
vus.set_lowest(); // Set all elements to the lowest/most negative possible value (0).
```
### A numpy-like `linspace()` function for C++

Another way to fill your vvec is to use `vvec::linspace`, which works like Python numpy's linspace:
```c++
morph::vvec<double> vd;
vd.linspace (0, 1.0, 4);
std::cout << vd << std::endl
```
Output is:
```
(0,0.33333333333333331,0.66666666666666663,1)
```
### Randomize the Content of the Vector

To place random values in the vector from a uniform distribution, call `vvec::randomize()`.
```c++
morph::vvec<float> vf;
vf.resize(3);
vf.randomize();
std::cout << vf << std::endl;
```
This sets each element of vf from a uniform random distribution with values between 0 and 1. The program might output:
```
(0.978181541,0.484534025,0.824066818)
```
You can also choose the range for the uniform random number generator with a min and max range specifier:
```c++
vf.randomize (4.0f, 6.0f);
std::cout << vf << std::endl;
```
Giving example output:
```
(5.53486824,5.04247379,4.88938808)
```
If you want to set values by selecting values from a normal distribution, then use `randomizeN()`:
```c++
float mean = 0.0f; // The mean of the normal distribution
float sd = 3.7f;   // The standard deviation of the distribution
vf.randomizeN (mean, sd);
```

## Doing Some Math

### Basic Arithmetic Operations

Let's actually do some maths with those vectors. When you have a one-dimensional array of numbers, you will often want to do something to the numbers element by element. For example, you might want to add corresponding elements in two vectors of the same length together.

Here's how you can create two vectors and add them together with vvec:
```c++
morph::vvec<double> v1 = { 2, 3, 4 };
morph::vvec<double> v2 = { 1, -4, 4.5 };
std::cout << v1 << "+" << v2 << "=" << v1 + v2 << std::endl;
```
Notice how simple that is? You just write `v1 + v2`. The operator overload returns a new `vvec` containing the element-wise sum of `v1` and `v2`. Other operators are defined:
```c++
std::cout << v1 << "*" << v2  << "=" << v1 * v2  << std::endl;
std::cout << v1 << "+" << 3.0 << "=" << v1 + 3.0 << std::endl;
std::cout << 1  << "/" << v1  << "=" << 1.0/v1   << std::endl;
std::cout << v2 << "/" << 2.0 << "=" << v2/2.0   << std::endl;
```
For each operator, you can also make use of the op= version. So these are all valid:
```c++
v1 += v2;
v1 -= 4.5;
v1 /= 6.0;
v1 *= v2;
```
On the whole, if the two operands are vvecs, then the operation is carried out element-wise; if one of the operands is a scalar, then this scalar is added/subtracted/multiplied by each element of the vvec operand.
Vector Operations

One intention of `morph::vvec` (and especially its fixed-size cousin `morph::vec`) was to do genuine vector algebra - vector addition, subtraction and scaling. These are covered by the basic arithmetic operators described above, but in addition (to addition) the cross product and scalar product operations are defined.

You can compute the scalar product (also known as the inner product or the dot product) of two vectors that have the same number of elements.
```c++
double scalar_product = v1.dot (v2);
```
The cross product is also defined, as long as the vectors have three elements. Mathematicians have defined cross products for other dimensionalities, and if you need one of those cross products, then please feel free to make a pull request on Github with your solution. The plain, 3D cross product can be very useful; here's an example with unit vectors:
```c++
morph::vvec<double> u1 = { 1, 0, 0 };
morph::vvec<double> u2 = { 0, 1, 0 };
std::cout << u1 << " cross " << u2 << " = " << u1.cross (u2) << std::endl;
```
### Functions

Another way to think of the numbers in your vvec are as a series, such as a time series. You can use linspace to define a sequence of times, and then compute a number of different functions as follows:
```c++
// Create a vvec of 100 time points from 0 to 10
morph::vvec<float> t;
t.linspace (0, 10, 100);

float s = 1.0f;
float p = 3.0f;
// Example functions that can be applied to t
morph::vvec<float> sine_of_t        = t.sin();     // sine of t
morph::vvec<float> cos_of_t         = t.cos();     // cosine of t
morph::vvec<float> exp_of_t         = t.exp();     // natural exponential (e^t)
morph::vvec<float> log_of_t         = t.log();     // natural log (base e)
morph::vvec<float> log10_of_t       = t.log();     // log (base 10)
morph::vvec<float> gaussian_of_t    = t.gauss(s);  // Gaussian fn: e^(-t*t/2*s*s)
morph::vvec<float> third_power_of_t = t.pow(p);    // powers (here, t^3)
morph::vvec<float> sqrt_of_t        = t.sqrt();    // square root of t
morph::vvec<float> sq_of_t          = t.sq();      // the square of t
```
Note that a second `vvec` is created in memory for the return data, and the values in `t` are not affected. If, instead, you want to compute the function of `t`, replacing the values in `t`, then you can call the corresponding "in place" versions of the functions:
```c++
t.sin_inplace();
t.exp_inplace();
// etc
```
There are a few other operations that can be useful, when applied to a sequence. You may want the absolute value of each element, or to remove elements that are below 0, or you may want the signum of the elements. Here are some examples:
```c++
morph::vvec<float> abs_of_t = t.abs();                     // element-wise absolute value
morph::vvec<float> signum_t = t.signum();                  // signum function
morph::vvec<float> zero_and_negative = t.prune_positive(); // prune positive elements
                                                           // (those >0)
morph::vvec<float> zero_and_positive = t.prune_negative(); // prune negative elements
                                                           // (those <0)
```
Again, these all have _inplace() versions.

### Comparisons

One important difference between `morph::vvec` and `std::vector` is what happens when you do a comparison such as:
```c++
v1 < v2
```
where both `v1` and `v2` are arrays of numbers. If `v1` and `v2` were `std::vector` objects, then the comparison would, by default, be **lexicographic**. This will return true if any of the pairs of elements, considered in order, fulfils the comparison. For example:
```c++
std::vector<int> v1 = { 10, 50, 2, 3, 700, 90 };
std::vector<int> v2 = { 12, 45, 1, 2, 699, 70 };
std::cout << "Is v1 < v2? " << (v1 < v2 ? "yes, it's less than" : "no,
                                not less than") << std::endl;
```
This outputs "yes, it's less than" because 10 < 12, which is the first comparison made. This kind of comparison is good when comparing strings of characters. However, I find it more useful to consider the comparison between arrays to be true only if every element gives a true comparison. For this reason, this example prints "Yes" because 10<12 and 44<45:
```c++
morph::vvec<int> vv3 = { 10, 44 };
morph::vvec<int> vv4 = { 12, 45 };
if (vv3 < vv4) { std::cout << "Yes\n"; }
else { std::cout << "No\n"; }
```
but this example prints "No" because 50 is not less than 45.
```c++
morph::vvec<int> vv3 = { 10, 50 };
morph::vvec<int> vv4 = { 12, 45 };
if (vv3 < vv4) { std::cout << "Yes\n"; }
else { std::cout << "No\n"; }
```
### Statistics

It's straightforward to compute a number of summary statistics from your vector of numbers:
```c++
morph::vvec<double> vv = { 0.4, 0.6, -0.8, 0.34 };

std::cout << "The mean of " << vv << " is " << vv.mean() << std::endl;
std::cout << "The sum of " << vv << " is " << vv.sum() << std::endl;
std::cout << "The standard deviation of " << vv << " is " << vv.std() << std::endl;
std::cout << "The cumulative product of " << vv << " is " << vv.product() << std::endl;
```
Output:
```
vv = (0.40000000000000002,0.59999999999999998,-0.80000000000000004,0.34000000000000002)
The mean of vv is 0.135
The sum of vv is 0.54
The standard deviation of vv is 0.633167
The cumulative product of vv is -0.06528
```
### Convolutions/Smoothing/Derivatives

Lastly, `morph::vvec` has a `convolve` function to convolve a 1D array of numbers with a 1D convolution kernel and a discrete derivative function. Both of these can be applied as if the vvec were wrapped around.

Here's an example of a convolution:
```c++
using mc = morph::mathconst<double>;
using wrapdata = morph::vvec<double>::wrapdata;

// Create x, and initialise with a sequence of values using vvec::linspace
morph::vvec<double> x;
x.linspace (-mc::pi, mc::pi-(mc::pi/5.0), 60);

// Create y as the sine of x
morph::vvec<double> y = x.sin();

// Create some random noise and add to y
morph::vvec<double> r (x.size(), 0.0);
r.randomize();
y += r;

// Manually create a convolution filter
morph::vvec<double> filter = {.2, .4, .6, .8, 1, .8, .6, .4, .2};
filter /= filter.sum();

// convolve y with the filter, copying the result into y2. Apply 1D wrapping.
morph::vvec<double> y2 = y.convolve (filter, wrapdata::wrap);
```
To see this example graphed, you can check out this example from morphologica: [vvec_convolve.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/vvec_convolve.cpp)

## Points of Interest

Because `morph::vvec` derives from `std::vector`, it's possible to pass a `morph::vvec` to any function that takes a reference to `std::vector`. This can be useful when working with other libraries.

As far as performance goes, `morph::vvec` has to be compared to other linear algebra libraries. Because I coded `vvec` for convenience rather than performance, I didn't expect it to be anywhere near as fast as other libraries such as Eigen. However, because compiler optimizations are so good, the performance of `vvec` isn't bad at all. I haven't yet had time to make a really good, comprehensive comparison between Eigen, `vvec` and other linear algebra libraries, but quick tests showed that for some array sizes, `vvec` was faster than Eigen and for other array sizes, Eigen was the fastest. For complex operations, in which Eigen uses clever templates to combine operations, I'd expect `vvec` to fall further behind.

However, `vvec`'s primary selling point is simplicity and convenience. You can code up mathematical operations with ease and the class is a pleasure to use. I hope you enjoy it.
