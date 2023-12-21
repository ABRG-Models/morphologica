---
title: Core maths classes
layout: page
permalink: /core/
nav_order: 2
has_children: true
---
As I developed the data-vis features of morphologica, I found I needed functionality that I've used repeatedly in many programs.
The most basic was a simple mathematical vector class, [morph::vec](/core/vec), which was essential for computing locations within the 3D graphics environment.

As the project progressed, I developed a number of these classes for handling mathematical operations with a convenient interface:

* [morph::vec](/core/vec) A mathematical vector class of a fixed size
* [morph::vvec](/core/vvec) A 'variable vector' class
* [morph::mathconst](/core/mathconst) Templated mathematical constants
* [morph::RandUniform](/core/random) Random number generation
* [morph::Scale](/core/scale) A class to scale numbers (log/linear scaling)
* [morph::ColourMap](/core/colourmap) When you visualise, you will need colour maps
* [morph::Matrix22](/core/matrix33) 2x2 matrix operations
* [morph::Matrix33](/core/matrix33) 3x3 matrix operations
* [morph::TransformMatrix](/core/transformmatrix) 4x4 matrix operations are common in graphics systems
* [morph::Quaternion](/core/quaternion) Quaternions are also commonly used for graphics
