# rocksdb #
---
 *a netty like asynchronous network I/O library*

## introdction ##
---
> DESC       : test rocksdb and give its code example
>
> LICENCE    : Apache License 2.0

## develop history ##
---

- 2018/01/04
    * 1 add testBlockCache

- 2018/01/04
    * 1 add testTTL

- 2018/01/02
    * 1 add testColumnFamilies

    	> column family的详细说明可见[Column-Families](https://github.com/facebook/rocksdb/wiki/Column-Families)

    	> 关于如何打开一个column family可参见代码 [column_families_example](https://github.com/facebook/rocksdb/blob/master/examples/column_families_example.cc#L36)，line36指出每次打开一个非default column family时，不管是Open还是OpenForReadOnly都必须连带着也打开default column family，否则rocksdb给出这样的错误：Default column family not specified

	* 2 查看cf的值：ldb --db=/tmp/rocksdb_data/ --column_family=test scan --value_hex

        >ldb的详细使用说明可见[Administration and Data Access Tool](https://github.com/facebook/rocksdb/wiki/Administration-and-Data-Access-Tool)

    * 3 测试过程中发现程序不小心core掉后，所有数据都丢失了

