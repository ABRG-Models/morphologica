---
title: Why was morphologica developed?
parent: Welcome
layout: page
permalink: /why
---
Why expend the time writing an OpenGL visualization system when there are alternatives, like Python's matplotlib already available?

I think the the answers are 'opportunity', 'preference' and 'benefit'.

'Opportunity' because I started a project in 2018 with Stuart Wilson at the University of Sheffield's Department of Psychology in which we were going to solve reaction-diffusion equations and there was a requirement to plot the results of those computations. I had a free hand in how to go about the work and plenty of time on a 4 year project.

'Preference', because I just don't really like Python very much. I dislike its lack of scope identifiers, which make it awkward to copy blocks of code around and I find the management of Python packages a headache. On the other hand, I do really like C++.

If the resulting visualization system had not had any promise of showing a benefit over matplotlib or MATLAB, I'd still not have started it and would have lived with the least bad alternative. But visualization based on computer gaming technology had the promise of being lightning fast with minimal use of computational resources. I really wrote morphologica so that I could have insightful graphics while still getting the most I possilbly could out of the CPU hardware I had access to.

Could I have used an alternative C++ plotting system? Possibly. Alternatives include CERN's [Root](https://root.cern/), (ugly graphs, too complex), [VTK](https://vtk.org/) from Kitware, (too big, too complex) and [matplot++](https://alandefreitas.github.io/matplotplusplus/), which copies the Python matplotlib API which I never got on with!

What I have ended up with is a library of code which is lightweight, very easy to incorporate into projects and which can animate 3D plots with very low CPU overhead. It was worth the effort!