#!/bin/sh

export QT_PATH=$HOME/Qt5.12.8/5.12.8/clang_64/
export MACOSX_DEPLOYMENT_TARGET=10.13 
export CMAKE_PREFIX_PATH=$QT_PATH:$CMAKE_PREFIX_PATH 

rm -rf build
mkdir build
cd build
cmake -DQT4BUILD=0 -DSDL1BUILD=0 ..
make package
