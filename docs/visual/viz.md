---
title: Visualization with OpenGL
layout: page
permalink: /viz/
---
morphologica uses OpenGL to draw graphics and text.
In a way it's like a game engine for data visualization because it provides a 3D world into which models are drawn and you can 'fly around' the models viewing them from different angles.
Unlike a game engine though, it uses very simple shaders and very simple graphics.
Where a game would take artist-generated 3D models and render and animate these models, morphologica draws very primitive models with triangles, which are generated at runtime.
For example, to draw a hexagonal grid, it simply creates vertices at the corners of each hexagon, and a vertex in the centre of each hex, then renders lots of triangles. Each group of vertices forms an 'OpenGL model'.

Before it can draw OpenGL models, morphologica needs an OpenGL context to have been set up. morphologica has a class called `morph::Visual`  which sets up a graphical 'scene' and deals with the OpenGL context. By default, morph::Visual uses the library GLFW3 to create a Window and an associated OpenGL context for all the drawing (if you want two windows in your program, you'll need two morph::Visuals).

Within your scene, you will then create one or more instances of a `morph::VisualModel`. This is the base class for a number of different classes that draw different kinds of objects. VisualModel provides a number of drawing primitives that can be used to draw the objects that form the model. Visualization models commonly consist of discs, rods, spheres and so on. Each primitive specifies the locations of OpenGL vertices that make up the rod, sphere or disc.

If you have a custom visualization need, you can derive your own class from  `morph::VisualModel`, and use the built-in primitives to draw your objects, or you can create your own.