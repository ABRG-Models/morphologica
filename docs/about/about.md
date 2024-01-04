---
title: What is morphologica?
layout: home
permalink: /about
parent: Welcome
nav_order: 1
---

morphologica is a library of **header-only C++** code for **data visualization**.

morphologica provides **simulation support facilities** for simulations of dynamical systems, agent-based models or, in fact, any program that needs dynamic, runtime visualization.

It helps with:

* **OpenGL Visualizations of your program while it runs**. A modern OpenGL visualization
  scheme called **[morph::Visual](/morphologica/visual/)**
  provides the ability to visualise 2D and 3D graphs
  of surfaces, lines, bars, scatter plots and quiver plots with minimal
  processing overhead.

* **Convenient maths**. Several of the headers provide easy ways to do some of the mathematical basics you're going to use regularly. Random number generators, vector maths, simple matrices, ranges, scaling and templated mathematical constants. You don't *have* to use them, but they're available.

* **Configuration**. morphologica allows you to easily set up a simulation
  parameter configuration system, using the JSON reading and writing
  abilities of **[morph::Config](https://github.com/ABRG-Models/morphologica/blob/main/morph/Config.h)**. ([morph::Config Example](https://github.com/ABRG-Models/morphologica/blob/main/examples/jsonconfig.cpp))

* **Saving data from your simulation**. morphologica provides a set of
  easy-to-use convenience wrappers (**[morph::HdfData](https://github.com/ABRG-Models/morphologica/blob/main/morph/HdfData.h)**) around the HDF5 C
  API. By saving data in a standard format, it is easy to access
  simulation data in python, MATLAB or Octave for analysis and graphing. ([HdfData Example](https://github.com/ABRG-Models/morphologica/blob/main/examples/hdfdata.cpp))

# What is this site for?

It's to help you to learn how to use the classes in morphologica in your own numerical simulations and data analysis programs.