---
layout: page
title: morph::Quaternion
parent: Core maths classes
grand_parent: Reference
permalink: /ref/coremaths/quaternion/
nav_order: 10
---
```c++
#include <morph/Quaternion.h>
```

A class for doing quaternion operations. Initially developed for use in `morph::Visual` to compute rotations of the scene.

Defined as:
```c++
namespace morph {
    template <typename Flt>
    class Quaternion
```
where `Flt` hints that the template arg is a floating point type. The Hamiltonian convention is adopted for the order of the member elements: `Flt w, x, y, z;`. These are the only member data attributes.



**Create** a Quaternion:

```c++
morph::Quaternion<float> q;             // By default w=1, and x, y, z = 0;
morph::Quaternion<float> q(1, 0, 0, 0); // Explicit setting of elements
```
A newly created Quaternion will be an identity Quaternion (1,0,0,0) unless explicity set otherwise.

**Copy** a Quaternion:
```c++
morph::Quaternion<float> q1;
morph::Quaternion<float> q2 = q1; // Assignment works as expected
```

You can use **equality** and **inequality operators** on a Quaternion: `q1 == q2` and `q1 != q2`.

**Stream** a Quaternion:
```c++
morph::Quaternion<float> q1(1, 0, 0, 0);
std::cout << q1 << std::endl;
```
Output: `Quaternion[wxyz]=(1,0,0,0)`

**Renormalize** to magnitude 1 or check if it's a unit quaternion:
```c++
q1.renormalize();
q1.checkunit();
```

**Setting the value of the Quaternion**. This function sets a rotation (specified in radians) about the given three dimensional axis:
```c++
void initFromAxisAngle (const vec<Flt>& axis, const Flt& angle);
```

The Quaternion can be reset to the identity with `reset()`.

**Manipulating rotations**. The most useful feature of Quaternions, and the reason for their popularity in computer programs is the fact that the rotations can be combined by **multiplication**.

```c++
morph::Quaternion<float> q1;
morph::Quaternion<float> q2;
using mc = morph::mathconst<float>;
q1.initFromAxisAngle (morph::vec<float>({1,0,0}), mc::pi_over_3);
q2.initFromAxisAngle (morph::vec<float>({0,1,0}), mc::pi_over_4);
morph::Quaternion<float> q3 = q1 * q2;
```

To multiply one Quaternion by another, it's important to specify the multiplication order.
```c++
q1.postmultiply (q2); // Places result of q1 * q2 into q1.
q1.premultiply (q2); // Places result of q2 * q1 into q1.
```

Quaternion **division** is also possible: `q3 = q1/q2;` as is division by a scalar.

The rotational inversion, or 'reverse rotation' is obtained with `invert`:
```c++
q.invert(); // Obtains the opposite rotation to the one specified by q
```
For the **algebraic inverse** or **reciprocal**, q<sup>-1</sup>, use `inverse`. This finds the reciprocal, q<sup>-1</sup> which has the property that q<sup>-1</sup> * q = the identity Quaternion (1,0,0,0).

```c++
q.inverse(); // q^-1
```
The conjugate is also obtainable with `q.conjugate();`.
