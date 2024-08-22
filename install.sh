#!/bin/sh
cmake -S . -B build/
cd build
make
sudo mv FileCounter /usr/local/bin/count
cd ..
rm -rf build
count
