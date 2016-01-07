#!/bin/bash
## USAGE: ./apply_patches.sh /path/to/ns-3.22/root/directory

path=$1

cp confidant.patch $path
cp confidant_run.patch $path

cd $path

patch -p2 < confidant.patch

patch -p2 < confidant_run.patch
