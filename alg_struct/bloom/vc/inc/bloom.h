
#ifndef __BLOOM_H__
#define __BLOOM_H__

// #ifdef __cplusplus
// extern "C" {
// #endif

typedef struct bloom_t bloom;

bloom* create_bloom(unsigned size);
void destroy_bloom(bloom* filter);

bloom* build_bloom(const char* buf, size_t size);

void bloom_add(const bloom* bf, const char* str);
int bloom_check(const bloom* bf, const char* str);

unsigned bloom_mem_size(const bloom* bf);
int bloom_serialize(const bloom* bf, char* buf, size_t size);

//#ifdef __cplusplus
//}
//#endif

#endif

