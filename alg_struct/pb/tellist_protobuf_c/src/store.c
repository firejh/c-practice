
#include "pc_public.h"
#include "tellist.pc.h"
#include <stdlib.h>
#include <stdio.h>

#define	MIN_PHONE_NUM						13000000000UL
#define	MAX_PHONE_NUM						19000000000UL
#define	PHONE_NUM_TYPE						unsigned long long int

int gen_rand_tel_num(phone_contact_uin_tel** phone, int num);
int compare_tel_num(const void*, const void*);
int diff_arr(phone_contact_uin_tel** phone, int num);
void free_tel_num_arr(phone_contact_uin_tel** phone, int num);

int main(void) {
	int					ret;
	int					idx;
	int					num;
	char*				out = "out";
	FILE*				file;
	size_t				file_size;

	num = 13;
	phone_contact_uin_tel* arr[num];
	for (idx = 0; idx < 13; idx++)	{
		arr[idx] = (phone_contact_uin_tel*)malloc(sizeof(phone_contact_uin_tel));
		phone_contact_uin_tel_init(arr[idx]);
	}

	// gen tel num array
	ret = gen_rand_tel_num(arr, num);
	if (ret != 0)	{
		fprintf(stderr, "error! gen_rand_tel_num(arr = %p, num = %d) = %d\n",
				arr, num, ret);
		return -1;
	}

	for (idx = 0; idx < num; idx++)	{
		fprintf(stdout, "the %04dth telephone info{name = %4s, number = %13llu\n",
				idx, arr[idx]->name, arr[idx]->num);
	}
	fprintf(stdout, "after sort arr\n");
	// sort
	qsort((void*)arr, num, sizeof(arr[0]), compare_tel_num);
	for (idx = 0; idx < num; idx++)	{
		fprintf(stdout, "the %04dth telephone info{name = %4s, number = %13llu\n",
				idx, arr[idx]->name, arr[idx]->num);
	}

	// difference
	diff_arr(arr, num);

	// assign value
	PC_INIT(phone_contact_uin_tel_list, tel_list);
	tel_list.n_tel = num;
	tel_list.tel = arr;

	// serialize
	file = NULL;
	do {
		PC_PACK(phone_contact_uin_tel_list, &tel_list, buf, buf_size);
		if ((buf_size + 1) != sizeof(buf))	{
			fprintf(stderr, "fail to serialize tel_list. buf_size = %u, sizeof(buf) = %zu",
						buf_size, sizeof(buf));
			break;
		}

		file = fopen(out, "w");
		if (!file)	{
			break;
		}

		file_size = fwrite(buf, sizeof(char), buf_size, file);
		if (file_size != buf_size)	{
			fprintf(stderr, "fwrite(buf = %p, size = %zu, buf size = %u, file = %p) = %zu",
						buf, sizeof(char), buf_size, file, file_size);
			break;
		}
	} while(0);

	if (file)	{
		fclose(file);
	}
	free_tel_num_arr(arr, num);

	return 0;
}

int gen_rand_tel_num(phone_contact_uin_tel** phone, int num)	{
	int					idx;
	int					nrand;
	double				frand;
	size_t				size;
	unsigned int		seed = 2013082120;
	double				max_num = 4294967296.f;

	if (NULL == phone && num <= 0)	{
		return -1;
	}

	srand(seed);

	size = 16;
	for (idx = 0; idx < num; idx++)	{
		nrand = rand();
		frand = nrand;
		if (frand < 0)	{
			frand += 2147483646.f;
		}
		phone[idx]->name = malloc(size * sizeof(char));
		snprintf(phone[idx]->name, size * sizeof(char), "%d", idx);
		phone[idx]->has_num = 1;
		phone[idx]->num = (PHONE_NUM_TYPE)((frand / max_num) * (MAX_PHONE_NUM - MIN_PHONE_NUM) + MIN_PHONE_NUM);
	}

	return 0;
}

int compare_tel_num(const void* arg0, const void* arg1)	{
	int				ret;
	PHONE_NUM_TYPE	num0;
	PHONE_NUM_TYPE	num1;

	num0 = (*(phone_contact_uin_tel**)(arg0))->num;
	num1 = (*(phone_contact_uin_tel**)(arg1))->num;

	ret = 1;
	if (num0 < num1)	{
		ret = -1;
	} else if (num0 == num1)	{
		ret = 0;
	}

	return ret;
}

int diff_arr(phone_contact_uin_tel** arr, int num)	{
	int		idx_i;
	int		idx_j;

	if (NULL == arr && num <= 0)	{
		return -1;
	}

	for (idx_i = 0; idx_i < num - 1; idx_i++)	{
		for (idx_j = idx_i + 1; idx_j < num; idx_j++)	{
			arr[idx_j]->num -= arr[idx_i]->num;
		}
	}

	return 0;
}

void free_tel_num_arr(phone_contact_uin_tel** phone, int num)	{
	int idx;
	if (!phone || !num)	{
		return;
	}

	for (idx = 0; idx < num; idx++)	{
		free(phone[idx]->name);
		free(phone[idx]);
	}
}

