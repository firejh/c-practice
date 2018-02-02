#!/bin/sh
mkdir -p bin  dev  etc  home  lib  lib64  mnt  opt  proc  root  run  sbin  sys  tmp  usr  var
cp -rf /bin/* ./bin/*
mkdir -p ./usr/bin/
cp -rf /usr/bin/* ./usr/bin/*
