Project 8 - Anti-Aliasing
=========================

Ray tracer now uses adaptive multi-sampling. This smooths out our scene, since our original single samples were at only one finite point in our pixels, rather than all possible points in our scene (which is infinite). This results in a significant slow-down in our ray tracer, so there is a delicate balance in adjusting the anti-aliasing variables and speed. This will be a critical factor in later projects as well.


- - -


Textured Scene
--------------

![](images/prj8/scene.png)

- - -

Textured Scene - Sample Count
-----------------------------

![](images/prj8/sceneSample.png)

- - -

Textured Scene - More Samples
-----------------------------

![](images/prj8/scene-h.png)

- - -

Details
-------

[*Specs*](specs.html)

| *render time*  | minimum samples | maximum samples | variance |
| -------------: | --------------: | --------------: | -------: |
|       26.107 s |               4 |              32 |    0.001 |
| 1 min 33.490 s |               6 |              64 |   0.0001 |

- - -
