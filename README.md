Ray Tracing in C++
==================

Implementation of a ray tracer in C++ for [CS 6620](http://www.cemyuksel.com/courses/utah/cs6620/) at the [University of Utah](http://www.utah.edu/), Fall 2013


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
