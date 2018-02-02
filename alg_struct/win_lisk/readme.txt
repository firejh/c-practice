tool		: vc2012
date		: 2014-04-23
author		: Alex Stocks
email		: alexstocks@foxmail.com
version		: 0.5
license		: GPL 2.0
description	:
	主要有一些数据结构:
	1 可伸缩性hashtable。
		其实现参考了redis的hashtable。所谓的可伸缩，即用户可以设置hashtable初始的大小，而后可平滑伸缩，此table会随着用户插入元素的增多而扩大，亦能随着删除后元素的减少而缩小，在用户计算机内存限制范围内，用户都不需要再考虑table的大小。

	2 由红黑树实现的map
		红黑树是我目前见过的最棒的数据结构。其插入删除以及查找效率高于上面的hashtable十倍。虽然如是，但是其操作逻辑也远比上面的hashtable复杂。
	
	3 多级hash(tencent非常常用的一种数据结构)
		关于多级hash，可以参考一下文章：
		http://ahfuzhang.blogspot.com/2012/09/hash.html					《多阶hash表 》
		http://www.cppblog.com/lmlf001/archive/2007/09/08/31858.html	《使用共享内存的多级哈希表的一种实现》
		http://blog.csdn.net/fullsail/article/details/6328702			《多次HASH算法解决冲突的效果测试》
		第三篇文章介绍的挺详细，可作参考。
		我的源码中的多级hash稍微不同于上面文章介绍的多级hash之处在于，我的多级hash是递增的。其结构如下：
		■■■
		■■■■■■
		■■■■■■■■■■
		■■■■■■■■■■■■■
		■■■■■■■■■■■■■■■
		这种相反的结构的好处是可以几乎100%的把用户的数据存储下来。
	
	4 双端队列 deque
		如果你只在一个链表的首尾进行push&pop操作，它的效率是最棒的。可代替queue和stack。


addition：欢迎各位通过邮箱向我说明bug或者其他东东^_^。
	
      





