---
title: Creating a new project
parent: Tutorials
layout: page
permalink: /ref/tutorials/newproject
---
This page will walk you through the process of setting up a new project to build morphologica code so that you can try the example code in the tutorials.

Because morphologica is header-only, it's very easy to work with. All you have to do is make sure your compiler knows where to find morphologica files, set a few flags in the compiler and link to a few essential libraries. The best way to coordinate all this is to use CMake from kitware, a widely used build management system.

Include the cmake readme I already wrote here.