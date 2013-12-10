Project 12 - Photon Mapping
===========================

Another method for calculating our scene lighting is implemented: photon mapping. Now, we trace photons through our scene to create a 3D volume of irradiance throughout our scene. This is then queried when running our ray tracers to get direct or indirect lighting from this computation.


- - -


Cornell Box
-----------

![](images/prj12/cornellBox.png)

- - -

Buggy Images
------------

![](images/prj12/buggy0.png)

![](images/prj12/buggy1.png)

![](images/prj12/buggy2.png)

![](images/prj12/buggy3.png)

![](images/prj12/buggy4.png)

![](images/prj12/buggy5.png)

- - -

Details
-------

[*Specs*](specs.html)


Cornell Box:

| *render time*  |
| -------------: |
|       38.463 s |

- - -
