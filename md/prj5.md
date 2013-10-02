Project 5 - Triangular Meshes
=============================

Planes are now rendered, specifically unit planes that have a limited size, generating the walls in our test scene. Triangular meshes are also rendered from OBJ files, which specify the object's vertices and vertex normals. For each of the triangular faces of this object, the ray tracer finds intersections along the object. The code instead uses [the Moller-Trumbore algorithm](http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-9-ray-triangle-intersection/m-ller-trumbore-algorithm/) to compute the barycentric coordinates of a ray hit on a triangular face.


- - -


Box Scene (with teapot)
-----------------------

![](images/prj5/box.png)

- - -

Box Scene (low-resolution)
--------------------------

![](images/prj5/box-low.png)

- - -

Problems
--------

![](images/prj5/bounding-box.png)

![](images/prj5/funky-planes.png)

![](images/prj5/making-planes.png)

![](images/prj5/explosion.png)

![](images/prj5/killer.png)

- - -

Details
-------

[*Specs*](specs.html)

|                   | *render time*   |
| :---------------- | --------------: |
| *Bounding Box*    | 3 min 07.560 s  |
| *No Bounding Box* | 16 min 40.143 s |

- - -
