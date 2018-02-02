#include "msg.pb.h"
#include "msg_array.pb.h"
#include <fstream>
#include <string>
#include <stdio.h>

using namespace std;

int main(int argc, char** argv)
{
    ::proto3_proto_example::MessageArray ma;
    ::google::protobuf::Map< ::std::string, ::proto3_proto::Message>*
        msg_map_map = ma.mutable_msg_map();

    char name[64] = {0};

    for (int idx = 0; idx < 10; idx++) {
        sprintf(name, "%s%d", "hello", idx);
        ::proto3_proto::Message msg;
        msg.set_name(name);
        msg.set_hilarity(::proto3_proto::Message_Humour_PUNS);
        msg.set_height_in_cm(123456);
        msg.set_data("world");
        msg.set_result_count(123456);
        msg.set_true_scotsman(true);
        msg.set_score(123.456f);
        for (unsigned long index = 0; index < 96; index++) {
            msg.add_key(index);
        }

        printf("msg size:%d\n", msg.ByteSize());
        (*msg_map_map)[name] = msg;
    }

    printf("msg array size:%d\n", ma.ByteSize());

    fstream output("../../pb_bin", ios::out | ios::trunc | ios::binary);
    if (!ma.SerializeToOstream(&output)) {
        printf("Failed to write msg.\n");
        return -1;
    }

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}

