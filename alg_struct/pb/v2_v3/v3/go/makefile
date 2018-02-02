all: gpb encoder decoder

#chmod u+x ./mk.sh && source ./mk.sh

gpb:
	-sh ./pb.sh

encoder:encoder.go
	-go build -race -o $@ $<

decoder:decoder.go
	-go build -race -o $@ $<

clean:
	-rm src/proto3_proto/msg.pb.go
	-rm src/proto3_proto_example/msg_array.pb.go
	-rm decoder encoder

