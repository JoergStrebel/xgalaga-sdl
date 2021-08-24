#! /bin/bash

autoreconf -i
./configure --datadir=`pwd` --bindir=`pwd`/bin
make clean
make -j4

