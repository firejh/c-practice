package main

import (
	"fmt"
	"io/ioutil"
)

import (
	"github.com/golang/protobuf/proto"
	"proto2_proto"
	"proto2_proto_example"
)

func main() {
	var ma proto2_proto_example.MessageArray
	ma.MsgMap = make(map[string]proto2_proto.Message)
	for idx := 0; idx < 10; idx++ {
		var msg proto2_proto.Message
		msg.Name = fmt.Sprintf("%s%d", "hello", idx)
		msg.Hilarity = proto2_proto.PUNS
		msg.HeightInCm = 123456
		msg.Data = []byte("world")
		msg.ResultCount = 123456
		msg.TrueScotsman = true
		msg.Score = 123.456
		msg.Key = make([]uint64, 100)
		for index := 0; index < 96; index++ {
			msg.Key[index] = uint64(index)
		}

		ma.MsgMap[msg.Name] = msg
	}

	data, err := proto.Marshal(&ma)
	if err != nil {
		fmt.Println("marshaling error: ", err)
	}
	//fmt.Println("data:", string(data))

	ioutil.WriteFile("../../pb_bin", data, 0666)
}
