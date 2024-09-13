#!/bin/bash

set -x

cd `pwd`/build
make clean
rm -rf `pwd`/*
cmake ..
make Server
make Client
