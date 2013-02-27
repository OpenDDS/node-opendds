# OpenDDS module for Node.js

## Requirements:
* OpenDDS (http://www.opendds.org) and its dependencies must be installed.
* Make sure all of the OpenDDS development environment variables are set before running npm to install this module.  If you use the OpenDDS configure script, these variables can be set using "setenv.sh" or "setenv.cmd" depending on your platform.

## Tested platforms:
* Linux (RHEL6.3, Ubuntu12.04) x86_64 using the distro-supplied GCC
* Windows 7 x86_64 using Visual Studio 2010
* Other OpenDDS-supported platforms should work, but may required changes to binding.gyp

## To build the module (development only, users can simply use npm -- see above)
> $ node-gyp configure build
