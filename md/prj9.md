Project 9 - Depth of Field
==========================

We now have the ability to use our circle of confusion to simulate depth of field in a scene. This requires our camera to now contain information about the focal distance to the plane that is in focus along with how large our circle of confusion is (aka how blurry things off our focal plane get). By simply changing the rays being cast in the scene with quasi-random sampling and random rotations of those samples on our circle of confusion, we get depth of field for free.


- - -


Many Spheres
------------

![](images/prj9/spheres.png)

- - -

Many Spheres - Sample Count
---------------------------

![](images/prj9/spheresSample.png)

- - -

Many Spheres - More Samples
---------------------------

![](images/prj9/spheres-h.png)

![](images/prj9/spheresSample-h.png)

- - -

Buggy Images
------------

![](images/prj9/glitched.png)

![](images/prj9/glitched-2.png)

- - -

Details
-------

[*Specs*](specs.html)

| *render time*  | minimum samples | maximum samples |
| -------------: | --------------: | --------------: |
|       49.136 s |               8 |              32 |
| 1 min 45.240 s |              16 |              64 |

| 7 min 50.853 s |              16 |              64 |

- - -
