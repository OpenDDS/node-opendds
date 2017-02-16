# Library of Stubs for unused V8 functions

## Purpose

When using "opendds_idl -Wb,v8" the generated shared library will have
unresolved external dependencies on some V8 functions.  That library can be
loaded in to the nodejs process without any issues.

However, a user may want the same library to be used in OpenDDS C++ programs.
In order to do this, the V8 functions in question must be defined, but they
don't have to do anything since they won't be called.

An example in this repository is the generated code in /test/idl and the C++
test program in /test/test_publisher.cpp.  Without these stubs, that program
would fail to link.

## Build

1. Set up the OpenDDS build environment using setenv.sh or setenv.cmd
2. Generate a makefile or project files by running
> mwc.pl -type [gnuace|vc__]
3. Build using make or Visual C++

## Use

The application executable should link to this library.
See /test/test_publisher.mpc for an example using MPC.
