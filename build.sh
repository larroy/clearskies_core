#!/bin/bash
gyp -f ninja test/test.gyp --depth=. --generator-output=build
ninja -C build/out/Default/ -v
