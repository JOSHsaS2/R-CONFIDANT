#!/bin/bash

## USAGE: ./module_build.sh /path/to/ns-3/root/directory
# 

rm -rf $1/ns-3.22/src/raodv
cd ..
cp -R raodv $1/ns-3.22/src

# copy manet-routing example to examples folder
cp -R raodv/test/manet-routing-compare.cc $1/ns-3.22/examples/routing/

cd $1
./build.py --enable-examples --enable-tests --qmake-path=/usr/bin/qmake-qt4 --build-options=--progress -- --enable-sudo --enable-mpi --with-python=/usr/bin/python2 --prefix=/usr --progress