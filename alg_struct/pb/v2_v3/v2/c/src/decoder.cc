#include "msg.pb.h"
#include "msg_array.pb.h"
#include <fstream>
#include <string>
#include <stdio.h>

using namespace std;

int main(int argc, char** argv)
{
    const char* file = "../../pb_bin";
    if (argc == 2) {
        // fprintf(stderr, "Pls input protobuf sirialized file!!\n");
        // return -1;
        file = argv[1];
    }

    ::proto2_proto_example::MessageArray ma;
    fstream input(file, ios::in | ios::binary);
    if (!ma.ParseFromIstream(&input)) {
        printf("Failed to parse %s.\n", file);
        return -1;
    }

    // ::google::protobuf::Map< ::std::string, ::proto2_proto::Message> _msg_map = ma.msg_map();
    ::google::protobuf::Map< ::std::string, ::proto2_proto::Message>::const_iterator
        it = ma.msg_map().begin();
    for (; it != ma.msg_map().end(); it++) {
        printf("key:%s\n", it->first.c_str());
        printf("  msg.name:%s\n", it->second.name().c_str());
        printf("  msg.hilarity:%u\n", it->second.hilarity());
        printf("  msg.height_in_cm:%u\n", it->second.height_in_cm());
        printf("  msg.data:%s\n", it->second.data().c_str());
        printf("  msg.result_data:%lld\n", it->second.result_count());
        printf("  msg.true_scotsman:%d\n", it->second.true_scotsman());
        printf("  msg.score:%f\n", it->second.score());
        printf("  msg.score num:%u\n", it->second.key().size());
        for (int index = 0; index < it->second.key().size(); index++) {
            printf("  msg.score[%d]:%llu\n", index, it->second.key(index));
        }
    }

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}

