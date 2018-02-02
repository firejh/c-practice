#!/bin/sh
export GOPATH=$PWD:$GOPATH
make -f makefile clean
make -f makefile
