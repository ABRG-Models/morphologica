---
layout: page
title: morph::Quaternion
parent: Core maths classes
grand_parent: Reference
permalink: /ref/coremaths/quaternion/
nav_order: 7
---
## A Quaternion implementation
```c++
#include <morph/Quaternion.h>
```
Header file: [morph/Quaternion.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/Quaternion.h). Test and example code:  [tests/testQuaternion](https://github.com/ABRG-Models/morphologica/blob/main/tests/testQuaternion.cpp)

A class for doing quaternion operations. Initially developed for use in `morph::Visual` to compute rotations of the scene.

Defined as:
```c++
namespace morph {
    template <typename Flt>
    class Quaternion
```
where `Flt` hints that the template arg is a floating point type. The Hamiltonian convention is adopted for the order of the member elements: `Flt w, x, y, z;`. These are the only member data attributes.



## Create a Quaternion:

```c++
morph::Quaternion<float> q;             // By default w=1, and x, y, z = 0;
morph::Quaternion<float> q(1, 0, 0, 0); // Explicit setting of elements
```
A newly created Quaternion will be an identity Quaternion (1,0,0,0) unless explicity set otherwise.

You can also create a Quaternion with a specified rotation about a given vector axis:
```c++
morph::vec<float, 3> z_axis = { 0, 0, 1 };              // set a rotation about the z axis...
float angle = morph::mathconst<float>::pi_over_2; // ...of pi/2 radians

morph::Quaternion<float> q (z_axis, angle);  // The (axis, angle) constructor
```


## Copy a Quaternion:
```c++
morph::Quaternion<float> q1;
morph::Quaternion<float> q2 = q1; // Assignment works as expected
```

You can use **equality** and **inequality operators** on a Quaternion: `q1 == q2` and `q1 != q2`.

## Stream a Quaternion:
```c++
morph::Quaternion<float> q1(1, 0, 0, 0);
std::cout << q1 << std::endl;
```
Output: `Quaternion[wxyz]=(1,0,0,0)`

## Operations on a Quaternion

Renormalize to magnitude 1 or check if it's a unit quaternion:

```c++
q1.renormalize();
q1.checkunit();
```
The Quaternion can be reset to the identity with `reset()`.

## Quaternion multiplication

The most useful feature of Quaternions, and the reason for their popularity in computer programs is the fact that the rotations can be combined by **multiplication** and their rotations applied to a vector by multiplication.

### Applying a Quaternion rotation to a `morph::vec`

You have a vector (3D or '3+1'D) and you want to apply the rotation specified by a Quaternion? You use the multiplication `operator*`:

```c++
using mc = morph::mathconst<float>;
// Create Quaternion for a rotation of pi radians
morph::Quaternion<float> q(morph::vec<float>{1, 0, 0}, mc::pi);
morph::vec<float> v = { 1, 2, 3 };
morph::vec<float> rotated = q * v; // rotates v by pi about the x axis
```

### Combining rotations by Quaternion multiplication If we have 2
Quaternions `q1` and `q2` that specify rotations, we can combine them
into a third Quaternion by multiplication. In the following code, `q3`
becomes the rotation that would result from first applying rotation
q1, then applying rotation q2.

```c++
using mc = morph::mathconst<float>;
morph::Quaternion<float> q1(morph::vec<float>({1,0,0}), mc::pi_over_3);
morph::Quaternion<float> q2(morph::vec<float>({0,1,0}), mc::pi_over_4);
morph::Quaternion<float> q3 = q2 * q1;
```

To multiply one Quaternion by another, it's important to specify the
multiplication order. The `postmultiply` and `premultiply` functions
allow you to control this.

```c++
q1.postmultiply (q2); // Places result of q1 * q2 into q1.
q1.premultiply (q2);  // Places result of q2 * q1 into q1.
```

The Quaternion `q3 = q2 * q1` is equivalent to applying an initial rotation of q1, followed by a rotation of q2. For this reason, in the following, `rotated_vec1` and `rotated_vec2` will hold the same result:

```c++
morph::Quaternion<float> q3 = q2 * q1;
morph::vec<float> unrotated = { 1, 0, 2 };
morph::vec<float> rotated_vec1 = q3 * unrotated;
morph::vec<float> rotated_vec2 = q2 * (q1 * unrotated); // More computation required
```

Quaternion **division** is also possible: `q3 = q1/q2;` as is division by a scalar. Note that Quaternion addition and subtraction are not implemented in this class at present.

### Inversions and conjugates

The rotational inversion, or 'reverse rotation' is obtained with `invert`:
```c++
q.invert(); // Obtains the opposite rotation to the one specified by q
```
For the **algebraic inverse** or **reciprocal**, q<sup>-1</sup>, use `inverse`. This finds the reciprocal, q<sup>-1</sup> which has the property that q<sup>-1</sup> * q = the identity Quaternion (1,0,0,0).

```c++
q.inverse(); // q^-1
```
The conjugate is also obtainable with `q.conjugate();` and the magnitude of the Quaternion with `q.magnitude();`.

`q.norm()` is an alias for the magnitude and `q.norm_squared()` returns the square of the norm.

## Setting the rotation of the Quaternion

The function `set_rotation` sets the values of the Quaternion to specify a rotation
(specified in degrees, not radians) about the given three dimensional
axis, starting from no rotation.

```c++
void set_rotation (const morph::vec<Flt>& axis, const Flt& angle);
```

### Rotate the Quaternion further with these methods:

```c++
void rotate (const morph::vec<Flt, 3>& axis, const Flt angle);
void rotate (const std::array<Flt, 3>& axis, const Flt angle);
void rotate (const Flt axis_x, const Flt axis_y, const Flt axis_z, const Flt angle)
```
Each method rotates the Quaternion by an angle in radians about a 3D axis specified by the axis array (or by the individual components of the axis).

### Rotation Matrix

You can obtain the equivalent 4x4 **rotation matrix** in column-major format (OpenGL friendly) from the Quaternion with
```c++
std::array<Flt, 16> rotationMatrix() const;           // Returns the rotation matrix
void rotationMatrix (std::array<Flt, 16>& mat) const  // sets the values in the passed-in matrix
```
If you know that your Quaternion can be assumed to be a unit Quaternion, you can use
```c++
std::array<Flt, 16> unitRotationMatrix() const;
void unitRotationMatrix (std::array<Flt, 16>& mat) const;
```
These involve slightly less computation.
