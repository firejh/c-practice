

#include <fstream>
#include <string>
#include <iostream>

using namespace std;

#include "bloom.h"

int main(int argc, char** argv)
{
 	if (argc < 2) {
 		cout << "Fatal error: you should input a advertisement "
			<< "md5 list file and its bloom database file name!." << endl;
 		return -1;
 	}

	// Open the advertisement md5 list file
	const char* ad_md5_file = argv[1];
	ifstream fin;
	fin.open(ad_md5_file);
	if (!fin) {
		cout << "Fatal error:" << argv[0]
			<< " can not open file " << ad_md5_file << "!" << endl;
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
	fin.open(ad_md5_file);
	while (fin.getline(line_str, sizeof(line_str))) {
		line_str[16] = '\0';
		//cout << line_str << endl;
		bloom_add(filter, line_str);
	}
	fin.close();

	// Writes the bloom filter into a file ".bf".
	const char* file_name = argv[2];
	int ret = 0;
	unsigned buf_size = 0;
	char* buf = NULL;
	ofstream fout;
	fout.open(file_name, ios::binary);
	if (!fout) {
		ret = -2;
		cout << "Fatal error: can not open file " << file_name << "!" << endl;
		goto ERROR1;
	}
	buf_size = bloom_mem_size(filter);
	buf = new char[buf_size];
	if (!buf) {
		ret = -2;
		cout << "Fatal error: malloc(" << buf_size << ") = nil." << endl;
		goto ERROR2;
	}
	bloom_serialize(filter, buf, buf_size);
	fout.write(buf, buf_size);
	delete []buf;
	buf = NULL;

	cout << "convert over! the output file is " << file_name << "." << endl;

	ret = 0;

ERROR2:
	fout.close();
ERROR1:
	destroy_bloom(filter);
	filter = NULL;

	return ret;
}

