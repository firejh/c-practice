
#include "shm_lru.h"

#include <string>
#include <iostream>

using namespace std;

#define SHM_KEY 0x20180131

/////////////////////////////
// test string
/////////////////////////////

void Output(const buf_t& buf)
{
    cout << (char*)(buf.data());
}

buf_t CvtString2Buf(const string& str)
{
    buf_t buf;
    buf.assign((unsigned char*)str.data(), (unsigned char*)str.data() + str.size());
    return buf;
}

string CvtBuf2String(const buf_t& buf)
{
    string str;
    str.assign((char*)buf.data(), (char*)buf.data() + buf.size());
    return str;
}

int Compare(const buf_t& key1, const buf_t& key2)
{
    string skey1;
    string skey2;
    skey1 = CvtBuf2String(key1);
    skey2 = CvtBuf2String(key2);

    return skey1.compare(skey2);
}

void testString()
{
    LruHashtable Cache;
    Cache.Init(5, SHM_KEY, 0x666, Compare);

    cout << "Display The LRU Cache...." << endl;
    // Cache.Display();
    Cache.DisplayByLRU(Output, Output);

    Cache.Set(CvtString2Buf("key1"), CvtString2Buf("value1"));
    Cache.Set(CvtString2Buf("key2"), CvtString2Buf("value2"));
    Cache.Set(CvtString2Buf("key3"), CvtString2Buf("value3"));
    Cache.Set(CvtString2Buf("key4"), CvtString2Buf("value4"));
    Cache.Set(CvtString2Buf("key5"), CvtString2Buf("value5"));
    Cache.Set(CvtString2Buf("key6"), CvtString2Buf("value6"));

    cout << "After insert 6 elements, display The LRU Cache...." << endl;
    // Cache.Display();
    Cache.DisplayByLRU(Output, Output);

    buf_t value;
    Cache.Get(CvtString2Buf("key4"), value);
    cout << "Now,Visit the LRU Cache with \"key4\"" << endl;
    cout << "The \"key4\" Value is : " << CvtBuf2String(value) << endl;
    cout << "The New LRU Cache is ... " << endl;
    // Cache.Display();
    Cache.DisplayByLRU(Output, Output);
}

int main()
{
    testString();

    return 0;
}