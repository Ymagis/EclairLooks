|Build Status|

Eclair Looks
============

EclairLooks is a project centered around color manipulation. For now, it is a
color processing tool that allows quick preview of various color
transformations on still images and browse through library of 3DLUT. The
project is currently in development.

Current features include :

-  Image viewer

   - OpenGL accelerated
   - Handle all formats supported by OpenImageIO
   - Image scopes (Parade, Waveform, Vectorscope)
   - Transformation scopes (Curves, 3D Cube)

-  Color Transformations

   - OpenColorIO FileFormat, Colorspace and Matrix
   - Color Transformation Language (CTL)

-  Look library

   - Handle all formats supported by OpenColorIO


Roadmap
-------

*In progress...*

Building
--------

The build uses CMake, most of the dependencies are downloaded and built as part
of the process. The only requirements are a C++17 compliant compiler with the
Filesystem library and Qt.

These are expected to be installed with Brew (with
a path prefix of `/usr/local/opt`), additional dependencies can be added to
expand the range of formats supported by OpenImageIO. The CI pipeline can be
used as a guide (Travis).

Debug build :

::

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make -j

Release build :

::

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j
    make package


Installation
------------

A macOS package is available for download on the Release page.

Requirements :

-  Platform :

   -  Development only on macOS (should work on other platforms with little
      efforts)
   -  Tested on : macOS 10.14

About
-----

Eclair_

.. |Build Status| image:: https://travis-ci.org/Ymagis/EclairLooks.svg?branch=master
   :target: https://travis-ci.org/Ymagis/EclairLooks


.. _Eclair: https://eclair.digital