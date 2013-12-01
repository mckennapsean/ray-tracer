Project 11 - Monte Carlo Global Illumination
============================================

As if we are not sending enough samples, now our ray tracers will shoot multiple light rays at every hit in order to estimate the indirect illumination at any particular pixel of our image. This cuts out the need to hack in indirect illumination with an ambient light (though, the global illumination has been written as its own ambient light that sends multiple rays). Lastly, to optimize the speed of our Monte Carlo global illumination calculations, we can implement an irradiance cache that runs before our ray tracer in order to pre-compute and interpolate our indirect illumination, giving us better and faster smooth global illumination. Lastly, we are now adjusting our final output colors in order to reverse the gamma correction that standard monitors output.


- - -


Skylight Teapot
---------------

![](images/prj11/skylightTeapot-none.png)

![](images/prj11/skylightTeapot-s.png)

![](images/prj11/skylightTeapot.png)

![](images/prj11/skylightTeapotSample.png)

- - -

Cornell Box
-----------

![](images/prj11/cornellBox-none.png)

![](images/prj11/cornellBox-none-gamma.png)

![](images/prj11/cornellBox-s.png)

![](images/prj11/cornellBox.png)

![](images/prj11/cornellBoxSample.png)

- - -

Irradiance Caching #1
---------------------

![](images/prj11/skylightTeapot.png)

![](images/prj11/skylightTeapotIRreg.png)

![](images/prj11/skylightTeapotIRz.png)

![](images/prj11/skylightTeapotIRnorm.png)

- - -

Irradiance Caching #2
---------------------

![](images/prj11/cornellBox.png)

![](images/prj11/cornellBoxIRreg.png)

![](images/prj11/cornellBoxIRz.png)

![](images/prj11/cornellBoxIRnorm.png)

![](images/prj11/cornellBoxIRnormSample.png)

- - -

Irradiance Caching #2 - Poor Choices
------------------------------------

![](images/prj11/cornellBoxIRnorm.png)

![](images/prj11/cornellBoxIRrand.png)

![](images/prj11/cornellBoxIRnorm-s.png)

- - -

Buggy Images
------------

![](images/prj11/bug0.png)

![](images/prj11/bug1.png)

![](images/prj11/bug2.png)

- - -

Details
-------

[*Specs*](specs.html)


Skylight Teapot:

| *render time*  | global illumination | indirect samples |
| -------------: | ------------------: | ---------------: |
|       03.013 s |                   - |                - |
| 1 min 49.769 s |                  GI |               16 |
| 4 min 39.872 s |                  GI |              128 |
|       36.957 s |                  IC |              128 |


Cornell Box:

| *render time*   | global illumination | indirect samples |
| --------------: | ------------------: | ---------------: |
|        07.028 s |                   - |                - |
| 02 min 03.465 s |                  GI |               16 |
| 15 min 25.159 s |                  GI |              128 |
|        44.978 s |                  IC |              128 |

- - -
