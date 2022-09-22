# OpenDDS module for Node.js

## Requirements:
* OpenDDS (http://www.opendds.org) and its dependencies must be installed.
* Make sure all of the OpenDDS development environment variables are set before running npm to install this module.  If you use the OpenDDS configure script, these variables can be set using "setenv.sh" or "setenv.cmd" depending on your platform.
** The OpenDDS development environment can use either the source tree or the installed tree (the one created by "make install").
** If using the installed tree, set the DDS_ROOT environment variable using the installed dds-devel.sh script.
** If using security, you may need to ensure that OpenDDS is built with the same version of OpenSSL used by Node.js

## Tested platforms:
* Node LTS versions 8, 10, 12, 14, 16, 18
* Linux (Ubuntu 20.04) x86_64 using gcc 9.4.0
* Linux (Ubuntu 22.04) x86_64 using gcc 11.2.0 (w/ openssl-1.1.1q)
* Windows (Server 2022) x86_64 using Visual Studio Enterprise 2022
* macOS (11.7) x86_54 using clang 13.0
* Other OpenDDS-supported platforms should work, but may required changes to binding.gyp

## Building and running the tests
* In OpenDDS directory:
```
$ source setenv.sh
```
* In node-opendds directory:
```
$ npm install
$ mwc.pl -type gnuace
$ make
$ cd tests
$ ./run_test.pl
```

## To build the module (for development of this module, users can simply use npm)
```
$ node-gyp configure build
```

## Changelog

### Version 0.2.0

* Remove requirement for V8 TypeSupport generation and NodeQosConversion
** uses ValueReader/Writer Implementation for NaN / Node.js
** requires OpenDDS 3.22, will not build with older versions of OpenDDS
** slightly changes QoS object formatting to align with IDL types
* Improved version support and CI test coverage
** Good Coverage of Node.js 14, 16, 18 (Linux / macOS / Windows)
** Limited Coverage of Node.js 8, 10, 12 (Linux / Windows)

### Version 0.1.1

* Add support and test coverage for multiple LTS versions of Node (12, 14, 16)

### Version 0.1.0

* Add support for publishing from Node.js

### Version 0.0.9

* Support building this module using an installed OpenDDS

### Version 0.0.8

* Requires at least OpenDDS 3.13. It will not build with older versions of OpenDDS.
* Added support for Security-enabled versions of OpenDDS.
* Compatibility with Node.js version 10 and NAN 2.10.

### Version 0.0.7

* Added Apple macOS support.
* Works with Node.js version 8.

### Version 0.0.6

* Added Microsoft Visual C++ support.

### Version 0.0.5

* Updated for Node.js version 6.9.5 and V8 version 5.8.1.

### Version 0.0.4

* Now hosted on GitHub which required a version bump to tell npm about it.
* No code changes.

### Version 0.0.3

* Updated to use new abstract interface callback from DataReaderImpl to
execute as an atomic operation rather than in two stages.  This change
uses updates to OpenDDS starting with r5997 in subversion and any release
after 3.4.1.  This will not build with older versions of OpenDDS.

### Version 0.0.2

* Updated to use extended data reference counting available in OpenDDS as
of r5993 in subversion.  This change will be available in releases
starting with the one following OpenDDS 3.4.1.  This will not build with
the older versions of OpenDDS.
