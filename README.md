# SOFA Front End Viewer

Viewer for the SOFA (Simulation Open Framework Architecture) simulations. 
Possibility to edit over a network, simplified creation of new simulations.

It is built as a generic viewer using Qt and OpenGL, with easy to create plugins.
The GUI is composed of a main 3D view, a graph view and zones where the plugins can add buttons, widgets and menus.

## Plugins
Proposed plugins are the following:

* SOFA Front End Local

Using the Sofa Front End library developped by Digital Trainers (currently still private), which makes interfacing with SOFA easier.
Basic visualisation of SOFA simulations, custom rendering (in another thread than the simulation), properties edition with instantaneous updating.
The user can create a SFE server from the opened simulation, which can then be accessed by another application running the next plugin.

* SOFA Front End Client

Connects over the network to a SFE server. Same functionalities than SFE Local (rendering, edition, ...)

* Model viewer

Using Assimp, open any 3d model for basic viewing.

* SOFA Graph Abstraction

Simplified creation of SOFA simulations using higher level objects.
Import one or multiple 3d models, then add physics and collision to them using more accessible abstractions (rigid, deformable, cloth, ...)
Run the simulation in the viewer, or export it as a SOFA scene.
