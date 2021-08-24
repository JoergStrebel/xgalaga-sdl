#! /bin/bash

autoreconf -i
./configure 
make clean
make -j4

