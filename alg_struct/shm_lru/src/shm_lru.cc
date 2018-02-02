#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/file.h>
#include <time.h>

#include <vector>
#include <string>
#include <map>
#include <list>
#include <iostream>
#include <algorithm>

#include "tx_time.h"
#include "shm_lru.h"

using namespace std;

///////////////////////////////////////
//  LRU
///////////////////////////////////////

LRU::LRU(void)
{
    lruMem.clear();
    keyToIterator.clear();
}

LRU::~LRU(void)
{
    lruMem.clear();
    keyToIterator.clear();
}

struct KeyTime {
    time_buf_t time;
    buf_t key;
};

bool CompareKeyTime(const KeyTime& a, const KeyTime& b)
{
    return a.time > b.time;
}

#include <iostream>

bool LRU::InitLruMem(LruHashtable *table)
{
    int max_slots = 0, used_slots = 0;
    table->GetCount(max_slots, used_slots);

    buf_t key, val;
    KeyTime keyTime;
    vector<KeyTime> keyList;
    for (int idx = 0; idx < max_slots;) {
        int ret = table->GetNext(key, val, idx);
        if (ret == OK) {
            // VisitKey(key);
            keyTime.key = key;
            memcpy(&keyTime.time, (time_buf_t*)&val[val.size() - sizeof(time_buf_t) - 1], sizeof(time_buf_t));
            cout << "key:" << ((char*)&key[0]) << ", time:" << keyTime.time << endl;
            keyList.push_back(keyTime);
        } else if (ERR_TBL_END == ret){
        } else{
            // LOG_ERR_KEY_INFO(key, "Failed to get next item in shmthis");
            return false;
        }
    }

    sort(keyList.begin(), keyList.end(), CompareKeyTime);
    vector<KeyTime>::iterator it;
    for (it = keyList.begin(); it != keyList.end(); it++) {
        this->VisitKey(it->key);
    }

    return true;
}

int LRU::GetRemoveKey(buf_t& key)
{
    if (lruMem.empty()) {
        // LOG_ERR("Memory is empty nothing to remove. Maybe it's too small");
        return ERR_NOT_FOUND;
    }
    key = lruMem.back();

    return OK;
}

buf_t LRU::RemoveKey(void)
{
    buf_t key = lruMem.back();
    lruMem.pop_back();
    if (keyToIterator.find(key) != keyToIterator.end()) {
        keyToIterator.erase(key);
    }

    return key;
}

void LRU::VisitKey(const buf_t& key)
{
    if (keyToIterator.find(key) == keyToIterator.end()) {
        lruMem.push_front(key);
        keyToIterator[key] = lruMem.begin();
    } else {
        list<buf_t>::iterator it = keyToIterator[key];
        lruMem.erase(it);
        lruMem.push_front(key);
        keyToIterator[key] = lruMem.begin();
    }

    return;
}

buf_t& LRU::GetKey(int index)
{
    std::list<buf_t>::iterator it = this->lruMem.begin();
    std::advance(it, index);

    return *it;
}


///////////////////////////////////////
//  Hashtable
///////////////////////////////////////

LruHashtable::LruHashtable()
{
    this->hashtable = NULL;
    this->lru = NULL;
    this->shm = NULL;
    // share memory set and remove lock
    pthread_mutex_init(&(this->mutex), NULL);
}

LruHashtable::~LruHashtable()
{
    // this->Clear(); // 退出的时候不清空数据
    pthread_mutex_destroy(&(this->mutex));
    if (this->shm) {
        delete this->shm;
    }
}

int init_hash_tbl(qhasharr_t *&tbl, key_t shmkey, mode_t mode, int flags)
{
    int shmid = -1;

    shmid = shmget(shmkey, 0, mode);
    if (-1 == shmid) return ERR_SHMGET;

    tbl = (qhasharr_t*)shmat(shmid, NULL, flags);
    if ((void*)-1 == (void*)tbl)
    {
        tbl = NULL;
        return ERR_SHMAT;
    }
    return OK;
}

int LruHashtable::Init(int maxSlotNum, key_t shmkey, mode_t mode, CompareBuf cmp)
{
    if (!cmp) {
        return ERR_PARAM;
    }

    int shmid = -1;
    size_t memsize = 0;
    void* shmptr = NULL;

    memsize = qhasharr_calculate_memsize(maxSlotNum);
    if (this->shm) {
        delete this->shm;
    }
    this->shm = new shm_adapter((uint64_t)(shmkey), (uint64_t)memsize, true, false, true, false, false);
    shmptr = this->shm->get_shm();
    if (shmptr == NULL) {
        delete this->shm;
        return ERR_SHMGET;
    }
    this->hashtable = (qhasharr_t *)(shmptr);
    // 如果共享内存已经实现存在，则不能清空相关数据
    if (NULL == this->hashtable || this->hashtable->maxslots != maxSlotNum) {
        this->hashtable = qhasharr(shmptr, memsize);
    }
    if (NULL == this->hashtable) {
        // LOG_FATAL_ERR("Failed to init shm of shmid:%d errno:%d", shmid, errno);
        return ERR_SHMINIT;
    }

    LRU *lru = new LRU();
    bool initRet = lru->InitLruMem(this);
    if (!initRet) {
        // LOG_ERR("Init LRU memory failed");
        delete lru;
        return ERR_MEM;
    }
    this->lru = lru;
    this->compare = cmp;

    return OK;
}

int LruHashtable::Set(const buf_t& key, const buf_t& val)
{
    if (key.empty()) {
        return ERR_PARAM;
    }

    time_buf_t curTime = TIME_USEC;
    buf_t value(val);
    value.insert(value.end(), (time_buf_t*)&curTime, (time_buf_t*)&curTime + sizeof(curTime));
    cout << "insert key:" << (char*)&(key[0]) << ", time:" << curTime << endl;

    buf_t val_in_mem;
    int retv = OK;
    retv = this->Get(key, val_in_mem);
    if (OK == retv && 0 == this->compare(val, val_in_mem)) {
        return ERR_SAME_VALUE;
    }

    pthread_mutex_lock(&this->mutex);
    bool ret = qhasharr_put(this->hashtable, (const char*)(key.data()), key.size(), (const char*)(value.data()), value.size());
    pthread_mutex_unlock(&this->mutex);
    while (!ret && errno == ENOBUFS) {
        buf_t RemovedKey;
        pthread_mutex_lock(&this->mutex);
        int retv = this->lru->GetRemoveKey(RemovedKey);
        pthread_mutex_unlock(&this->mutex);
        if (retv != OK) {
            // LOG_ERR("remove key from shared memory failed");
            break;
        }
        errno = 0;
        retv = this->Remove(RemovedKey);
        if (retv == OK) {
            this->lru->RemoveKey();
        } else {
            // LOG_ERR("remove key from shared memory failed");
            break;
        }
        pthread_mutex_lock(&this->mutex);
        ret = qhasharr_put(this->hashtable, (const char*)(key.data()), key.size(), (const char*)(value.data()), value.size());
        pthread_mutex_unlock(&this->mutex);
    }
    if (ret) {
        pthread_mutex_lock(&this->mutex);
        this->lru->VisitKey(key);
        pthread_mutex_unlock(&this->mutex);
    }

    return ret ? OK : ERR_TBL_SET;
}

bool LruHashtable::Exist(const buf_t& key)
{
    if (key.empty()) {
        return false;
    }

    bool ret = false;
    pthread_mutex_lock(&this->mutex);
    ret = qhasharr_exist(this->hashtable, (const char*)(key.data()), key.size());
    pthread_mutex_unlock(&this->mutex);

    return ret;
}

int LruHashtable::Get(const buf_t& key, buf_t& val)
{
    if (key.empty()) {
        return ERR_PARAM;
    }

    char *val_tmp = NULL;
    size_t val_tmp_len = 0;

    pthread_mutex_lock(&this->mutex);
    val_tmp = (char*)qhasharr_get(this->hashtable, (const char*)(key.data()), key.size(), &val_tmp_len);
    pthread_mutex_unlock(&this->mutex);
    if (NULL == val_tmp) {
        return ERR_NOT_FOUND;
    }

    val.assign((unsigned char*)val_tmp, (unsigned char*)val_tmp + val_tmp_len - sizeof(time_buf_t));
    free(val_tmp);
    val_tmp = NULL;

    return OK;
}

int LruHashtable::GetCount(int &max_slots, int &used_slots)
{
    return qhasharr_size(this->hashtable, &max_slots, &used_slots);
}

int LruHashtable::GetNext(buf_t& key, buf_t& val, int& idx)
{
    qnobj_t obj;
    int ret = OK;

    memset(&obj, 0, sizeof(qnobj_t));
    bool status = qhasharr_getnext(this->hashtable, &obj, &idx);
    if (!status) {
        idx++;
        if (ENOENT == errno && idx == (this->hashtable->maxslots+1)) {
            return ERR_TBL_END;
        }

        // LOG_ERR("Failed to qhasharr_getnext! idx:%d; errno:%d", idx-1, errno);
        return ERR_NOT_FOUND;
    }

    val.assign((unsigned char*)obj.data, (unsigned char*)obj.data + obj.data_size);
    key.assign((unsigned char*)(obj.name), (unsigned char*)(obj.name) + obj.name_size);

    free(obj.name);
    free(obj.data);

    return ret;
}

int LruHashtable::Remove(const buf_t& key)
{
    if (key.empty()) {
        return ERR_PARAM;
    }

    pthread_mutex_lock(&this->mutex);
    bool ret = qhasharr_remove(this->hashtable, (const char*)(key.data()), key.size());
    pthread_mutex_unlock(&this->mutex);

    if (!ret) {
        return (ENOENT == errno) ? OK : ERR_OTHER;
    }

    return OK;
}

int LruHashtable::Clear(void)
{
    pthread_mutex_lock(&this->mutex);
    qhasharr_clear(this->hashtable);
    delete this->lru;
    pthread_mutex_unlock(&this->mutex);

    return OK;
}

// void LruHashtable::Display(void)
// {
//     int max_slots = 0, used_slots = 0;
//     this->GetCount(max_slots, used_slots);

//     string key, value;

//     for (int idx = 0; idx < max_slots;) {
//         int ret = this->GetNext(key, value, idx);
//         if (ret == OK) {
//             cout << "idx:" << idx << ", key:" << key << ", value:" << value << endl;
//         } else if (ERR_TBL_END == ret) {
//             cout << "idx:" << idx << "ERR_TBL_END" << endl;
//         } else {
//             cout << "idx:" << idx << "Failed to get next item in shm hashtable" << endl;
//             return;
//         }
//     }

//     return;
// }

void LruHashtable::DisplayByLRU(DisplayBuf displayKey, DisplayBuf displayValue)
{
    int max_slots = 0, used_slots = 0;
    this->GetCount(max_slots, used_slots);

    buf_t key, value;
    for (int idx = 0; idx < used_slots; idx++) {
        key = this->lru->GetKey(idx);
        int ret = this->Get(key, value);
        if (ret == OK) {
            cout << "idx:" << idx << ", key:";
            displayKey(key);
            cout << ", value:";
            displayValue(value);
            cout << endl;
        } else if (ERR_TBL_END == ret) {
            cout << "idx:" << idx << "ERR_TBL_END" << endl;
        } else {
            cout << "idx:" << idx << "Failed to get next item in shm hashtable" << endl;
            return;
        }
    }

    return;
}
