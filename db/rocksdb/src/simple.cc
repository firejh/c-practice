/******************************************************
 # DESC    :
 # AUTHOR  : Alex Stocks
 # VERSION : 1.0
 # LICENCE : Apache License 2.0
 # EMAIL   : alexstocks@foxmail.com
 # MOD     : 2017-12-29 00:31
 # FILE    : simple.cc
 ******************************************************/

#include <stdint.h>
#include <unistd.h>

#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/table.h"
#include "rocksdb/options.h"
#include "rocksdb/compaction_filter.h"
#include "rocksdb/utilities/db_ttl.h"
#include "rocksdb/utilities/options_util.h"

using namespace std;
using namespace rocksdb;

DB* db = nullptr;
string kDBPath = "/tmp/rocksdb_data/";
const int32_t DB_TTL = 10;

enum RocksOp {
    RO_WRITE = 1,
    RO_DELETE = 2,
};

namespace {
    // A dummy compaction filter
    class DummyCompactionFilter : public CompactionFilter {
        public:
            virtual ~DummyCompactionFilter() {}
            virtual bool Filter(int level, const Slice& key, const Slice& existing_value,
                        std::string* new_value, bool* value_changed) const {
                return false;
            }
            virtual const char* Name() const { return "DummyCompactionFilter"; }
    };

}  // namespace


void err_exit(char *pre, char *err)
{
    if (err) {
        printf("%s:%s", pre, err);
        if (db != nullptr) {
            delete db;
        }
        _exit(1);
    }
}

void err_exit(string pre, Status& s)
{
    if (!s.ok()) {
        printf("%s, err:%d-%d-%s\n", pre.data(), s.code(), s.subcode(), s.ToString().c_str());
        if (db != nullptr) {
            delete db;
        }
        _exit(1);
    }
}

void put(DB* db, string key, int32_t value)
{
    Status s = db->Put(WriteOptions(), key, Slice((char*)&value, sizeof(value)));
    err_exit((char*)"DB::Put", s);
}

void batch_write(WriteBatch* db, RocksOp op, string key, int32_t value)
{
    switch (op) {
        case RO_WRITE: {
            Status s = db->Put(key, Slice((char*)&value, sizeof(value)));
            err_exit((char*)"DB::Put", s);
            break;
        }
        case RO_DELETE: {
            Status s = db->Delete(key);
            err_exit((char*)"DB::Delete", s);
            break;
        }
    }
}

int testDefaultCF()
{
    Options options;
    options.OptimizeLevelStyleCompaction();
    options.IncreaseParallelism();
    options.create_if_missing = true;

    db = nullptr;
    Status s = DB::Open(options, kDBPath, &db);
    err_exit((char*)"DB::Open", s);
    // Put key-value
    cout << "----put------------" << endl;
    WriteBatch batch;
    batch_write(&batch, RO_WRITE, "/20171229/service0/attr0/0", 1234);
    batch_write(&batch, RO_WRITE, "/20171228/service0/attr0/0", 989);
    batch_write(&batch, RO_WRITE, "/20171227/service0/attr0/0", 4893949);
    batch_write(&batch, RO_WRITE, "/20171226/service0/attr0/0", 83482384);
    s = db->Write(WriteOptions(), &batch);
    err_exit((char*)"DB::Write", s);

    // get value
    cout << "----get------------" << endl;
    // string value;
    // s = db->Get(ReadOptions(), "/20171226/service0/attr0/0", &value);
    PinnableSlice value;
    s = db->Get(ReadOptions(), db->DefaultColumnFamily(), "/20171226/service0/attr0/0", &value);
    err_exit((char*)"DB::Get(\"/20171226/service0/attr0/0\")", s);
    cout << *(int32_t*)(value.data()) << endl;
    assert(*(int32_t*)(value.data()) == (int32_t)(83482384));
    value.Reset();

    // multi-get values
    cout << "----multi-get -----" << endl;
    vector<Status> statusArray;
    vector<Slice> keyArray = {"/20171229/service0/attr0/0", "/20171226/service0/attr0/0"};
    vector<string> valueArray;
    statusArray = db->MultiGet(ReadOptions(), keyArray, &valueArray);
    int idx = 0;
    for (auto &s:statusArray) {
        err_exit((char*)("MultiGet"), s);
        cout << "key:" << keyArray[idx].data_ << ", value:" << *(int32_t*)(valueArray[idx].data()) << endl;
        idx ++;
    }

    // for-range
    cout << "----for-range -----" << endl;
    Iterator* it = db->NewIterator(ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        cout << "key:" << it->key().ToString() << ", value:" << *(int32_t*)(it->value().data()) << endl;
    }

    cout << "-reverse-for-range-" << endl;
    for (it->SeekToLast(); it->Valid(); it->Prev()) {
        cout << "key:" << it->key().ToString() << ", value:" << *(int32_t*)(it->value().data()) << endl;
    }

    // iterator
    cout << "----iterator ------" << endl;
    Slice sliceStart("/20171227");
    string sliceEnd("/20171229");
    for (it->Seek(sliceStart);
         it->Valid() && it->key().ToString() < sliceEnd;
         it->Next()) {
        cout << "key:" << it->key().ToString() << ", value:" << *(int32_t*)(it->value().data()) << endl;
    }

    cout << "--reverse iterator-" << endl;
    for (it->SeekForPrev(sliceEnd);
         it->Valid() && it->key().ToString() > string(sliceStart.data_);
         it->Prev()) {
        cout << "key:" << it->key().ToString() << ", value:" << *(int32_t*)(it->value().data()) << endl;
    }

    // delete value
    cout << "----delete---------" << endl;
    // s = db->Delete(WriteOptions(), "/20171226/service0/attr0/0");
    WriteBatch batch_delete;
    batch_write(&batch_delete, RO_DELETE, "/20171226/service0/attr0/0", 0);
    s = db->Write(WriteOptions(), &batch_delete);
    err_exit((char*)"DB::Delete(\"/20171226/service0/attr0/0\")", s);

    value.Reset();
    // s = db->Get(ReadOptions(), "/20171226/service0/attr0/0", &value);
    s = db->Get(ReadOptions(), db->DefaultColumnFamily(), "/20171226/service0/attr0/0", &value);
    if (s.ok()) {
        cout << "fail to delete key:" << "/20171226/service0/attr0/0" << endl;
    }

    delete db;

    return 0;
}

void cf_batch_write(WriteBatch* batch, ColumnFamilyHandle* cf, RocksOp op, string key, int32_t value)
{
    switch (op) {
        case RO_WRITE: {
            Status s = batch->Put(cf, key, Slice((char*)&value, sizeof(value)));
            err_exit((char*)"Batch::Put", s);
            break;
        }
        case RO_DELETE: {
            Status s = batch->Delete(cf, key);
            err_exit((char*)"Batch::Delete", s);
            break;
        }
    }
}


int testColumnFamilies()
{
    Options options;
    options.OptimizeLevelStyleCompaction();
    options.IncreaseParallelism();
    options.create_if_missing = true;

    ColumnFamilyOptions cf_options(options);
    vector<string> column_family_names;
    vector<ColumnFamilyDescriptor> descriptors;
    vector<ColumnFamilyHandle*> handles;
    string table = "test";

    cout << "-create column family--" << endl;
    Status s = DB::Open(options, kDBPath, &db);
    err_exit((char*)"DB::Open", s);
    // create column family
    ColumnFamilyHandle* cf = NULL;
    s = db->CreateColumnFamily(ColumnFamilyOptions(), table, &cf);
    if (!s.ok()) {
        printf("create table %s, err:%d-%d-%s\n", table.data(), s.code(), s.subcode(), s.ToString().c_str());
    }

    // close DB
    delete cf;
    cf = nullptr;
    delete db;
    db = nullptr;

    // list column family
    s = DB::ListColumnFamilies(options, kDBPath, &column_family_names);
    // err_exit((char*)"DB::ListColumnFamilies", s);
    for (auto name : column_family_names) {
        cout << "column family: " << name << endl;
    }


    cout << "--open read only---" << endl;
    // Open database
    // open DB with two column families
    descriptors.clear();
    // have to open default column family
    descriptors.push_back(ColumnFamilyDescriptor(kDefaultColumnFamilyName, ColumnFamilyOptions()));
    // open the new one, too
    descriptors.push_back(ColumnFamilyDescriptor(table, ColumnFamilyOptions()));
    db = nullptr;
    handles.clear();
    s = DB::OpenForReadOnly(options, kDBPath, descriptors, &handles, &db);
    err_exit((char*)"DB::OpenForReadOnly", s);
    delete db;

    cout << "----open-----------" << endl;
    // Open database
    // open DB with two column families
    descriptors.clear();
    // have to open default column family
    descriptors.push_back(ColumnFamilyDescriptor(kDefaultColumnFamilyName, ColumnFamilyOptions()));
    // open the new one, too
    descriptors.push_back(ColumnFamilyDescriptor(table, ColumnFamilyOptions()));
    db = nullptr;
    handles.clear();
    s = DB::Open(options, kDBPath, descriptors, &handles, &db);
    err_exit((char*)"DB::Open", s);

    // create column family
    if (handles.size() > 0) {
        for (auto &h:handles) {
            if (h->GetName() == table) {
                cf = h;
            } else {
                delete h;
            }
        }
    }
    if (!cf) {
        s = db->CreateColumnFamily(ColumnFamilyOptions(), table, &cf);
        err_exit((char*)"DB::CreateColumnFamily", s);
    }
    cout << "table name:" << cf->GetName() << endl;

    // Put
    cout << "----put------------" << endl;

    string key = "/20180102/service0/attr0/720";
    int value = 12345;

    WriteBatch batch;
    cf_batch_write(&batch, cf, RO_WRITE, key, value);
    s = db->Write(WriteOptions(), &batch);
    err_exit((char*)"DB::Write", s);

    // Get
    cout << "----get------------" << endl;
    PinnableSlice ps;
    cout << "hello0" << endl;
    s = db->Get(ReadOptions(), cf, key, &ps);
    cout << "hello1" << endl;
    err_exit((char*)(("DB::Get(" + key + ")").data()), s);
    cout << *(int32_t*)(ps.data()) << endl;
    assert(*(int32_t*)(ps.data()) == (int32_t)(value));
    ps.Reset();

    // multi-get values
    cout << "----multi-get -----" << endl;
    vector<Status> statusArray;
    vector<ColumnFamilyHandle*> cfArray = {cf};
    vector<Slice> keyArray = {key};
    vector<string> valueArray;
    statusArray = db->MultiGet(ReadOptions(), cfArray, keyArray, &valueArray);
    int idx = 0;
    for (auto &s:statusArray) {
        err_exit((char*)("MultiGet"), s);
        cout << "key:" << keyArray[idx].data_ << ", value:" << *(int32_t*)(valueArray[idx].data()) << endl;
        idx ++;
    }

    // for-range
    cout << "----for-range -----" << endl;
    Iterator* it = db->NewIterator(ReadOptions(), cf);
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        cout << "key:" << it->key().ToString() << ", value:" << *(int32_t*)(it->value().data()) << endl;
    }

    cout << "-reverse-for-range-" << endl;
    for (it->SeekToLast(); it->Valid(); it->Prev()) {
        cout << "key:" << it->key().ToString() << ", value:" << *(int32_t*)(it->value().data()) << endl;
    }

    // iterator
    cout << "----iterator ------" << endl;
    Slice sliceStart("/20171227");
    string sliceEnd("/20181229");
    for (it->Seek(sliceStart);
         it->Valid() && it->key().ToString() < sliceEnd;
         it->Next()) {
        cout << "key:" << it->key().ToString() << ", value:" << *(int32_t*)(it->value().data()) << endl;
    }

    cout << "--reverse iterator-" << endl;
    for (it->SeekForPrev(sliceEnd);
         it->Valid() && it->key().ToString() > string(sliceStart.data_);
         it->Prev()) {
        cout << "key:" << it->key().ToString() << ", value:" << *(int32_t*)(it->value().data()) << endl;
    }

    // delete value
    cout << "----delete---------" << endl;
    // s = db->Delete(WriteOptions(), cf, key);
    WriteBatch batch2;
    cf_batch_write(&batch2, cf, RO_DELETE, key, 0);
    s = db->Write(WriteOptions(), &batch2);
    err_exit((char*)(("DB::Delete(" + key + ")").data()), s);

    ps.Reset();
    // s = db->Get(ReadOptions(), "/20171226/service0/attr0/0", &value);
    s = db->Get(ReadOptions(), cf, key, &ps);
    if (s.ok()) {
        cout << "fail to delete key:" << key << endl;
    }

    // close column family
    // db->DropColumnFamily(cf); // this api will delete all "test" column family's data
    delete cf;

    delete db;

    // delete the DB
    DestroyDB(kDBPath, Options());

    return 0;
}

// rm -rf kDBPath before run this test
int testTTL()
{
    Options options;
    options.OptimizeLevelStyleCompaction();
    options.IncreaseParallelism();
    options.create_if_missing = true;
    options.disable_auto_compactions = true;

    ColumnFamilyOptions cf_options(options);
    vector<string> column_family_names;
    vector<ColumnFamilyDescriptor> descriptors;
    vector<ColumnFamilyHandle*> handles;
    string cfname = "test_cf";
    vector<int32_t> TTL;

    TTL.push_back(DB_TTL);
    TTL.push_back(DB_TTL);

    cout << "-create column family--" << endl;
    DB* db = nullptr;
    ColumnFamilyHandle* cf = nullptr;
    Status s = DB::Open(options, kDBPath, &db);
    err_exit((char*)"DB::Open", s);
    s = db->CreateColumnFamily(ColumnFamilyOptions(), cfname, &cf);
    assert(s.ok());
    delete cf;
    delete db;

    descriptors.push_back(ColumnFamilyDescriptor(kDefaultColumnFamilyName, ColumnFamilyOptions()));
    descriptors.push_back(ColumnFamilyDescriptor(cfname, ColumnFamilyOptions()));

    rocksdb::DBWithTTL* ttlDB = nullptr;
    s = DBWithTTL::Open(options, kDBPath, descriptors, &handles, &ttlDB, TTL, false);
    err_exit((char*)"DB::Open", s);
    db = ttlDB;
    // create column family
    // ColumnFamilyHandle* cf = NULL;
    // s = db->CreateColumnFamily(ColumnFamilyOptions(), cfname, &cf);
    // if (!s.ok()) {
    //     printf("create table %s, err:%d-%d-%s\n", cfname.data(), s.code(), s.subcode(), s.ToString().c_str());
    // }

    // Put
    cout << "----put------------" << endl;

    string key = "/20180102/service0/attr0/720";
    int value = 12345;
    s = db->Put(WriteOptions(), handles[1], key, Slice((char*)&value, sizeof(value)));
    err_exit((char*)"DB::Put", s);

    // Get
    cout << "----get------------" << endl;
    PinnableSlice ps;
    s = db->Get(ReadOptions(), handles[1], key, &ps);
    err_exit((char*)(("DB::Get(" + key + ")").data()), s);
    cout << *(int32_t*)(ps.data()) << endl;
    ps.Reset();

    // Flush
    FlushOptions flush_options;
    flush_options.wait = true;
    db->Flush(flush_options, handles[1]);

    // Sleep
    chrono::seconds dura(DB_TTL + 10);
    this_thread::sleep_for(dura);

    // Manual Compaction
    // s = db->CompactFiles(rocksdb::CompactionOptions(), {kDBPath + "/000010.sst"}, 0);
    // err_exit((char*)"DB::CompactFiles", s);
    Slice sliceStart(key);
    string end("/20181229");
    Slice sliceEnd(end);
    s = db->CompactRange(CompactRangeOptions(), handles[1], &sliceStart, &sliceEnd);
    err_exit((char*)"DB::CompactRange", s);

    // iterator
    cout << "----iterator ------" << endl;
    Iterator* it = db->NewIterator(ReadOptions(), handles[1]);
    for (it->Seek(sliceStart);
         it->Valid() && it->key().ToString() < end;
         it->Next()) {
        cout << "key:" << it->key().ToString() << ", value:" << *(int32_t*)(it->value().data()) << endl;
    }

    // Get
    cout << "----get------------" << endl;
    ps.Reset();
    s = db->Get(ReadOptions(), handles[1], key, &ps);
    err_exit((char*)(("DB::Get(" + key + ")").data()), s);
    cout << *(int32_t*)(ps.data()) << endl;
    ps.Reset();

    // close DB
    delete cf;
    cf = nullptr;
    delete db;
    db = nullptr;

    return 0;
}

int testBlockCache()
{
    DBOptions db_opt;
    db_opt.create_if_missing = true;

    std::vector<ColumnFamilyDescriptor> cf_descs;
    cf_descs.push_back({kDefaultColumnFamilyName, ColumnFamilyOptions()});
    cf_descs.push_back({"new_cf", ColumnFamilyOptions()});

    // initialize BlockBasedTableOptions
    // auto cache = NewLRUCache(1 * 1024 * 1024 * 1024);
    auto cache = NewLRUCache(1 * 1024);
    BlockBasedTableOptions bbt_opts;
    bbt_opts.block_size = 32 * 1024;
    bbt_opts.block_cache = cache;

    // initialize column families options
    std::unique_ptr<CompactionFilter> compaction_filter;
    compaction_filter.reset(new DummyCompactionFilter());
    cf_descs[0].options.table_factory.reset(NewBlockBasedTableFactory(bbt_opts));
    cf_descs[0].options.compaction_filter = compaction_filter.get();
    cf_descs[1].options.table_factory.reset(NewBlockBasedTableFactory(bbt_opts));

    // destroy and open DB
    DB* db;
    Status s = DestroyDB(kDBPath, Options(db_opt, cf_descs[0].options));
    assert(s.ok());
    s = DB::Open(Options(db_opt, cf_descs[0].options), kDBPath, &db);
    assert(s.ok());

    // Create column family, and rocksdb will persist the options.
    ColumnFamilyHandle* cf;
    s = db->CreateColumnFamily(ColumnFamilyOptions(), "new_cf", &cf);
    assert(s.ok());

    // close DB
    delete cf;
    delete db;

    // In the following code, we will reopen the rocksdb instance using
    // the options file stored in the db directory.

    // Load the options file.
    DBOptions loaded_db_opt;
    std::vector<ColumnFamilyDescriptor> loaded_cf_descs;
    s = LoadLatestOptions(kDBPath, Env::Default(), &loaded_db_opt, &loaded_cf_descs);
    assert(s.ok());
    assert(loaded_db_opt.create_if_missing == db_opt.create_if_missing);

    // Initialize pointer options for each column family
    for (size_t i = 0; i < loaded_cf_descs.size(); ++i) {
        auto* loaded_bbt_opt = reinterpret_cast<BlockBasedTableOptions*>(
                    loaded_cf_descs[0].options.table_factory->GetOptions());
        // Expect the same as BlockBasedTableOptions will be loaded form file.
        assert(loaded_bbt_opt->block_size == bbt_opts.block_size);
        // However, block_cache needs to be manually initialized as documented
        // in rocksdb/utilities/options_util.h.
        loaded_bbt_opt->block_cache = cache;
    }
    // In addition, as pointer options are initialized with default value,
    // we need to properly initialized all the pointer options if non-defalut
    // values are used before calling DB::Open().
    assert(loaded_cf_descs[0].options.compaction_filter == nullptr);
    loaded_cf_descs[0].options.compaction_filter = compaction_filter.get();

    // reopen the db using the loaded options.
    std::vector<ColumnFamilyHandle*> handles;
    s = DB::Open(loaded_db_opt, kDBPath, loaded_cf_descs, &handles, &db);
    assert(s.ok());

    // close DB
    for (auto* handle : handles) {
        delete handle;
    }
    delete db;

    return 0;
}

int main(int argc, char** argv)
{
    // testDefaultCF();
    // testColumnFamilies();
    // testTTL();
    testBlockCache();

    return 0;
}
