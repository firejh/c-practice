1 debug.bz2 这个是带有debug信息的redis源码，基于redis3.0 unstable
2 test.bz2 带有redis数据和client的测试包
3 redis-3.0.2.debug.migrate 带有debug和修改后的带有password验证的redis源码包
   2015/06/18 今天修改了hiredis/asyn.c:__redisAsyncCommand:
    /* Always schedule a write when the write buffer is non-empty */
    // _EL_ADD_WRITE(ac);
	redisAsyncHandleWrite(ac);
	redisAsyncCommand把命令写入缓存，然后注册一个写的函数到事件库，等待事件库发现fd可写之后调用这个函数来做真的写。我把这个添加write事件去除了，直接发送。
