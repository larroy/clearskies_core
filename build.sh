#!/bin/bash

set -e

# TODO: Remove this once the bug in GYP is fixed!
# https://groups.google.com/forum/#!topic/gyp-developer/m7LUzJwL9Zc
if [[ `uname` == "Darwin" ]]; then
    export CC="`which clang++` -std=c++11 -stdlib=libc++"
    export CXX="`which clang++` -std=c++11 -stdlib=libc++"
fi

gyp -f ninja test/test.gyp --depth=. --generator-output=build
ninja -C build/out/Debug/ -v
#ninja -C build/out/Release/ -v
