
#ifndef __BLOOM_H__
#define __BLOOM_H__

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct bloom_t bloom;

/**
 * description	: initialise a bloom_t object. please attention
 *				  that you should destroy it by invoking @destroy_bloom
 *				  when you do not wanna use it again. <br/><br/>
 *
 * in-@size		: item number
 *
 * out-@ret		: if successful, ret is the bloom_t object;
 *				  otherwise nil instead.
 **/
bloom* create_bloom(unsigned size);
/**
 * description	: construct a bloom_t object by the bloom
 *				  string buffer @buf. please attention
 *				  that you should destroy it by invoking @destroy_bloom
 *				  when you do not wanna use it again. <br/><br/>
 *
 * out-@ret		: if successful, ret is the bloom_t object;
 *				  otherwise nil instead.
 **/
bloom* build_bloom(const char* buf, size_t size);
/**
 * description	: deconstruct a bloom_t object. <br/><br/>
 *
 * out-@ret		: the return value is void.
 **/
void destroy_bloom(bloom* filter);

/**
 * description	: insert a item into the bloom filter @bf <br/><br/>
 *
 * in-out@bf	: the bloom filter
 * in-@str		: a data item
 *
 * out-@ret		: the return value is void.
 **/
void bloom_add(const bloom* bf, const char* str);
/**
 * description	: find @str in the bloom filter @bf. <br/><br/>
 *
 * in-out@bf	: the bloom filter
 * in-@str		: a data item
 *
 * out-@ret		: if @bf contains @str, the return value is 0;
 *				  otherwise -1 instead if @bf or @str is nil
 *				  or -2 if @bf does not contain @str.
 **/
int bloom_find(const bloom* bf, const char* str);

/**
 * description	: get the memory size of the bloom_t object @bf <br/><br/>
 *
 * in-out@bf	: the bloom filter
 *
 * out-@ret		: if successful, ret is @bs's memory size;
 *				  otherwise 0 instead if @bf is nil.
 **/
unsigned bloom_mem_size(const bloom* bf);
/**
 * description	: copy the memory content of the bloom_t object @bf
 *				  to a string buffer @buf whose length is @size. <br/><br/>
 *
 * in-out@bf	: the bloom filter
 * in-out@buf	: the string buffer
 * in-@buf_len	: @buf's length
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead if
 *				  @bf is nil or @buf is nil or @size is 0 or @size is
 *				  less than @bf's size.
 **/
int bloom_serialize(const bloom* bf, char* buf, size_t size);

#ifdef __cplusplus
	}
#endif

#endif

