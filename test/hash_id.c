/******************************************************
# DESC    :
# AUTHOR  : Alex Stocks
# VERSION : 1.0
# LICENCE : Apache License 2.0
# EMAIL   : alexstocks@foxmail.com
# MOD     : 2018-02-08 23:17
# FILE    : a.c
******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint64_t ID;
#define ID_BITS (sizeof(ID) << 3)

inline ID get_hash_bits(int group_num)
{
    switch (group_num) {
    case 1:
        return 0x0000;
    case 2:
        return 0x0001;
    case 4:
        return 0x0003;
    case 8:
        return 0x0007;
    case 16:
        return 0x000F;
    case 32:
        return 0x001F;
    case 64:
        return 0x003F;
    case 128:
        return 0x007F;
    case 256:
        return 0x00FF;
    case 512:
        return 0x01FF;
    case 1024:
        return 0x03FF;
    case 2048:
        return 0x07FF;
    case 4096:
        return 0x0FFF;
    case 8192:
        return 0x1FFF;
    case 16384:
        return 0x3FFF;
    case 32768:
        return 0x7FFF;
    case 65536:
        return 0xFFFF;
    }

    return 0;
}

inline ID get_hash_id(ID id, int group_num)
{
    return id & get_hash_bits(group_num);
}

int main(int argc, char** argv)
{
    int i = 0;
    int j = 0;
    for (i = 1; i <= 65536; i <<= 1) {
        printf("%llu,", get_hash_id(0xFFFF, i));
    }
    printf("\n");
    return 0;
}

