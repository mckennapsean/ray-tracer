Project 10 - Soft Shadows & Glossy Surfaces
===========================================

Multi-sampling provides more useful features for our ray tracers: softer shadows from area lights and estimation of rougher surfaces with glossiness added to our reflections and refractions. For generating soft shadows, I used a combination of deterministic ray generation followed by quasi-random sampling using the Halton sequence. I did this so that I sampled the outer edges of the light disk (sphere) before doing the random sampling. This gives us softer penumbras, at the cost of more initial shadow samples.


- - -


Glossy Spheres
--------------

![](images/prj10/original.png)

![](images/prj10/glossy.png)

![](images/prj10/glossy-h.png)

- - -

Softer Shadows
--------------

![](images/prj10/soft-8-64.png)

![](images/prj10/soft-8-64-two-determ.png)

![](images/prj10/soft-8-64-four-determ.png)

- - -

Many Colors
-----------

to come...

- - -

Buggy Images
------------

![](images/prj10/bug.png)

- - -

Details
-------

[*Specs*](specs.html)

Glossy Spheres:

| *render time*  | minimum samples | maximum samples | color variance |
| -------------: | --------------: | --------------: | -------------: |
| 3 min 25.311 s |               8 |              32 |         0.0001 |
| 7 min 03.998 s |              16 |              64 |         0.0001 |

Many Colors:

to come...

| *render time*   | minimum samples | maximum samples | color variance | minimum shadow samples | maximum shadow samples |
| --------------: | --------------: | --------------: | -------------: | ---------------------: | ---------------------: |
| 19 min 08.591 s |              16 |             128 |         0.0001 |                      8 |                     32 |

- - -
