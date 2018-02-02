package main

import (
	"fmt"
	"io/ioutil"
	"os"
)

import (
	// "github.com/golang/protobuf/proto"
	"proto3_proto_example"
)

func main() {
	ma := &proto3_proto_example.MessageArray{}
	var fileName string = "../../pb_bin"
	if 2 == len(os.Args) {
		fileName = os.Args[1]
	}

	data, _ := ioutil.ReadFile(fileName)
	// proto.Unmarshal(data, ma)
	ma.Unmarshal(data)

	fmt.Println(len(data))
	msg_map := ma.MsgMap
	fmt.Println("MessageArray.msg_map size:", len(msg_map))
	for k, v := range msg_map {
		fmt.Println("key:", k)
		fmt.Println("  msg.name:", v.Name)
		fmt.Println("  msg.hilarity:", v.Hilarity)
		fmt.Println("  msg.height_in_cm:", v.HeightInCm)
		fmt.Println("  msg.data:", v.Data)
		fmt.Println("  msg.result_data:", v.ResultCount)
		fmt.Println("  msg.true_scotsman:", v.TrueScotsman)
		fmt.Println("  msg.score:", v.Score)
		fmt.Println("  msg.score num:", len(v.Key))
		for index, value := range v.Key {
			fmt.Println("  msg.score[", index, "]:", value)
		}
	}
}
