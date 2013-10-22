Project 7 - Textures
====================

This project required a significant rewrite of a good chunk of the base code in order to effectively map textures onto objects in our scene. Textures can now be loaded as PPM images and then mapped to our objects (unit planes, spheres, triangular mesh objects as ready-made models). Our scene files now include background and environment textures, in order to give us a true surrounding scene. Lastly, texture filtering is implemented using simple cones that are sent as our rays into the scene, in order to approximate the screen-space derivative texture coordinates.


- - -


Textured Scene
--------------

![](images/prj7/texturing.png)

- - -

Fun Failures
------------

![](images/prj7/clean.png)

![](images/prj7/hit-coord.png)

![](images/prj7/trippy.png)

- - -

Details
-------

[*Specs*](specs.html)

*render time:* **5.149** s

- - -
