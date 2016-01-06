#!/bin/bash

## USAGE: ./module_build.sh /path/to/ns-3/root/directory
# 


# copy manet-routing example to examples folder
# mv -f LCONFIDANT/manet-* $1/ns-3.22/examples/routing/

# cp -R LCONFIDANT/* $1/ns-3.22/src/aodv/model



cd $1/ns-3.22

./waf clean

CXXFLAGS="-std=c++0x" ./waf configure

cd ..

./build.py --enable-examples --enable-tests --qmake-path=/usr/bin/qmake-qt4 --build-options=--progress -- --enable-sudo --enable-mpi --with-python=/usr/bin/python2 --prefix=/usr --progress
