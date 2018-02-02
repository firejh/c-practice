#!/usr/bin/env bash
# ******************************************************
# DESC    :
# AUTHOR  : Alex Stocks
# VERSION : 1.0
# LICENCE : Apache License 2.0
# EMAIL   : alexstocks@foxmail.com
# MOD     : 2017-09-04 22:53
# FILE    : pb.sh
# ******************************************************

# descriptor.proto
gopath=~/test/golang/lib/src/github.com/gogo/protobuf/protobuf
# If you are using any gogo.proto extensions you will need to specify the
# proto_path to include the descriptor.proto and gogo.proto.
# gogo.proto is located in github.com/gogo/protobuf/gogoproto
gogopath=~/test/golang/lib/src/
# ../proto the path of msg.proto
# protoc -I=../proto/ --gogoslick_out=src/proto3_proto msg.proto
protoc -I=$gopath:$gogopath:../go_proto/ --gogoslick_out=src/proto3_proto msg.proto
protoc -I=$gopath:$gogopath:../go_proto/ --gogofast_out=Mmsg.proto="proto3_proto":src/proto3_proto_example msg_array.proto
