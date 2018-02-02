
#include <stdlib.h>
#include <string.h>

#include "bloom.h"
#define BLOOM_VERSION		"cmppn_w112"  //"bloom_v001"

#define	RATIO				20
#define	BYTE_LEN			8
#define	SETBIT(array, pos)	(array[pos >> 3] |= (1 << (pos % BYTE_LEN)))
#define	GETBIT(array, pos)	(array[pos >> 3] & (1 << (pos % BYTE_LEN)))

/* A Simple Hash Function */
unsigned simple_hash(char *str);

/* RS Hash Function */
unsigned RS_hash(char *str);

/* JS Hash Function */
unsigned JS_hash(char *str);

/* P. J. Weinberger Hash Function */
unsigned PJW_hash(char *str);

/* ELF Hash Function */
unsigned ELF_hash(char *str);

/* BKDR Hash Function */
unsigned BKDR_hash(char *str);

/* SDBM Hash Function */
unsigned SDBM_hash(char *str);

/* DJB Hash Function */
unsigned DJB_hash(char *str);

/* AP Hash Function */
unsigned AP_hash(char *str);

/* CRC Hash Function */
unsigned CRC_hash(char *str);

typedef unsigned (*hash_func)(char* str);

hash_func g_hash[] = {
	simple_hash,
	RS_hash,
	JS_hash,
	PJW_hash,
	// ELF_hash,
	BKDR_hash,
	SDBM_hash,
	DJB_hash,
	AP_hash,
	// CRC_hash
};

/* A Simple Hash Function */
unsigned simple_hash(char *str)
{
	register unsigned hash;
	register unsigned char *p;

	for(hash = 0, p = (unsigned char *)str; *p ; p++) {
		hash = 31 * hash + *p;
	}

	return (hash & 0x7FFFFFFF);
}

/* RS Hash Function */
unsigned RS_hash(char *str)
{
	unsigned b = 378551;
	unsigned a = 63689;
	unsigned hash = 0;

	while (*str) {
		hash = hash * a + (*str++);
		a *= b;
	}

	return (hash & 0x7FFFFFFF);
}

/* JS Hash Function */
unsigned JS_hash(char *str)
{
	unsigned hash = 1315423911;

	while (*str) {
		hash ^= ((hash << 5) + (*str++) + (hash >> 2));
	}

	return (hash & 0x7FFFFFFF);
}

/* P. J. Weinberger Hash Function */
unsigned PJW_hash(char *str)
{
	unsigned BitsInUnignedInt = (unsigned)(sizeof(unsigned) * 8);
	unsigned ThreeQuarters     = (unsigned)((BitsInUnignedInt   * 3) / 4);
	unsigned OneEighth         = (unsigned)(BitsInUnignedInt / 8);

	unsigned HighBits          = (unsigned)(0xFFFFFFFF) << (BitsInUnignedInt - OneEighth);
	unsigned hash              = 0;
	unsigned test              = 0;

	while (*str) {
		hash = (hash << OneEighth) + (*str++);
		if ((test = hash & HighBits) != 0)	{
			hash = ((hash ^ (test >> ThreeQuarters)) & (~HighBits));
		}
	}

	return (hash & 0x7FFFFFFF);
}

/* ELF Hash Function */
unsigned ELF_hash(char *str)
{
	unsigned hash = 0;
	unsigned x    = 0;

	while (*str) {
		hash = (hash << 4) + (*str++);
		if ((x = hash & 0xF0000000L) != 0)	{
			hash ^= (x >> 24);
			hash &= ~x;
		}
	}

	return (hash & 0x7FFFFFFF);
}

/* BKDR Hash Function */
unsigned BKDR_hash(char *str)
{
	unsigned seed = 131; // 31 131 1313 13131 131313 etc..
	unsigned hash = 0;

	while (*str) {
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}

/* SDBM Hash Function */
unsigned SDBM_hash(char *str)
{
	unsigned hash = 0;

	while (*str) {
		hash = (*str++) + (hash << 6) + (hash << 16) - hash;
	}

	return (hash & 0x7FFFFFFF);
}

/* DJB Hash Function */
unsigned DJB_hash(char *str)
{
	unsigned hash = 5381;

	while (*str) {
		hash += (hash << 5) + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}

/* AP Hash Function */
unsigned AP_hash(char *str)
{
	unsigned hash = 0;
	int i;
	for (i=0; *str; i++) {
		if ((i & 1) == 0) {
			hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
		} else {
			hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
		}
	}

	return (hash & 0x7FFFFFFF);
}

/* CRC Hash Function */
unsigned CRC_hash(char *str)
{
	unsigned        nleft   = strlen(str);
	unsigned long long  sum     = 0;
	unsigned short int *w       = (unsigned short int *)str;
	unsigned short int  answer  = 0;

	/*
	* Our algorithm is simple, using a 32 bit accumulator (sum), we add
	* sequential 16 bit words to it, and at the end, fold back all the
	* carry bits from the top 16 bits into the lower 16 bits.
	*/
	while ( nleft > 1 ) {
		sum += *w++;
		nleft -= 2;
	}
	/*
	* mop up an odd byte, if necessary
	*/
	if ( 1 == nleft ) {
		*( unsigned char * )( &answer ) = *( unsigned char * )w ;
		sum += answer;
	}
	/*
	* add back carry outs from top 16 bits to low 16 bits
	* add hi 16 to low 16
	*/
	sum = ( sum >> 16 ) + ( sum & 0xFFFF );
	/* add carry */
	sum += ( sum >> 16 );
	/* truncate to 16 bits */
	answer = ~sum;

	return (answer & 0xFFFFFFFF);
}

struct bloom_t
{
	unsigned long size;			// filter size
	char filter[];
};

bloom* create_bloom(unsigned size)
{
	bloom bf;
	bloom* bf_ptr;
	size_t size_;

	size *= RATIO;
	bf.size = (size >> 3) + ((size % BYTE_LEN) ? 1 : 0);
	bf.size *= BYTE_LEN;

	size_ = sizeof(bf) + (bf.size >> 3);
	bf_ptr = (bloom*)malloc(size_);
	if (!bf_ptr) {
		return bf_ptr;
	}
	bf_ptr->size = bf.size;
	memset(bf_ptr->filter, 0, bf_ptr->size >> 3);

	return bf_ptr;
}

bloom* build_bloom(const char* buf, size_t size)
{			
	int ret;
	size_t offset;
	bloom* bf = NULL;
	const char* version_str = BLOOM_VERSION;

	if (!buf || !size)	{
		return bf;
	}

	offset = 0;
	ret = memcmp((void*)(version_str), (void*)(buf), strlen(version_str));
	offset += strlen(version_str);
	if (ret) {
		return bf;
	}

	size -= offset;
	bf = (bloom*)malloc(sizeof(*bf) + size);
	if (!bf) {
		return bf;
	}
	bf->size = size << 3;
	memcpy(bf->filter, buf + offset, size);

	return bf;
}

void destroy_bloom(bloom* bf)
{
	if (bf) {
		free(bf);
		bf = NULL;
	}
}

void bloom_add(const bloom* bf, const char* str)
{
	unsigned idx = 0;
	unsigned pos = 0;
	unsigned hash = 0;
	char* filter = (char*)(bf->filter);
	unsigned hash_num = sizeof(g_hash) / sizeof(g_hash[0]);
	unsigned long capability = bf->size;

	for (idx = 0; idx < hash_num; idx++) {
		hash = (g_hash[idx])((char*)(str));
		pos = hash % capability;
		SETBIT(filter, pos);
	}
}

int bloom_check(const bloom* bf, const char* str)
{
	unsigned idx = 0;
	unsigned pos = 0;
	unsigned hash = 0;
	unsigned flag = 0;
	char* filter = (char*)(bf->filter);
	unsigned hash_num = sizeof(g_hash) / sizeof(g_hash[0]);
	unsigned long capability = bf->size;

	if (!bf || !str) {
		return -1;
	}

	for (idx = 0; idx < hash_num; idx++) {
		hash = (g_hash[idx])((char*)(str));
		pos = hash % capability;
		flag = GETBIT(filter, pos);
		if (!flag) {
			return -2;
		}
	}

	return 0;
}

unsigned bloom_mem_size(const bloom* bf)
{
	return strlen(BLOOM_VERSION) + (bf->size >> 3);
}

int bloom_compare(const bloom* bf1, const bloom* bf2)
{
	int ret = -1;
	if (!bf1 || !bf2) {
		return ret;
	}

	if (bf1->size != bf2->size) {
		ret = bf1->size < bf2->size -1 ? -1 : 1;
		return ret;
	}

	ret = memcmp((void*)(bf1->filter), (void*)(bf2->filter), bf1->size >> 3);

	return ret;
}

int bloom_serialize(const bloom* bf, char* buf, size_t size)
{
	unsigned long bf_size;
	size_t offset;

	if (!bf || !buf || !size) {
		return -1;
	}

	bf_size = bloom_mem_size(bf);
	if (size < bf_size) {
		return -1;
	}

	offset = 0;
	memcpy(buf + offset, BLOOM_VERSION, strlen(BLOOM_VERSION));
	offset += strlen(BLOOM_VERSION);

	memcpy(buf + offset, bf->filter, (bf->size >> 3));

	return 0;
}
