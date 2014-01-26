#!/bin/bash
gyp -f ninja test/test.gyp --depth=. --generator-output=build
ninja -C build/out/Debug/ -v
ninja -C build/out/Release/ -v
