#!/bin/bash

rm -rf build
mkdir build -p
cd build

cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug ..
make
