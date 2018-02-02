#ifndef __SHM_LRU_H__
#define __SHM_LRU_H__

#include <pthread.h>

#include <vector>
#include <list>
#include <map>

#include "structs.h"
#include "shm_adapter.h"

class LruHashtable;

typedef std::vector<unsigned char> buf_t;
typedef void (*DisplayBuf)(const buf_t&);
typedef int (*CompareBuf)(const buf_t&, const buf_t&);

class LRU
{
public:
    LRU(void);
    ~LRU(void);

    bool InitLruMem(LruHashtable* tbl);

    int GetRemoveKey(buf_t& key);
    buf_t RemoveKey(void);
    void VisitKey(const buf_t& key);

    buf_t& GetKey(int index);

private:
    std::list<buf_t> lruMem;
    std::map<buf_t, std::list<buf_t>::iterator> keyToIterator;
};

class LruHashtable
{
public:
    LruHashtable(void);
    ~LruHashtable(void);

public:
    int Init(int maxSlotNum, key_t shmkey, mode_t mode, CompareBuf cmp);
    int Get(const buf_t& key, buf_t& val);
    int Set(const buf_t& key, const buf_t& val);
    bool Exist(const buf_t& key);
    int GetNext(buf_t& key, buf_t &val, int& idx);
    int GetCount(int& max_slots, int& used_slots);
    int Remove(const buf_t& key);
    int Clear(void);

    // void Display(void, );
    void DisplayByLRU(DisplayBuf displayKey, DisplayBuf displayValue);

private:
    CompareBuf compare;
    qhasharr_t *hashtable;
    LRU *lru;
    pthread_mutex_t mutex;
    shm_adapter *shm;
};

#endif

