/**
 * FILE		: lisk_hashtable.c
 * DESC		: standard data struct hashtable like dict of redis
 * AUTHOR	: v0.3.0 written by Alex Stocks
 * DATE		: on Nov 28, 2012
 * MOD		: modified by * on June 2, 2012
 **/

#include <lisk_math.h>
#include <lisk_hash.h>
#include <lisk_hashtable.h>
#include <lisk_log.h>

#define	MAX_AVG_BUCKET_SIZE		5

typedef	key_t				ht_key_t;
typedef	key_p				ht_key_p;
typedef	key_a				ht_key_a;
typedef	value_t				ht_value_t;
typedef	value_p				ht_value_p;
typedef	value_a				ht_value_a;

#pragma pack(1)

typedef struct ht_elem_tag	{
	ht_key_t				key;
	ht_value_t				value;
	struct ht_elem_tag*		next;
} ht_elem_t, *ht_elem_p, ht_elem_a[1];

typedef struct ht_bucket_tag {
	ht_elem_p				head;
} ht_bucket_t, *ht_bucket_p, ht_bucket_a[1];

typedef struct table_tag {
	u8						size;
	u8						number;
	struct table_tag*		next;
	ht_bucket_t				bucket[0];
} table_t, *table_p;

typedef struct hashtable_tag {
	compare			cmp;
	table_p			table;
} hashtable_t, *hashtable_p, hashtable_a[1];

#pragma pack()

#define	ht_next(elem)				((elem)->next)

#define	ht_set_next(elem, next_ptr)	(elem)->next = (next_ptr)

n4 hashtable_init(vpp ht, u8 size, compare cmp)
{
	table_p		table;
	u8			length;
	hashtable_p	hashtable;

	if (IS_NL(ht) || IS_ZR(size) || IS_NL(cmp)) {
		RET_INT(-1, "@hashtable = %p, size = %llu, cmp = %p", ht, size, cmp);
	}

	*ht = nil;

	size = next_prime(size);
	length = sizeof(table_t) + size * sizeof(ht_bucket_t);
	ALLOC(table_p, table, length);
	if (IS_NL(table)) {
		RET_INT(-2, "ALLOC(table_p = %p, size = %llu)", table, length);
	}
	SET_ZERO(table, length);
	table->size = size;

	length = sizeof(hashtable_t);
	ALLOC(hashtable_p, hashtable, length);
	if (IS_NL(hashtable)) {
		DEALLOC(table);
		RET_INT(-2, "ALLOC(hashtable_t = %p, size = %llu)", hashtable, length);
	}
	hashtable->table	= table;
	hashtable->cmp		= cmp;

	*ht = hashtable;

	RET_INT(0, nil_str);
}

#define	MAX_BUCKET_SIZE		5

n4 hashtable_expand(vp ht, u8 size)
{
	hashtable_p	hashtable;
	table_p		table;
	u8			length;

	if (IS_NL(ht)) {
		RET_INT(-1, "@hashtable = %p", ht);
	}

	hashtable = (hashtable_p)(ht);
	if (IS_ZR(size)) {
		size = hashtable->table->size * 4;
		if (IS_LT(size, hashtable->table->size)) {
			size = hashtable->table->size * 2;
			if (IS_LT(size, hashtable->table->size)) {
				size = hashtable->table->size + hashtable->table->size / 2;
				if (IS_LT(size, hashtable->table->size)) {
					RET_INT(0, nil_str);
				}
			}
		}
	}
	size = next_prime(size);
	if (IS_LT(size, hashtable->table->size)) {
		RET_INT(0, nil_str);
	}

	length = sizeof(table_t) + size * sizeof(ht_bucket_t);
	ALLOC(table_p, table, length);
	if (IS_NL(table)) {
		RET_INT(-2, "ALLOC(table_p = %p, size = %llu)", table, length);
	}
	SET_ZERO(table, length);
	table->size			= size;
	table->next			= hashtable->table;
	hashtable->table	= table;

	RET_INT(0, nil_str);
}

n4 hashtable_release_table(table_p table)
{
	ht_bucket_p	bucket;
	ht_elem_p	elem;
	ht_elem_p	next;
	u8			idx;

	if (IS_NL(table)) {
		RET_INT(-1, "@table = %p", table);
	}

	for (idx = 0; idx < table->size; idx++) {
		bucket	= table->bucket + idx;
		elem	= bucket->head;
		while (IS_NNL(elem)) {
			next = ht_next(elem);
			DEALLOC(elem);
			elem = next;
		}
	}
	DEALLOC(table);

	RET_INT(0, nil_str);
}

n4 hashtable_free(vpp ht)
{
	hashtable_p	hashtable;
	table_p		table;
	table_p		next;

	hashtable = nil;
	if (IS_NL(ht) || IS_NL(hashtable = (hashtable_p)(*ht))) {
		RET_INT(-1, "@hashtable = %p, *@hashtable = %p", ht, hashtable);
	}

	table = hashtable->table;
	while (IS_NNL(table)) {
		next = table->next;
		hashtable_release_table(table);
		table = next;
	}

	DEALLOC(*ht);

	RET_INT(0, nil_str);
}

n4 hashtable_search(vp ht, ht_key_p key, ht_elem_p* elem)
{
	n4				ret;
	u8				elem_pos;
	hashtable_p		hashtable;
	table_p			table;
	ht_bucket_p		bucket;
	ht_elem_p		elem_itr;
	table_p			target_table;
	table_p			target_table_pre;
	ht_bucket_p		target_bucket;
	ht_elem_p		target_elem;

	if (IS_NL(hashtable = (hashtable_p)(ht)) || IS_NL(key) || IS_NL(elem)) {
		RET_INT(-1, "@hashtable = %p, @key = %p, @elem = %p", hashtable, key, elem);
	}

	if (IS_NL(key->data) || IS_ZR(key->size)) {
		RET_INT(-1, "@key->data = %p, @key-size = %hu", key->data, key->size);
	}

	target_elem			= nil;
	elem_pos			= bkdr64(key->data, key->size);
	table				= hashtable->table;
	target_table_pre	= table;
	while (IS_NNL(table)) {
		bucket		= table->bucket + elem_pos % table->size;
		elem_itr	= bucket->head;
		while (IS_NNL(elem_itr)) {
			ret = hashtable->cmp(&(elem_itr->key), key);
			//IS_EQ(key->size, elem_itr->key.size) &&
			if (IS_ZR(ret)) {
				target_table	= table;
				target_bucket	= bucket;
				target_elem		= elem_itr;
				break;
			}
			elem_itr = ht_next(elem_itr);
		}
		if (IS_NNL(target_elem)) {
			break;
		}
		target_table_pre	= table;
		table				= table->next;
	}

	if (IS_NL(target_elem)) {
		RET_INT(-2, nil_str);
	}

	if (IS_NEQ(target_table, hashtable->table)) {
		//del a old bucket @target_bucket
		elem_itr = target_bucket->head;
		while (IS_NNL(elem_itr)) {
			//del a element @target_elem in the old table
			target_bucket->head = ht_next(elem_itr);
			ht_set_next(elem_itr, nil);
			target_table->number--;

			//add that element @target_elem to the first table
			elem_pos = bkdr64(elem_itr->key.data, elem_itr->key.size);
			bucket = hashtable->table->bucket + elem_pos % hashtable->table->size;
			ht_set_next(elem_itr, bucket->head);
			bucket->head = elem_itr;
			hashtable->table->number++;

			elem_itr = target_bucket->head;
		}
		if (IS_ZR(target_table->number)) {
			table = hashtable->table;
			//while (IS_NNL(table->next)) {
			//	if (IS_EQ(table->next, target_table)) {
			//		table->next = target_table->next;
			//		hashtable_release_table(target_table);
			//		target_table = nil;
			//		break;
			//	}
			//	table = table->next;
			//}
			target_table_pre->next = target_table->next;
			hashtable_release_table(target_table);
		}
	}
	*elem = target_elem;

	RET_PTR(0, nil_str);
}

n4 hashtable_find(vp ht, key_p ht_key, value_p* ht_value)
{
	ht_key_p	key;
	ht_elem_p	elem;
	n4			ret;

	if (IS_NL(key = (ht_key_p)(ht_key)) || IS_NL(ht_value)) {
		RET_INT(-1, "@key = %p, @value = %p", key, ht_value);
	}

	ret = hashtable_search(ht, key, &elem);
	if (IS_ZR(ret) && IS_NNL(elem)) {
		*ht_value = (value_p)(&(elem->value));
	} else {
		ret = -2;
	}

	RET_INT(ret, nil_str);
}

n4 hashtable_insert(vp ht, key_p ht_key, value_p ht_value)
{
	hashtable_p	hashtable;
	ht_key_p	key;
	ht_value_p	value;
	ht_elem_p	ht_elem_ptr;
	n4			ret;
	ht_elem_p	elem;
	u8			elem_pos;
	ht_bucket_p	bucket;

	hashtable	= (hashtable_p)(ht);
	key			= (ht_key_p)(ht_key);
	value		= (ht_value_p)(ht_value);
	if (IS_NL(hashtable) || IS_NL(key) || IS_NL(value)) {
		RET_INT(-1, "@hashtable = %p, @key = %p, @value = %p", hashtable, key, value);
	}

	if (IS_NL(key->data) || IS_ZR(key->size) || IS_ZR(hashtable->table->size)) {
		RET_INT(-1, "@key->data = %p, @key->size = %hu, @hashtable->table->size %llu",
				key->data, key->size, hashtable->table->size);
	}

	if (IS_ZR(value->size) || IS_NL(value->data)) {
		RET_INT(-1, "@value->size = %u, @value->data = %p", value->size, value->data);
	}

	//expand table
	if (IS_GT(hashtable->table->number / hashtable->table->size, MAX_BUCKET_SIZE)) {
		hashtable_expand(hashtable, 0);
	}

	//find elem
	ret = hashtable_search(hashtable, key, &ht_elem_ptr);
	if (IS_ZR(ret)) {
		RET_INT(-3, nil_str);
	}

	//elem = ht_elem_t + ht_key_p->data(key.size) + ht_value_p->data(value.size)
	ALLOC(ht_elem_p, elem, sizeof(ht_elem_t) + key->size + value->size);
	if (IS_NL(elem)) {
		RET_INT(-2, "ALLOC(ht_elem_p = %p, size = %lu) = nil",
					elem, (long)(sizeof(ht_elem_t) + key->size + value->size));
	}

	elem->key.size = key->size;
	elem->key.data = (n1*)elem + sizeof(ht_elem_t);
	CPY(elem->key.data, key->data, key->size);

	elem->value.size = value->size;
	elem->value.data = (n1*)(elem->key.data) + elem->key.size;
	CPY(elem->value.data, value->data, value->size);

	elem->next = nil;

	elem_pos = bkdr64(key->data, key->size);
	bucket = hashtable->table->bucket + elem_pos % hashtable->table->size;
	ht_set_next(elem, bucket->head);
	bucket->head = elem;
	hashtable->table->number++;

	RET_INT(0, nil_str);
}

n4 hashtable_erase(vp ht, key_p ht_key)
{
	n4			ret;
	u8			elem_pos;
	ht_key_p	key;
	ht_elem_p	value;
	ht_elem_p	pre;
	ht_elem_p	elem;
	ht_bucket_p	bucket;
	hashtable_p	hashtable;

	hashtable = (hashtable_p)(ht);
	key = (ht_key_p)(ht_key);
	if (IS_NL(hashtable) || IS_NL(key)) {
		RET_INT(-1, "@hashtable = %p, @key = %p", ht, key);
	}

	if (IS_NL(key->data) || IS_ZR(key->size)) {
		RET_INT(-1, "@key->data = %p, @key->size = %hu", key->data, key->size);
	}

	//find elem
	ret = hashtable_search(hashtable, key, &value);
	if (IS_NZR(ret)) {
		RET_INT(-2, nil_str);
	}

	pre			= nil;
	elem_pos	= bkdr64(key->data, key->size);
	bucket		= hashtable->table->bucket + elem_pos % hashtable->table->size;
	elem		= bucket->head;
	while (IS_NNL(elem)) {
		if (IS_EQ(elem, value)) {
			if (IS_EQ(bucket->head, elem)) {
				bucket->head = elem->next;
			} else {
				pre->next = elem->next;
			}
			DEALLOC(elem);
			hashtable->table->number--;
			break;
		}
		pre = elem;
		elem = ht_next(elem);
	}

	RET_INT(0, nil_str);
}

n4 hashtable_update(vp ht, key_p ht_key, value_p value)
{
	n4			ret;
	u4			size;
	u8			elem_pos;
	ht_key_p	key;
	ht_elem_p	elem_itr;
	ht_bucket_p	bucket;
	table_p		table;
	ht_elem_p	new_elem;
	ht_elem_p	ht_elem_ptr;
	hashtable_p	hashtable;

	if (IS_NL(hashtable = ht) || IS_NL(key = (ht_key_p)(ht_key)) || IS_NL(value)) {
		RET_INT(-1, "@hashtable = %p, @key = %p, @value = %p", ht, ht_key, value);
	}

	if (IS_NL(key->data) || IS_ZR(key->size)) {
		RET_INT(-1, "@key->data = %p, @key->size = %hu", key->data, key->size);
	}

	if (IS_ZR(value->size) || IS_NL(value->data)) {
		RET_INT(-1, "@value->size = %u, @value->data = %p", value->size, value->data);
	}

	//find elem
	ret = hashtable_search(hashtable, key, &ht_elem_ptr);
	if (IS_NZR(ret)) {
		RET_INT(-2, nil_str);
	}

	//find bucket
	table		= hashtable->table;
	elem_pos	= bkdr64(key->data, key->size);
	bucket		= table->bucket + elem_pos % table->size;
	elem_itr	= bucket->head;
	if (IS_EQ(elem_itr, ht_elem_ptr))	{
		elem_itr = nil;
	} else {
		while (IS_NNL(elem_itr)) {
			if (IS_EQ(elem_itr->next, ht_elem_ptr)) {
				break;
			}
			elem_itr = ht_next(elem_itr);
		}
		if (IS_NL(elem_itr))	{
			RET_INT(-2, nil_str);
		}
	}

	size = sizeof(ht_elem_t) + key->size + value->size;
	if (IS_LT(value->size, ht_elem_ptr->value.size))	{
		ht_elem_ptr->value.size = value->size;
		CPY(ht_elem_ptr->value.data, value->data, ht_elem_ptr->value.size);
	} else {
		REALLOC(ht_elem_p, ht_elem_ptr, size);
		if (IS_NL(ht_elem_ptr)) {
			ALLOC(ht_elem_p, new_elem, size);
			if (IS_NL(new_elem))	{
				RET_INT(-2, "ALLOC(new_elem = %p, size = %u) = nil", new_elem, size);
			}

			//CPY(new_elem, ht_elem_ptr, sizeof(ht_elem_t)
			//+ ht_elem_ptr->key.size + ht_elem_ptr->value.size);
			new_elem->key.size = ht_elem_ptr->key.size;
			CPY(new_elem->key.data, ht_elem_ptr->key.data, ht_elem_ptr->key.size);
			new_elem->next = ht_elem_ptr->next;

			DEALLOC(ht_elem_ptr);
			ht_elem_ptr = new_elem;
		}

		//do not change key.size && key.data content && next
		//ht_elem_ptr->key.size = key->size;
		ht_elem_ptr->key.data = (n1*)ht_elem_ptr + sizeof(ht_elem_t);
		//CPY(elem->key.data, key->data, ht_elem_ptr->key.size);

		ht_elem_ptr->value.size = value->size;
		ht_elem_ptr->value.data = (n1*)(ht_elem_ptr->key.data) + ht_elem_ptr->key.size;
		CPY(ht_elem_ptr->value.data, value->data, ht_elem_ptr->value.size);
	}

	//bucket->head == old ht_elem_ptr
	if (IS_NNL(elem_itr))	{
		elem_itr->next = ht_elem_ptr;
	} else {
		bucket->head = ht_elem_ptr;
	}

	RET_INT(0, nil_str);
}

n4 hashtable_size(vp ht, u4* size)
{
	hashtable_p	hashtable;
	u4			ht_size;
	table_p		table;
	table_p		next;

	if (IS_NL(hashtable = (hashtable_p)(ht)) || IS_NL(size)) {
		RET_INT(-1, "@hashtable = %p, @size = %p", hashtable, size);
	}

	ht_size	= 0;
	table	= hashtable->table;
	while (IS_NNL(table)) {
		next	= table->next;
		ht_size	+= table->number;
		table	= next;
	}

	*size = ht_size;

	RET_INT(0, nil_str);
}

n4 hashtable_table_size(table_p table, u4* size)
{
	u4			table_size;
	ht_bucket_p	bucket;
	ht_elem_p	elem;
	ht_elem_p	next;
	u8			idx;

	if (IS_NL(table) || IS_NL(size)) {
		RET_INT(-1, "@table = %p, @size = %p", table,size);
	}

	*size = 0;
	table_size = sizeof(*table);
	for (idx = 0; idx < table->size; idx++) {
		bucket	= table->bucket + idx;
		elem	= bucket->head;
		table_size += sizeof(*bucket);
		while (IS_NNL(elem)) {
			next = ht_next(elem);
			table_size += sizeof(ht_elem_t) + elem->key.size + elem->value.size;
			elem = next;
		}
	}
	*size = table_size;

	RET_INT(0, nil_str);
}

n4 hashtable_memory_size(vp ht, u4* size)
{
	hashtable_p		hashtable;
	table_p			table;
	table_p			next;
	u4				ht_size;
	u4				table_size;

	if (IS_NL(hashtable = (hashtable_p)(ht)) || IS_NL(size)) {
		RET_INT(-1, "@hashtable = %p, @size = %p", hashtable, size);
	}

	ht_size		= sizeof(*hashtable);
	table		= hashtable->table;
	while (IS_NNL(table)) {
		next = table->next;
		table_size = 0;
		hashtable_table_size(table, &table_size);
		ht_size += table_size;
		table = next;
	}
	*size = ht_size;

	RET_INT(0, nil_str);
}
 