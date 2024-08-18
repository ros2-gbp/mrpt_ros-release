#!/bin/sh
# LD_LIBRARY_PATH=${PWD}/src/.libs/ valgrind --leak-check=full ./src/.libs/libfyaml-parser -mscan "$1" 2>&1 | tee valgrind.log
LD_LIBRARY_PATH=${PWD}/src/.libs/ valgrind --tool=massif ./src/.libs/libfyaml-parser $* 2>&1 | tee massif.log
