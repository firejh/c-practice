

#include <fstream>
#include <string>
#include <iostream>

using namespace std;

#include "bloom.h"

int main(int argc, char** argv)
{
// 	if (argc < 2) {
// 		cout << "Fatal error: you should input a advertisement md5 list file." << endl;
// 		return -1;
// 	}

	// Open the advertisement md5 list file
	const char* ad_md5_file = "..\\tl.txt";//argv[1];
	ifstream fin;
	fin.open(ad_md5_file);
	if (!fin) {
		cout << "Fatal error:" << argv[0] << " can not open file " << ad_md5_file << "!" << endl;
		return -1;
	}

	// create bloom filter
	unsigned int idx = 0;
	char line_str[1024];
	while (fin.getline(line_str, sizeof(line_str))) {
		idx++;
	}
	cout << "file lines number:" << idx << endl;
	fin.close();
	unsigned item_num = idx;
	bloom* filter = create_bloom(item_num);
	if (!filter) {
		cout << "Fatal error: create_bloom(" << item_num << ") = nil" << endl;
		return -1;
	}																	  

	// Read the advertisement md5 list file
	// and Inserts md5 into the bloom filter.
	cout << "building bloom filter" << endl;
	bool flag = true;
	fin.open(ad_md5_file);
	while (fin.getline(line_str, sizeof(line_str))) {
		line_str[16] = '\0';
		//cout << line_str << endl;
		bloom_add(filter, line_str);
	}
	fin.close();

	// Writes the bloom filter into a file ".bf".
	string file_name(ad_md5_file);
	file_name += ".bf";
	ofstream fout;
	fout.open(file_name.data(), ios::binary);
	unsigned buf_size = bloom_mem_size(filter); 
	char* buf = (char*)malloc(buf_size);
	bloom_serialize(filter, buf, buf_size);
	fout.write(buf, buf_size);
	fout.close();
	free(buf);
	buf = NULL;

	cout << "convert over! the output file is " << file_name << "." << endl;

	// and check it by the bloom filter.
// 	cout << "checking md5 list" << endl;
// 	fin.open(ad_md5_file);
// 	if (!fin) {
// 		destroy_bloom(filter);
// 		cout << "Fatal error:" << argv[0] << " can not open file " << ad_md5_file << "!" << endl;
// 		return -2;
// 	}
// 	int ret = 0;
// 	while (fin.getline(line_str, sizeof(line_str))) {
// 		line_str[16] = '\0';
// 
// 		ret = bloom_check(filter, line_str);
// 		if (ret == 0) {
// 			cout << "can not find md5:" << line_str << "in the bloom filter dictionary!" << endl;
// 		}
// 	}
// 	fin.close();

	destroy_bloom(filter);
	filter = NULL;

	return 0;

}

