package main

import (
	"fmt"
	"io/ioutil"
)

import (
	"github.com/golang/protobuf/proto"
	"proto3_proto"
	"proto3_proto_example"
)

func main() {
	var ma proto3_proto_example.MessageArray
	ma.MsgMap = make(map[string]*proto3_proto.Message)
	for idx := 0; idx < 10; idx++ {
		var msg proto3_proto.Message
		msg.Name = fmt.Sprintf("%s%d", "hello", idx)
		msg.Hilarity = proto3_proto.PUNS
		msg.HeightInCm = 123456
		msg.Data = []byte("world")
		msg.ResultCount = 123456
		msg.TrueScotsman = true
		msg.Score = 123.456
		msg.Key = make([]uint64, 100)
		for index := 0; index < 96; index++ {
			msg.Key[index] = uint64(index)
		}

		ma.MsgMap[msg.Name] = &msg
	}

	data, err := proto.Marshal(&ma)
	if err != nil {
		fmt.Println("marshaling error: ", err)
	}
	//fmt.Println("data:", string(data))

	ioutil.WriteFile("../../pb_bin", data, 0666)
}
