
#include "tellist.pb.h"
#include <fstream>
#include <iostream>
using namespace std;

void ListMsg(const ::ikurento::hi::hi_tel_num_list& tel_list) {
	int							idx;
	unsigned long long int		sum;
	unsigned long long int		tel_num;
	int							tel_list_size;

	sum = 0;
	tel_list_size = tel_list.tel_num_size();
	cout << "the size of telephone number list is " << tel_list_size << endl;
	for (idx = 0; idx < tel_list_size; idx++)	{
		sum += tel_list.tel_num(idx);
		tel_num = sum;
		cout << "the " << idx << "th telephone number is " << tel_num << endl;
	}
}

int main(int argc, char* argv[]) {
	::ikurento::hi::hi_tel_num_list tel_list;

	fstream input("./out", ios::in | ios::binary);
	if (!tel_list.ParseFromIstream(&input)) {
		cerr << "Failed to parse address book." << endl;
		return -1;
	}

	ListMsg(tel_list);
}
