---
title: morph::VisualOwnable
parent: Internal classes
grand_parent: Reference
permalink: /ref/visualint/visualownable
layout: page
nav_order: 9
---

`VisualOwnable` and `VisualOwnableMX` derive from `VisualBase` and add
functionality that requires OpenGL function calls, but without adding
any windowing system specific functionality.

This class is a morphologica-internal class and there is typically no
access of its methods in morphologica client code.

However, if you want to incorporate morphologica graphics into another
windowing system, this class provides the drawing functionality. For examples, see [morph/qt/viswidget.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/qt/viswidget.h) and [morph/qt/viswidget_mx.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/qt/viswidget_mx.h) for Qt and [morph/wx/viswx.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/wx/viswx.h) for wxWidgets.
