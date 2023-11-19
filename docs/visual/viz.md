---
title: Visualization with OpenGL
layout: page
permalink: /viz/
---
morphologica uses OpenGL to draw graphics and text. In a way it's like a game engine for data visualization. Unlike a game engine, it uses very simple shaders and very simple graphics. Where a game would take artist-generated 3D models and render and animate these models, morphologica draws very primitive models out of triangles, which are generated at runtime. For example, to draw a hexagonal grid, it simply creates vertices at the corners of each hexagon, and a vertex in the centre of each hex, then renders lots of triangles. Each group of vertices forms an 'OpenGL model'.

Before it can draw OpenGL models, morphologica needs an OpenGL context to have been set up. morphologica has a class called `morph::Visual`  which sets up a graphical 'scene' and deals with the OpenGL context. By default, morph::Visual uses the library GLFW3 to create a Window and an associated OpenGL context for all the drawing (if you want two windows in your program, you'll need two morph::Visuals).

Within your scene, you will then create one or more instances of a `morph::VisualModel`. This is the base class for a number of different classes that draw different kinds of objects. If you have a custom visualization need, you can derive your own class from  `morph::VisualModel`.