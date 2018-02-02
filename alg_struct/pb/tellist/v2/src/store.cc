#include "tellist.pb.h"
#include <stdlib.h>
#include <fstream>
#include <iostream>
using namespace std;

#define	PHONE_NUM_TYPE	unsigned long long int
#define	nil				NULL
#define	MIN_PHONE_NUM	13000000000UL
#define	MAX_PHONE_NUM	19000000000UL

int gen_rand_tel_num(PHONE_NUM_TYPE* phone, int num);
int compare_tel_num(const void*, const void*);
int diff_arr(PHONE_NUM_TYPE* phone, int num);

int main(void) {
	int								ret;
	int								idx;
	int								max_idx;
	int								num;
	PHONE_NUM_TYPE					arr[1024];
	::ikurento::hi::hi_tel_num_list	tel_list;

	// gen tel num array
	num = 13;
	ret = gen_rand_tel_num(arr, num);
	if (ret != 0)	{
		cerr << "error! gen_rand_tel_num(arr, " << num << ") = " << ret << endl;
		return -1;
	}

	// sort
	qsort(arr, num, sizeof(arr[0]), compare_tel_num);
	for (idx = 0; idx < num; idx++)	{
		cout << "the " << idx << "th telephone number is " << arr[idx] << endl;
	}

	// difference
	diff_arr(arr, num);

	// assign value
	max_idx = num;
	for (idx = 0; idx < max_idx; idx++)	{
		tel_list.add_tel_num(arr[idx]);
	}
	//tel_list.set_tel_num(0, num);

	// serialize
	fstream output("./out", ios::out | ios::trunc | ios::binary);

	if (!tel_list.SerializeToOstream(&output)) {
		cerr << "Failed to write msg." << endl;
		return -1;
	}

	return 0;
}

int gen_rand_tel_num(PHONE_NUM_TYPE* phone, int num)	{
	int					idx;
	int					nrand;
	double				frand;
	unsigned int		seed = 2013082120;
	double				max_num = 4294967296.f;

	if (nil == phone && num <= 0)	{
		return -1;
	}

	srand(seed);

	for (idx = 0; idx < num; idx++)	{
		nrand = rand();
		frand = nrand;
		if (frand < 0)	{
			frand += 2147483646.f;
		}
		phone[idx] = (PHONE_NUM_TYPE)((frand / max_num) * (MAX_PHONE_NUM - MIN_PHONE_NUM) + MIN_PHONE_NUM);
	}

	return 0;
}

int compare_tel_num(const void* arg0, const void* arg1)	{
	int				ret;
	PHONE_NUM_TYPE	num0;
	PHONE_NUM_TYPE	num1;

	num0 = *(PHONE_NUM_TYPE*)(arg0);
	num1 = *(PHONE_NUM_TYPE*)(arg1);

	ret = 1;
	if (num0 < num1)	{
		ret = -1;
	} else if (num0 == num1)	{
		ret = 0;
	}

	return ret;
}

int diff_arr(PHONE_NUM_TYPE* arr, int num)	{
	int		idx_i;
	int		idx_j;

	if (nil == arr && num <= 0)	{
		return -1;
	}

	for (idx_i = 0; idx_i < num - 1; idx_i++)	{
		for (idx_j = idx_i + 1; idx_j < num; idx_j++)	{
			arr[idx_j] -= arr[idx_i];
		}
	}

	return 0;
}

