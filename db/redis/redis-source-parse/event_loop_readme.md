# redis server event loop 源码分析#
---
*written by Alex Stocks on 2015/08/02*

##1 redis主流程 ##

<font color=blue>

**redis启动流程:**

- 1 加载配置；
- 2 初始化redis master、slave以及sentinel的sri；
- 3 注册事件事件serverCron。

</font>

<font color=green>

    int main(int argc, char **argv) {
        // 初始化conf
        initServerConfig();

        // 加载配置
        if (argc >= 2) {
            loadServerConfig(configfile,options);
            sdsfree(options);
        }
        // 启动server
        initServer();

        // 启动linux的内存优化项[overcommit_memory & transparent_huge_page]，从磁盘加载数据
        if (!server.sentinel_mode) {
            /* Things not needed when running in Sentinel mode. */
            redisLog(REDIS_WARNING,"Server started, Redis version " REDIS_VERSION);
    #ifdef __linux__
            linuxMemoryWarnings();
    #endif
            checkTcpBacklogSettings();
            loadDataFromDisk();
        }

        // 开足马力，运转发动机
        aeSetBeforeSleepProc(server.el,beforeSleep);
        aeMain(server.el);
        aeDeleteEventLoop(server.el);

        return 0;
    }

</font>

###1.1 初始化server相关参数###

<font color>
初始化server.runid，并给replication相关参数赋值。
</font>

<font color=yellow>

>Linux中的随机数可以从两个特殊的文件中产生，一个是/dev/urandom.另外一个是/dev/random。他们产生随机数的原理是利用当前系统的熵池来计算出固定一定数量的随机比特，然后将这些比特作为字节流返回。
>
 >熵池就是当前系统的环境噪音，熵指的是一个系统的混乱程度，系统噪音可以通过很多参数来评估，如内存的使用，文件的使用量，不同类型的进程数量等等。如果当前环境噪音变化的不是很剧烈或者当前环境噪音很小，比如刚开机的时候，而当前需要大量的随机比特，这时产生的随机数的随机效果就不是很好了。这就是为什么会有/dev/urandom和/dev/random这两种不同的文件，后者在不能产生新的随机数时会阻塞程序，而前者不会（ublock），当然产生的随机数效果就不太好了，这对加密解密这样的应用来说就不是一种很好的选择。
>
>/dev/random会阻塞当前的程序，直到根据熵池产生新的随机字节之后才返回，所以使用/dev/random比使用/dev/urandom产生大量随机数的速度要慢。
>

</font>

<font color=blue>

    /*
     * 这个函数为一个redis instance生成一个160bit的id
     * (如果未来显示用则是320bit，即40个字母长度的可显示字符串)。
     */
    void getRandomHexChars(char *p, unsigned int len)；
    {
        char *charset = "0123456789abcdef";
        unsigned int j;

        /* Global state. */
        static int seed_initialized = 0;
        static unsigned char seed[20]; /* The SHA1 seed, from /dev/urandom. */
        static uint64_t counter = 0; /* The counter we hash with the seed. */
        //  生成一个随机种子数，存储到seed里面
        if (!seed_initialized) {
            FILE *fp = fopen("/dev/urandom", "r");
            if (fp && fread(seed, sizeof(seed), 1, fp) == 1)
                seed_initialized = 1;
            if (fp) fclose(fp);
        }

        if (seed_initialized) {
            // 如果从/dev/urandom读取到了随机字符串，则利用SHA算法生成一个id
            while (len) {
                // len可能并不是20或者40字节的倍数，所以这里要通过循环把p的内容填满
                unsigned char digest[20];
                SHA1_CTX ctx;
                unsigned int copylen = len > 20 ? 20 : len;

                SHA1Init(&ctx);
                SHA1Update(&ctx, seed, sizeof(seed));
                SHA1Update(&ctx, (unsigned char*)&counter, sizeof(counter));
                SHA1Final(digest, &ctx);
                counter++;

                memcpy(p, digest, copylen);
                /* Convert to hex digits. */
                // 把数字转化为可读字符串，只是这里只用了一个字节的后半部分
                for (j = 0; j < copylen; j++) p[j] = charset[p[j] & 0x0F];

                // 移动光标
                len -= copylen;
                p += copylen;
            }
        }
        else {
            // 如果从/dev/urandom读取随机字符串失败，则利用时间和当前进程的id来生成一个随机字符串
            char *x = p;
            unsigned int l = len;
            struct timeval tv;
            pid_t pid = getpid();

            /* Use time and PID to fill the initial array. */
            // 先在buf中填充时间的秒和微秒两个部分，然后再补充上进程的id
            gettimeofday(&tv, NULL);
            if (l >= sizeof(tv.tv_usec)) {
                memcpy(x, &tv.tv_usec, sizeof(tv.tv_usec));
                l -= sizeof(tv.tv_usec);
                x += sizeof(tv.tv_usec);
            }
            if (l >= sizeof(tv.tv_sec)) {
                memcpy(x, &tv.tv_sec, sizeof(tv.tv_sec));
                l -= sizeof(tv.tv_sec);
                x += sizeof(tv.tv_sec);
            }
            if (l >= sizeof(pid)) {
                memcpy(x, &pid, sizeof(pid));
                l -= sizeof(pid);
                x += sizeof(pid);
            }
            // 再利用随机数进行异或后，转化为16进制可视字符串
            for (j = 0; j < len; j++) {
                p[j] ^= rand();
                p[j] = charset[p[j] & 0x0F];
            }
        }
    }

    void initServerConfig(void) {
        int j;

        getRandomHexChars(server.runid,REDIS_RUN_ID_SIZE);
        server.configfile = NULL;
        server.hz = REDIS_DEFAULT_HZ;
        server.runid[REDIS_RUN_ID_SIZE] = '\0';
        server.arch_bits = (sizeof(long) == 8) ? 64 : 32;
        server.port = REDIS_SERVERPORT;
        server.repl_ping_slave_period = REDIS_REPL_PING_SLAVE_PERIOD;
        server.repl_timeout = REDIS_REPL_TIMEOUT;
        server.repl_min_slaves_to_write = REDIS_DEFAULT_MIN_SLAVES_TO_WRITE;
        server.repl_min_slaves_max_lag = REDIS_DEFAULT_MIN_SLAVES_MAX_LAG;

        /* Replication related */
        server.masterauth = NULL;
        server.masterhost = NULL;
        server.masterport = 6379;
        server.master = NULL;
        server.cached_master = NULL;
        server.repl_master_initial_offset = -1;
        server.repl_state = REDIS_REPL_NONE;
        server.repl_syncio_timeout = REDIS_REPL_SYNCIO_TIMEOUT;
        server.repl_serve_stale_data = REDIS_DEFAULT_SLAVE_SERVE_STALE_DATA;
        server.repl_slave_ro = REDIS_DEFAULT_SLAVE_READ_ONLY;
        server.repl_down_since = 0; /* Never connected, repl is down since EVER. */
        server.repl_disable_tcp_nodelay = REDIS_DEFAULT_REPL_DISABLE_TCP_NODELAY;
        server.repl_diskless_sync = REDIS_DEFAULT_REPL_DISKLESS_SYNC;
        server.repl_diskless_sync_delay = REDIS_DEFAULT_REPL_DISKLESS_SYNC_DELAY;
        server.slave_priority = REDIS_DEFAULT_SLAVE_PRIORITY;
        server.master_repl_offset = 0;

        /* Replication partial resync backlog */
        server.repl_backlog = NULL;
        server.repl_backlog_size = REDIS_DEFAULT_REPL_BACKLOG_SIZE;
        server.repl_backlog_histlen = 0;
        server.repl_backlog_idx = 0;
        server.repl_backlog_off = 0;
        server.repl_backlog_time_limit = REDIS_DEFAULT_REPL_BACKLOG_TIME_LIMIT;
        server.repl_no_slaves_since = time(NULL);

        /* write index */
        // server.write_index.start = (long long)((long long)(0XE20150620180647F) * getpid() * server.port * time(NULL));
        server.write_index.start = 0;
        server.write_index.current = server.write_index.start;
        server.slave_write_index = dictCreate(&slaveWriteIndexDictType,NULL);
    }

</font>

###1.2 加载配置###

####1.2.1 读取配置文件####

<font color=blue>
如果进程启动的时候已经被赋予了redis的options，则使用这些options，不再读conf文件。
</font>

<font color=green>

    void loadServerConfig(char *filename, char *options) {
        sds config = sdsempty();
        char buf[REDIS_CONFIGLINE_MAX+1];

        /* Load the file content */
        if (filename) {
            FILE *fp;

            // 如果文件名称为空，则从stdin读取
            if (filename[0] == '-' && filename[1] == '\0') {
                fp = stdin;
            } else {
                if ((fp = fopen(filename,"r")) == NULL) {
                    redisLog(REDIS_WARNING,
                        "Fatal error, can't open config file '%s'", filename);
                    exit(1);
                }
            }
            while(fgets(buf,REDIS_CONFIGLINE_MAX+1,fp) != NULL)
                config = sdscat(config,buf);
            if (fp != stdin) fclose(fp);
        }
        /* Append the additional options */
        if (options) {
            config = sdscat(config,"\n");
            config = sdscat(config,options);
        }
        loadServerConfigFromString(config);
        sdsfree(config);
    }

</font>

####1.2.2 对字符流@config逐行拆分####

<font color=blue>
这个函数中比较重要的就是如果redis的role是slave，则获取master的host和port，并把state设置为REDIS_REPL_CONNECT
</font>

<font color=green>

    void loadServerConfigFromString(char *config) {
        char *err = NULL;
        int linenum = 0, totlines, i;
        int slaveof_linenum = 0;
        sds *lines;

        // 按照行进行分割，结果存在lines数组中，行数为totlines
        lines = sdssplitlen(config,strlen(config),"\n",1,&totlines);

        for (i = 0; i < totlines; i++) {
            sds *argv;
            int argc;

            linenum = i+1; //记录行号，一旦出错，下面的loaderr就能说明出错所在的行号
            // 去掉tab、换行、回车等空格键
            lines[i] = sdstrim(lines[i]," \t\r\n");
            // 不处理空行和注释行
            if (lines[i][0] == '#' || lines[i][0] == '\0') continue;

            // 把每行再进行分割，分割结果存进@argv数组，数组elem个数为args
            argv = sdssplitargs(lines[i],&argc);
            if (argv == NULL) { // 处理argv为空的情况
                err = "Unbalanced quotes in configuration line";
                goto loaderr;
            }
            if (argc == 0) { // 处理element number为0的情况
                sdsfreesplitres(argv,argc);
                continue;
            }
            sdstolower(argv[0]);  // 把line key转换为消息

            /* Execute config directives */
            if (!strcasecmp(argv[0],"slaveof") && argc == 3) {
                slaveof_linenum = linenum;
                server.masterhost = sdsnew(argv[1]);
                server.masterport = atoi(argv[2]);
                server.repl_state = REDIS_REPL_CONNECT;
            } else if (!strcasecmp(argv[0],"repl-ping-slave-period") && argc == 2) {
                server.repl_ping_slave_period = atoi(argv[1]);
                if (server.repl_ping_slave_period <= 0) {
                    err = "repl-ping-slave-period must be 1 or greater";
                    goto loaderr;
                }
            } else if (!strcasecmp(argv[0],"repl-timeout") && argc == 2) {
                server.repl_timeout = atoi(argv[1]);
                if (server.repl_timeout <= 0) {
                    err = "repl-timeout must be 1 or greater";
                    goto loaderr;
                }
            } else if (!strcasecmp(argv[0],"repl-disable-tcp-nodelay") && argc==2) {
                if ((server.repl_disable_tcp_nodelay = yesnotoi(argv[1])) == -1) {
                    err = "argument must be 'yes' or 'no'"; goto loaderr;
                }
            } else if (!strcasecmp(argv[0],"repl-diskless-sync") && argc==2) {
                if ((server.repl_diskless_sync = yesnotoi(argv[1])) == -1) {
                    err = "argument must be 'yes' or 'no'"; goto loaderr;
                }
            } else if (!strcasecmp(argv[0],"repl-diskless-sync-delay") && argc==2) {
                server.repl_diskless_sync_delay = atoi(argv[1]);
                if (server.repl_diskless_sync_delay < 0) {
                    err = "repl-diskless-sync-delay can't be negative";
                    goto loaderr;
                }
            } else if (!strcasecmp(argv[0],"repl-backlog-size") && argc == 2) {
                long long size = memtoll(argv[1],NULL);
                if (size <= 0) {
                    err = "repl-backlog-size must be 1 or greater.";
                    goto loaderr;
                }
                resizeReplicationBacklog(size);
            } else if (!strcasecmp(argv[0],"repl-backlog-ttl") && argc == 2) {
                server.repl_backlog_time_limit = atoi(argv[1]);
                if (server.repl_backlog_time_limit < 0) {
                    err = "repl-backlog-ttl can't be negative ";
                    goto loaderr;
                }
            }

            else {
                err = "Bad directive or wrong number of arguments"; goto loaderr;
            }
            // 释放element数组
            sdsfreesplitres(argv,argc);
        }

        // 释放line数组
        sdsfreesplitres(lines,totlines);
        return;

    loaderr:
        fprintf(stderr, "\n*** FATAL CONFIG FILE ERROR ***\n");
        fprintf(stderr, "Reading the configuration file, at line %d\n", linenum);
        fprintf(stderr, ">>> '%s'\n", lines[i]);
        fprintf(stderr, "%s\n", err);
        exit(1);
    }

</font>

###1.3 启动server ###

####1.3.1 监听端口 ####

<font color=green>

	static int anetListen(char *err, int s, struct sockaddr *sa, socklen_t len, int backlog) {
	    if (bind(s,sa,len) == -1) {
	        anetSetError(err, "bind: %s", strerror(errno));
	        close(s);
	        return ANET_ERR;
	    }
	
	    if (listen(s, backlog) == -1) {
	        anetSetError(err, "listen: %s", strerror(errno));
	        close(s);
	        return ANET_ERR;
	    }
	    return ANET_OK;
	}
	
	static int _anetTcpServer(char *err, int port, char *bindaddr, int af, int backlog)
	{
	    int s, rv;
	    char _port[6];  /* strlen("65535") */
	    struct addrinfo hints, *servinfo, *p;
	
	    snprintf(_port,6,"%d",port);
	    memset(&hints,0,sizeof(hints));
	    hints.ai_family = af;
	    hints.ai_socktype = SOCK_STREAM;
	    hints.ai_flags = AI_PASSIVE;    /* No effect if bindaddr != NULL */
	
	    if ((rv = getaddrinfo(bindaddr,_port,&hints,&servinfo)) != 0) {
	        anetSetError(err, "%s", gai_strerror(rv));
	        return ANET_ERR;
	    }
	    for (p = servinfo; p != NULL; p = p->ai_next) {
	        if ((s = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
	            continue;
	        
	        if (af == AF_INET6 && anetV6Only(err,s) == ANET_ERR) goto error;
	        if (anetSetReuseAddr(err,s) == ANET_ERR) goto error;
	        if (anetListen(err,s,p->ai_addr,p->ai_addrlen,backlog) == ANET_ERR) goto error;
	        goto end;
	    }
	    if (p == NULL) {
	        anetSetError(err, "unable to bind socket");
	        goto error;
	    }
	
	error:
	    s = ANET_ERR;
	end:
	    freeaddrinfo(servinfo);
	    return s;
	}
	
	int anetTcpServer(char *err, int port, char *bindaddr, int backlog)
	{
	    return _anetTcpServer(err, port, bindaddr, AF_INET, backlog);
	}
	
	int listenToPort(int port, int *fds, int *count) {
	    int j;
	
	    /* Force binding of 0.0.0.0 if no bind address is specified, always
	     * entering the loop if j == 0. */
	    if (server.bindaddr_count == 0) server.bindaddr[0] = NULL;
	    for (j = 0; j < server.bindaddr_count || j == 0; j++) {
	        if (server.bindaddr[j] == NULL) {
	            /* Bind * for both IPv6 and IPv4, we enter here only if
	             * server.bindaddr_count == 0. */
	            fds[*count] = anetTcp6Server(server.neterr,port,NULL,
	                server.tcp_backlog);
	            if (fds[*count] != ANET_ERR) {
	                anetNonBlock(NULL,fds[*count]);
	                (*count)++;
	            }
	            fds[*count] = anetTcpServer(server.neterr,port,NULL,
	                server.tcp_backlog);
	            if (fds[*count] != ANET_ERR) {
	                anetNonBlock(NULL,fds[*count]);
	                (*count)++;
	            }
	            /* Exit the loop if we were able to bind * on IPv4 or IPv6,
	             * otherwise fds[*count] will be ANET_ERR and we'll print an
	             * error and return to the caller with an error. */
	            if (*count) break;
	        } else if (strchr(server.bindaddr[j],':')) {
	            /* Bind IPv6 address. */
	            fds[*count] = anetTcp6Server(server.neterr,port,server.bindaddr[j],
	                server.tcp_backlog);
	        } else {
	            /* Bind IPv4 address. */
	            fds[*count] = anetTcpServer(server.neterr,port,server.bindaddr[j],
				                server.tcp_backlog);
	        }
	        if (fds[*count] == ANET_ERR) {
	            redisLog(REDIS_WARNING,
	                "Creating Server TCP listening socket %s:%d: %s",
	                server.bindaddr[j] ? server.bindaddr[j] : "*",
	                port, server.neterr);
	            return REDIS_ERR;
	        }
	        anetNonBlock(NULL,fds[*count]);
	        (*count)++;
	    }
	    return REDIS_OK;
	}
	
	void initServer(void) {
	    /* Open the TCP listening socket for the user commands. */
	    if (server.port != 0 &&
	        listenToPort(server.port,server.ipfd,&server.ipfd_count) == REDIS_ERR)
	        exit(1);
		}
	}

</font>

####1.3.2 注册监听回调函数 ####

<font color=green>
	
	void initServer(void) {
		/* Create an event handler for accepting new connections in TCP and Unix
	     * domain sockets. */
	    for (j = 0; j < server.ipfd_count; j++) {
	        if (aeCreateFileEvent(server.el, server.ipfd[j], AE_READABLE,
	            acceptTcpHandler,NULL) == AE_ERR)
	            {
	                redisPanic(
	                    "Unrecoverable error creating server.ipfd file event.");
	            }
	    }
	}
	
</font>

####1.3.3 接受外部连接，并创建对应的客户端 ####
	
####1.3.3.1 接受外部连接 ####

<font color=green>
	
	static int anetGenericAccept(char *err, int s, struct sockaddr *sa, socklen_t *len) {
	    int fd;
	    while(1) {
	        fd = accept(s,sa,len);
	        if (fd == -1) {
	            if (errno == EINTR)
	                continue;
	            else {
	                anetSetError(err, "accept: %s", strerror(errno));
	                return ANET_ERR;
	            }
	        }
	        break;
	    }
	    return fd;
	}

	int anetTcpAccept(char *err, int s, char *ip, size_t ip_len, int *port) {
	    int fd;
	    struct sockaddr_storage sa;
	    socklen_t salen = sizeof(sa);
	    if ((fd = anetGenericAccept(err,s,(struct sockaddr*)&sa,&salen)) == -1)
	        return ANET_ERR;
	
	    if (sa.ss_family == AF_INET) {
	        struct sockaddr_in *s = (struct sockaddr_in *)&sa;
	        if (ip) inet_ntop(AF_INET,(void*)&(s->sin_addr),ip,ip_len);
	        if (port) *port = ntohs(s->sin_port);
	    } else {
	        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
	        if (ip) inet_ntop(AF_INET6,(void*)&(s->sin6_addr),ip,ip_len);
	        if (port) *port = ntohs(s->sin6_port);
	    }
	    return fd;
	}

</font>

####1.3.3.2 创建客户端 ####

<font color=blue>
 
>创建客户端后注册接受client的请求的回调函数readQueryFromClient，最后把客户端放入server的客户端集合:server.clients。
>
>如果client集合超限[默认是10000]就给client返回err msg，然后再释放client句柄并关闭连接。

</font>

<font color=green>

	redisClient *createClient(int fd) {
	    redisClient *c = zmalloc(sizeof(redisClient));
	
	    /* passing -1 as fd it is possible to create a non connected client.
	     * This is useful since all the Redis commands needs to be executed
	     * in the context of a client. When commands are executed in other
	     * contexts (for instance a Lua script) we need a non connected client. */
	    if (fd != -1) {
	        anetNonBlock(NULL,fd);
			// 把fd设置为nodelay类型，有利于数据及时发送给客户端
	        anetEnableTcpNoDelay(NULL,fd);
	        if (server.tcpkeepalive)
	            anetKeepAlive(NULL,fd,server.tcpkeepalive);
		    // 注册接受客户端请求的函数
	        if (aeCreateFileEvent(server.el,fd,AE_READABLE,
	            readQueryFromClient, c) == AE_ERR)
	        {
	            close(fd);
	            zfree(c);
	            return NULL;
	        }
	    }
		
		// 把外部请求客户端放入客户端集合，即fake client是不会被放进去的
	    if (fd != -1) listAddNodeTail(server.clients,c);
	    initClientMultiState(c);
	    return c;
	}

	#define MAX_ACCEPTS_PER_CALL 1000
	static void acceptCommonHandler(int fd, int flags) {
	    redisClient *c;
	    if ((c = createClient(fd)) == NULL) {
	        redisLog(REDIS_WARNING,
	            "Error registering fd event for the new client: %s (fd=%d)",
	            strerror(errno),fd);
	        close(fd); /* May be already closed, just ignore errors */
	        return;
	    }
	    /* If maxclient directive is set and this is one client more... close the
	     * connection. Note that we create the client instead to check before
	     * for this condition, since now the socket is already set in non-blocking
	     * mode and we can send an error for free using the Kernel I/O */
		// 如果连接过多[#define REDIS_MAX_CLIENTS 10000]，先给client发送一个error msg，
		// 然后就把连接代表的client给free掉
		//
		// 这里解释了为何最后才检查超限的原因：需要给客户端发送error message，以让用户明白错误的原因。
	    if (listLength(server.clients) > server.maxclients) {
	        char *err = "-ERR max number of clients reached\r\n";
	
	        /* That's a best effort error message, don't check write errors */
	        if (write(c->fd,err,strlen(err)) == -1) {
	            /* Nothing to do, Just to avoid the warning... */
	        }
	        server.stat_rejected_conn++;
	        freeClient(c);
	        return;
	    }
	    server.stat_numconnections++;
	    c->flags |= flags;
	}
			
</font>
	
####1.3.3.3 在监听端口上接受请求并创建对应客户端的综合流程 ####

<font color=blue>

>目前逻辑步骤是：接受客户端的请求；创建对应的client句柄；插入client集合；检查client集合是否超限。
>
>那为什么不先检查server.clients的数目后直接把超限的连接请求给拒绝掉呢？
>
>acceptCommonHandler()函数里给出了一段解释：需要给客户端发送error message，以让用户明白错误的原因。

</font>

<font color=green>

	// 接受外部请求，一次最多接受1000个请求
	void acceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
	    int cport, cfd, max = MAX_ACCEPTS_PER_CALL;
	    char cip[REDIS_IP_STR_LEN];
	    REDIS_NOTUSED(el);
	    REDIS_NOTUSED(mask);
	    REDIS_NOTUSED(privdata);
	
	    while(max--) {
	        cfd = anetTcpAccept(server.neterr, fd, cip, sizeof(cip), &cport);
	        if (cfd == ANET_ERR) {
	            if (errno != EWOULDBLOCK)
	                redisLog(REDIS_WARNING,
	                    "Accepting client connection: %s", server.neterr);
	            return;
	        }
	        redisLog(REDIS_VERBOSE,"Accepted %s:%d", cip, cport);
	        acceptCommonHandler(cfd,0);
	    }
	}

</font>

###1.4 检查内存###

####1.4.1 检查overcommit_memory ####
<font color=blue>

overcommit_memory文件指定了内核针对内存分配的策略，overcommit_memory值可以是0、1、2。

- 0， 表示内核将检查是否有足够的可用内存供应用进程使用；如果有足够的可用内存，内存申请允许；否则，内存申请失败，并把错误返回给应用进程。
- 1， 表示内核允许分配所有的物理内存，而不管当前的内存状态如何。
- 2， 表示内核允许分配超过所有物理内存和交换空间总和的内存

</font>

<font color=green>

    int linuxOvercommitMemoryValue(void) {
        FILE *fp = fopen("/proc/sys/vm/overcommit_memory","r");
        char buf[64];

        if (!fp) return -1;
        if (fgets(buf,64,fp) == NULL) {
            fclose(fp);
            return -1;
        }
        fclose(fp);

        return atoi(buf);
    }

</font>

####1.4.2 检查Huge Page ####

<font color=blue>

Transparent Huge Page用户合并物理内存的page

>内核线程khugepaged周期性自动扫描内存，自动将地址连续可以合并的4KB的普通Page并成2MB的Huge Page。
>
>Redhat系统
>
>>通过内核参数/sys/kernel/mm/redhat_transparent_hugepage/enabled打开.
>
>其他Linux系统
>
>>通过内核参数/sys/kernel/mm/transparent_hugepage/enabled打开.

</font>

<font color=green>

    /* Returns 1 if Transparent Huge Pages support is enabled in the kernel.
     * Otherwise (or if we are unable to check) 0 is returned. */
    int THPIsEnabled(void) {
        char buf[1024];

        FILE *fp = fopen("/sys/kernel/mm/transparent_hugepage/enabled","r");
        if (!fp) return 0;
        if (fgets(buf,sizeof(buf),fp) == NULL) {
            fclose(fp);
            return 0;
        }
        fclose(fp);
        return (strstr(buf,"[never]") == NULL) ? 1 : 0;
    }

</font>

####1.4.3 综合流程 ####

<font color=green>

    void linuxMemoryWarnings(void) {
        if (linuxOvercommitMemoryValue() == 0) {
            redisLog(REDIS_WARNING,"WARNING overcommit_memory is set to 0! Background save may fail under low memory condition. To fix this issue add 'vm.overcommit_memory = 1' to /etc/sysctl.conf and then reboot or run the command 'sysctl vm.overcommit_memory=1' for this to take effect.");
        }
        if (THPIsEnabled()) {
            redisLog(REDIS_WARNING,"WARNING you have Transparent Huge Pages (THP) support enabled in your kernel. This will create latency and memory usage issues with Redis. To fix this issue run the command 'echo never > /sys/kernel/mm/transparent_hugepage/enabled' as root, and add it to your /etc/rc.local in order to retain the setting after a reboot. Redis must be restarted after THP is disabled.");
        }
    }

</font>

