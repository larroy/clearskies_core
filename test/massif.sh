#!/bin/bash
valgrind --tool=massif build/out/Debug/unit_test
# see results with ms_print massif.out..
