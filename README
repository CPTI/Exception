Exception
=========

Exception is debugging and logging library for C++. It includes a hierarchy of exceptions
like Java's and support for obtaining backtraces in linux and windows. On linux a low-level
C++ runtime library hook is wrapped to provide backtraces for all thrown exception.

The logging library tries to be as unobstructive as possible and has a nice integration with
the exception and backtracing library.

This library was originally written before C++11 was widely available. Because it requires
a threading library and also shared pointers it was originally built on Qt, but now can be
compiled with C++11 as well.

Building
========

There are two build systems here, one based on qmake and cmake. The qmake system is 
only used internally at CPTI, where the library orinated as a submodule. It is recommended
to use the cmake build:

#> mkdir build
#> cd build
#> cmake path_to_source -DCMAKE_INSTALL_PREFIX=/usr
#> make
#> make install
