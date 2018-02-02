

#include <fstream>
#include <string>
#include <iostream>
#include <random>

using namespace std;

#include "bloom.h"

static void num_2_str(unsigned char num[8], char num_str[17])
{
	int idx_i = 0;
	int idx_j = 0;
	static char *hex = "0123456789ABCDEF";

	for (idx_i = 0; idx_i < 8; idx_i++) {
		num_str[idx_j++] = hex[(0xf0 & num[idx_i]) >> 4];
		num_str[idx_j++] = hex[0x0f & num[idx_i]];
	}

	num_str[idx_j] = '\0';
}

int main(int argc, char** argv)
{
// 	if (argc < 3) {
// 		cout << "Fatal error: you should input a advertisement dict file and a advertisement md5 file." << endl;
// 		return -1;
// 	}

	// Open the advertisement dict file
	const char* ad_dict_file = "..\\tl.txt.bf";//argv[1];
	ifstream fin;
	fin.open(ad_dict_file, ios::binary);
	if (!fin) {
		cout << "Fatal error:" << argv[0] << " can not open file " << ad_dict_file << "!" << endl;
		return -1;
	}

	// create bloom filter
	fin.seekg(0, ios::end);
	size_t buf_size = fin.tellg();
	fin.seekg(0, ios::beg);
	if (!buf_size) {
		fin.close();
		cout << "Fatal error: the size of the advertisement md5 dict file " << ad_dict_file << "is " << buf_size << "." << endl;
		return -1;
	}
	char* buf = (char*)malloc(buf_size);
	if (!buf) {
		fin.close();
		cout << "Fatal error: malloc(" << buf_size << ") = nil." << endl;
		return -1;
	}
	fin.read(buf, buf_size);
	bloom* filter = build_bloom(buf, buf_size);
	if (!filter) {
		free(buf);
		fin.close();
		cout << "Fatal error: build_bloom(buf:" << buf << ", buf_size:" << buf_size << ") = nil." << endl;
		return -2;
	}
	free(buf);
	fin.close();																	  

	// Read the advertisement md5 list file
	// and check it by the bloom filter.
	cout << "checking md5 list" << endl;
	char* ad_md5_file = "..\\tl.txt";
	fin.open(ad_md5_file);
	if (!fin) {
		destroy_bloom(filter);
		cout << "Fatal error:" << argv[0] << " can not open file " << ad_md5_file << "!" << endl;
		return -2;
	}
	char line_str[1024];
	int ret = 0;
	while (fin.getline(line_str, sizeof(line_str))) {
		line_str[16] = '\0';
		
		ret = bloom_check(filter, line_str);
		if (ret) {
			cout << "can not find md5:" << line_str << "in the bloom filter dictionary!" << endl;
		}
	}
	fin.close();

	// construct random number
	cout << "generate random string and check it by bloom filter..." << endl;
	unsigned char num[8];
	unsigned char* ptr;
	char num_str[17];
	srand(0X12345678);
	ret = 0;
	unsigned idx = 0;
	unsigned false_num = 0;
	for (idx = 0; idx < 1500000; idx++) {
		ptr = num;
		*(unsigned*)(ptr) = rand();
		*(unsigned*)(ptr + sizeof(unsigned)) = rand();
		num_2_str(num, num_str);
		ret = bloom_check(filter, num_str);
		if (!ret) {
			false_num++;
			cout << "find md5:" << num_str << " in the bloom filter dictionary!" << endl;
		}
	}
	cout << "we generate almost " << idx << "random strings, and find " << false_num << " of them in bloom filter!" << endl;

	destroy_bloom(filter);

	return 0;
}

