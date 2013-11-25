Ray Tracing in C++
==================

Implementation of a ray tracer in C++ for [CS 6620](http://www.cemyuksel.com/courses/utah/cs6620/) at the [University of Utah](http://www.utah.edu/), Fall 2013


Projects
--------

Throughout the course, our ray tracer has evolved and tested with more advanced scenes. Each project has a dedicated project page to highlight the progression of the ray tracer:

  - [Project 1](prj1.html)
  - [Project 2](prj2.html)
  - [Project 3](prj3.html)
  - [Project 4](prj4.html)
  - [Project 5](prj5.html)
  - [Project 6](prj6.html)
  - [Project 7](prj7.html)
  - [Project 8](prj8.html)
  - [Project 9](prj9.html)
  - [Project 10](prj10.html)
  - [Project 11](prj11.html)
  - [Project 12](prj12.html)
  - [Final Project](prj13.html)


Compiling and Running
---------------------

The main file (*ray-tracer.cpp*) must be compiled using C++11, since it uses threading. There is a *run* script developed for Mac to assist with compiling.

When run, the program will create binary PPM image files in the *images/* folder, which can then be converted to other image formats.

The provided script takes an integer parameter to compile, run, and convert images for the user.

    ./run 0    # cleanup
    ./run 1    # compile
    ./run 2    # compile & package
    ./run 3    # compile & run
    ./run 4    # compile & run & convert
    ./run 5    # compile & run & convert & open
    ./run 6    # compile & run & convert & open & cleanup


Credit
------

[Cem Yuksel](http://www.cemyuksel.com/) for both his [cyCodeBase library](http://www.cemyuksel.com/cyCodeBase/) and a bulk of the original code that inspired and guided the inner workings of the ray tracer.
