# SOFA Front End Viewer

Generic viewer used mainly for [SOFA](http://www.sofa-framework.org/) (Simulation Open Framework Architecture) simulations. 

It is built as a generic viewer using Qt and OpenGL, and plugins.  
The GUI is composed of a main 3D view, a graph view and zones where the plugins can add buttons, widgets and menus.  
A property system is used for the edition of objects in the graph. It is based on templates, supporting standard C++11 types (plain old data, vectors, arrays, and vectors of arrays or 2d arrays). A serialization of the graph to and from xml files is proposed. 

The plugins add document types, each document associated with specific file types. The documents must manage the OpenGL rendering but a very basic render library is used for most of the created plugins.

## Plugins
Existing plugins are the following:

### Model viewer

Using [Assimp](http://www.assimp.org/), open any 3d model for viewing and basic editing.  
Materials can be modified, nodes can be moved and rotated.  
Use **double click** on a graph item to access its properties, **right click** to add or remove a item (here only for the Nodes and Instances).

![ModelViewer](http://i.imgsafe.org/da91492.jpg)

### SOFA Front End Local

Basic visualisation of SOFA simulations, custom rendering (in another thread than the simulation), properties edition with instantaneous updating.
This plugin use the [Sofa Front End](http://www.digital-trainers.com/sofa-front-end-en/) library developed by Digital Trainers (currently still private), which makes interfacing with SOFA easier.

![Caduceus](http://i.imgsafe.org/d716030.jpg)

This offers the same basic features as the runSOFA application bundled with the SOFA framework. **Double click** on an item in the graph to open its properties. For the time being, the graph is not automatically updated, it is necessary to click the "Update graph" button.

![Properties](http://i.imgsafe.org/dd61fe7.jpg)

**Note**: when opening scenes using resources (meshes, textures) in another directory, the latter must be added to the Sofa paths list. On the first time opening a Sofa scene, a dialog box will ask to fill this list. If you will open Sofa examples, you must add "[...]/**Sofa/share**".

![SofaPaths](http://i.imgsafe.org/e95211d.jpg)

#### SFE Server

The user can create a SFE server from the opened simulation, which can then be accessed by another application running the SFE Client plugin.  
Go to *Tools/Launch server*, modify the parameters if necessary, and click *Ok*.  
The *Read only* parameter make it so that the server rejects client calls that would modify the simulation. It is recommended to leave it unchecked for now, as some actions on the client side will indefinitely block.

![Server](http://i.imgsafe.org/dec683a.jpg)

### SOFA Front End Client

Connect over the network to a SFE server. Same features as SFE Local (rendering, edition, ...)  
Multiple clients can connect to a single server, view and modify the properties of the simulation.

Once a server is running, go to *File/New*, select "Sofa Client". Enter the address and port of the server, and click *Ok*.

![Client](http://i.imgsafe.org/d874778.jpg)

### SOFA Graph Abstraction

Simplified creation of SOFA simulations using higher level objects.  
This uses the SGA library created for [Blender SOFA](http://www.digital-trainers.com/blender-sofa-en/), for adding SOFA physics to graphical scenes.

The workflow is usually:

#### Create an empty document
Go to *File/New*, select *Sofa Graph Abstraction*.  
An empty graph will be created. It is possible at any time to save the document as an XML file (with the .sga extension) and reload it later.

#### Import 3d models
Go to *Tools/Import mesh*. All models types supported by Assimp can be imported.

![SGA_Import](http://i.imgsafe.org/e4aa0cc.jpg)

If you click the *Run* button at this step, a dialog box will open asking for the type of simulation to use (*Constraints Collision* is usually the best choice), but then the screen will turn black. Click *Run* again to stop and return to the edition mode.  
We now have to add roles to each model (physics, collision, visual).

#### Setting roles

A **right click on an Instance** will open a context menu from which it is possible to add a role to an object.

![SGA_AddPhysics](http://i.imgsafe.org/e059d10.jpg)

Then the type must be chosen.

![SGA_SelectPhysics](http://i.imgsafe.org/e7c3155.jpg)

A **double click** on the created object will open its properties.

![SGA_scene](http://i.imgsafe.org/e6d9a2d.jpg)

#### Execution

The *Run* button will convert the graph to a SOFA simulation, and run it. Clicking it again to stop the simulation (destroy it), and return to the edition mode.

#### Export to SOFA scene

We then can export the scene. Go to *Tools/Export Sofa scene* and save the file.  
For now the export does not create separate files for the meshes and the materials. The latter can be lost in the operation.  
It is then possible to load the scene using the SFE Local plugin (*File/Open*, choose the exported file)

We see here the SOFA graph instead of the simplified SGA one.  

![SGA_Export](http://i.imgsafe.org/e31c0ad.jpg)
