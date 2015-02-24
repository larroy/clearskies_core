clearskies_core
===============

[![Build Status](https://travis-ci.org/larroy/clearskies_core.png?branch=master)](https://travis-ci.org/larroy/clearskies_core)

Open source, distributed data synchronization software using the clearskies protocol.

[Protocol description](https://github.com/jewel/clearskies/blob/master/protocol/core.md)

# Status of the project

* This project is **abandoned**, but is left here as a reference, especially wrt uvpp, sqlitepp and
  vector clock management, reasons follow:
 - The protocol should be binary for efficiency.
 - I would use FRP patterns to handle events instead of callbacks.
 - In retrospective using the visitor pattern was a lot of boilerplate.
 - There's [Syncthing](http://syncthing.net/) that while is programmed in go, covers most of my use cases.


* Early stages of development, there's nothing here for users. Only if you are willing to help and
  contribute. **THIS SOFTWARE IS NOT YET FUNCTIONAL**

* For a functioning synchronization client please go to https://github.com/syncthing


# Contributing

We would like you to join and help with development, documentation or graphic design. Please post to
the group first to avoid duplicating efforts.


* See [ARCHITECTURE](http://larroy.github.io/clearskies_core/architecture) for a high level description of the software.
* See [CONTRIBUTING](http://larroy.github.io/clearskies_core/contributing)

# Donate

If you can't dedicate some time to the project you can still give support by donating some money such as **Bitcoin** to the following address:

    18QEEJuR2ACAunMVeAxZ46o4QprxFKyGkw

Or through the [bountysource fundraiser](https://www.bountysource.com/fundraisers/551-clearskies-open-source-file-synchronization)

# Dependencies

* boost >= 1.49
* libboost-test-dev
* libboost-filesystem (v3)
* libsqlite3-dev
* ninja  (ninja-build on debian)

You can use one of the `scripts/setup_environment...`  scripts to install the dependencies or as a
guide to install the required dependencies manually.

# Included dependencies

* libuv (vendor/libuv)
* gyp (tools/gyp) WARNING: watch out for interactions with the installed system gyp modules! this
  can produce errors as the python module search path might use the ones on the system instead of
  the installed one.

# Supported compilers

Development is done with **GCC 4.8** and boost 1.54, so far this is the only compiler guaranteed to be able to
build the project. (My development machine runs Debian GNU/Linux testing (jessie))

The project is implemented in C++11, the compiler should support the features used from this
standard. The following compilers should also work:

* GCC >= 4.7
* Visual Studio 2013
* Clang

Our target is to support the following platforms in order of development effort: Linux, Android, Windows, Mac, iOS.

# Build instructions

    ./build.sh
    ./test.sh

# Status of the project

- Low level assembling of messages and payload completed
- Implementing the CS protocol
- Integrating required libraries
- Implementing share infrastructure


## Discussion group

https://groups.google.com/forum/#!forum/clearskies-dev




