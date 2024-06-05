#!/bin/bash

rm -rf build
mkdir build
pushd build
cmake -DCMAKE_BUILD_TYPE=relwithdebinfo ..
cmake --build .
popd

# popd

# echo continue && read -n 1
