# redis sentinel 源码分析 #
---
*written by Alex Stocks on 2015/06/23*


##0 主要数据结构##

<font color=blue>
 >此处首先是sentinel实例，其地位就相当于redis-server模式下的server对象。sentinel实例比较重要的成员就是masters，里面存储了所有的redis主从对的主sri，其key是主sri的name，value就是主sri。
</font>

    struct sentinelState {
        uint64_t current_epoch;   // 当前处在第几个世纪（每次fail over，current_epoch+1）
        dict *masters;            // master实例字典（一个sentinle可监控多个master），key是sri的name，value是sri
        int tilt;                 // 是否在TITL模式中，后面详细介绍TITL模式
        int running_scripts;      // 当前正在执行的脚本
        mstime_t tilt_start_time; // TITL模式开始的时间 */
        mstime_t previous_time;   // 上次执行sentinel周期性执行任务的时间，用以判断是否进入TITL模式
        list *scripts_queue;      // 待执行脚本队列
        char *announce_ip;        // 执行gossip协议的时候标识sentinel的ip
        int announce_port;        // 执行gossip协议的时候标识sentinel的port
    } sentinel;

<font color=blue>
 其次是是sentinel实例，比较重要的成员就是sentinels和slaves，slaves里面存储了所有的redis主从对的从sri，而sentinels则存储了其他sentinel实例的sri，这两个dict的key都是sri的name，name形式是host：port。
</font>

<font color=green>

    typedef struct sentinelRedisInstance {
        int flags;                 // sentinel的身份，查看SRI_... 系列的定义
        char *name;                // sri的名称
        char *runid;               // sri的实例
        uint64_t config_epoch;     // Configuration epoch.
        sentinelAddr *addr;        // Master host.
        redisAsyncContext *cc;     // Hiredis context for commands
        redisAsyncContext *pc;     // Hiredis context for Pub / Sub
        int pending_commands;      // 等待回复的命令的个数
        mstime_t cc_conn_time;     // cc connection time.
        mstime_t pc_conn_time;     // pc connection time.
        mstime_t pc_last_activity; // Last time we received any message.
        mstime_t last_avail_time;  // Last time the instance replied to ping with a reply we consider valid.
        mstime_t last_ping_time;  /* Last time a pending ping was sent in the
                                  context of the current command connection
                                  with the instance. 0 if still not sent or
                                  if pong already received. */
        mstime_t last_pong_time;  /* Last time the instance replied to ping,
                                  whatever the reply was. That's used to check
                                  if the link is idle and must be reconnected. */
        mstime_t last_pub_time;   /* Last time we sent hello via Pub/Sub. */
        mstime_t last_hello_time; /* Only used if SRI_SENTINEL is set. Last time
                                  we received a hello from this Sentinel
                                  via Pub/Sub. */
        mstime_t last_master_down_reply_time; /* Time of last reply to
                                              SENTINEL is-master-down command. */
        mstime_t s_down_since_time; /* Subjectively down since time. */
        mstime_t o_down_since_time; /* Objectively down since time. */
        mstime_t down_after_period; /* Consider it down after that period. */
        mstime_t info_refresh;  /* Time at which we received INFO output from it. */

        /* Role and the first time we observed it.
        * This is useful in order to delay replacing what the instance reports
        * with our own configuration. We need to always wait some time in order
        * to give a chance to the leader to report the new configuration before
        * we do silly things. */
        int role_reported;
        mstime_t role_reported_time;
        mstime_t slave_conf_change_time; /* Last time slave master addr changed. */

        /* Master specific. */
        dict *sentinels;                 // 监控同一个ri master的其他sentinel实例的哈希表
        dict *slaves;                    // ri master 的slave 哈希表
        unsigned int quorum;             // 同意ri master由sdown进入odown状态的最少票数
        int parallel_syncs;              // 同时可以进行多少个failover
        char *auth_pass;                 // auth的password

        /* Slave specific. */
        mstime_t master_link_down_time;  // Slave replication link down time. */
        int slave_priority; /* Slave priority according to its INFO output. */
        mstime_t slave_reconf_sent_time; /* Time at which we sent SLAVE OF <new> */
        struct sentinelRedisInstance *master; // slave sri的master sri
        char *slave_master_host;              // slave ri的host[从info命令获取到]
        int slave_master_port;                // slave ri的port[从info命令获取到]
        int slave_master_link_status;         // slave ri的连接状态
        unsigned long long slave_repl_offset; // slave ri的replication offset

        /* Failover */
        char *leader;       /* If this is a master instance, this is the runid of
                            the Sentinel that should perform the failover. If
                            this is a Sentinel, this is the runid of the Sentinel
                            that this Sentinel voted as leader. */
        uint64_t leader_epoch; /* Epoch of the 'leader' field. */
        uint64_t failover_epoch; /* Epoch of the currently started failover. */
        int failover_state; /* See SENTINEL_FAILOVER_STATE_* defines. */
        mstime_t failover_state_change_time;
        mstime_t failover_start_time;   /* Last failover attempt start time. */
        mstime_t failover_timeout;      /* Max time to refresh failover state. */
        mstime_t failover_delay_logged; /* For what failover_start_time value we
                                        logged the failover delay. */
        struct sentinelRedisInstance *promoted_slave; /* Promoted slave instance. */
        /* Scripts executed to notify admin or reconfigure clients: when they
        * are set to NULL no script is executed. */
        char *notification_script;
        char *client_reconfig_script;
    } sentinelRedisInstance;`

</font>

##1 main##

<font color=blue>

**sentinel启动流程:**

- 1 加载配置；
- 2 初始化redis master、slave以及sentinel的sri；
- 3 注册事件事件serverCron，定时地调用sentinel的逻辑loop函数sentinelTimer。

</font>

<font color=green>

    int main(int argc, char **argv) {
        // sentinel模式判断
        server.sentinel_mode = checkForSentinelMode(argc,argv);

        // sentinel模式下需要完成的初始化工作
        initServerConfig();
        if (server.sentinel_mode) {
            initSentinelConfig();
            initSentinel();
        }

        // 加载配置
        if (argc >= 2) {
            loadServerConfig(configfile,options);
            sdsfree(options);
        }
        // 启动server，加好油门sentinelTimer
        initServer();

        // 检查setinel是否做好了准备，如configfile可写等
        if (!server.sentinel_mode) {
        } else {
            sentinelIsRunning();
        }

        // 开足马力，运转发动机
        aeSetBeforeSleepProc(server.el,beforeSleep);
        aeMain(server.el);
        aeDeleteEventLoop(server.el);

        return 0;
    }

</font>

##2 检查程序是否进入sentinel模式以及sentinel模式下需要完成的初始化工作##

###2.1 检查是否进入sentinel模式 ###

<font color=green>

    // 创建一个server id
    int checkForSentinelMode(int argc, char **argv) {
        int j;

        if (strstr(argv[0],"redis-sentinel") != NULL) return 1;
        for (j = 1; j < argc; j++)
            if (!strcmp(argv[j],"--sentinel")) return 1;
        return 0;
    }

</font>

###2.2 生成runid ###

<font color=blue>

 >Linux中的随机数可以从两个特殊的文件中产生，一个是/dev/urandom.另外一个是/dev/random。他们产生随机数的原理是利用当前系统的熵池来计算出固定一定数量的随机比特，然后将这些比特作为字节流返回。
 >
 >熵池就是当前系统的环境噪音，熵指的是一个系统的混乱程度，系统噪音可以通过很多参数来评估，如内存的使用，文件的使用量，不同类型的进程数量等等。如果当前环境噪音变化的不是很剧烈或者当前环境噪音很小，比如刚开机的时候，而当前需要大量的随机比特，这时产生的随机数的随机效果就不是很好了。这就是为什么会有/dev/urandom和/dev/random这两种不同的文件，后者在不能产生新的随机数时会阻塞程序，而前者不会（ublock），当然产生的随机数效果就不太好了，这对加密解密这样的应用来说就不是一种很好的选择。
 >
 >/dev/random会阻塞当前的程序，直到根据熵池产生新的随机字节之后才返回，所以使用/dev/random比使用/dev/urandom产生大量随机数的速度要慢。
 >
</font>

<font color=green>

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
    // 给server分配一个id
    void initServerConfig(void) {
        // ./redis.h:95:#define REDIS_RUN_ID_SIZE 40
        getRandomHexChars(server.runid,REDIS_RUN_ID_SIZE);
    }

</font>

###2.3 初始化sentinel模式下sentinel对象的相关参数###

<font color=green>

    // 初始化server.port
    void initSentinelConfig(void) {
        // #define REDIS_SENTINEL_PORT 26379
        server.port = REDIS_SENTINEL_PORT;
    }
    // 初始化sentinel对象
    void initSentinel(void) {
        unsigned int j;

        /* Remove usual Redis commands from the command table, then just add
         * the SENTINEL command. */
        // 初始化sentinel的命令字典
        dictEmpty(server.commands,NULL);
        for (j = 0; j < sizeof(sentinelcmds)/sizeof(sentinelcmds[0]); j++) {
            int retval;
            struct redisCommand *cmd = sentinelcmds+j;

            retval = dictAdd(server.commands, sdsnew(cmd->name), cmd);
            redisAssert(retval == DICT_OK);
        }

        /* Initialize various data structures. */
        sentinel.current_epoch = 0;
        // 初始化sentinel的master字典
        sentinel.masters = dictCreate(&instancesDictType,NULL);
        sentinel.tilt = 0;
        sentinel.tilt_start_time = 0;
        sentinel.previous_time = mstime();
        sentinel.running_scripts = 0;
        sentinel.scripts_queue = listCreate();
        sentinel.announce_ip = NULL;
        sentinel.announce_port = 0;
    }

</font>

##3 初始化配置##

###3.1 读取配置文件###

<font color=green>

    // 分析config的每一行，创建sri
    char *sentinelHandleConfiguration(char **argv, int argc) {
        sentinelRedisInstance *ri;

        if (!strcasecmp(argv[0],"monitor") && argc == 5) {
            /* monitor <name> <host> <port> <quorum> */
            int quorum = atoi(argv[4]);

            if (quorum <= 0) return "Quorum must be 1 or greater.";
            // 创建monitor sri
            if (createSentinelRedisInstance(argv[1],SRI_MASTER,argv[2],
                                            atoi(argv[3]),quorum,NULL) == NULL)
            {
                switch(errno) {
                case EBUSY: return "Duplicated master name.";
                case ENOENT: return "Can't resolve master instance hostname.";
                case EINVAL: return "Invalid port number";
                }
            }
        } else if (!strcasecmp(argv[0],"down-after-milliseconds") && argc == 3) {
            /* down-after-milliseconds <name> <milliseconds> */
            // 根据master的name获取sri，然后给其参数down_after_period赋值
            ri = sentinelGetMasterByName(argv[1]);
            if (!ri) return "No such master with specified name.";
            ri->down_after_period = atoi(argv[2]);
            if (ri->down_after_period <= 0)
                return "negative or zero time parameter.";
            sentinelPropagateDownAfterPeriod(ri);
        } else if (!strcasecmp(argv[0],"failover-timeout") && argc == 3) {
            /* failover-timeout <name> <milliseconds> */
            // 根据master的name获取sri，然后给其参数failover-timeout赋值
            ri = sentinelGetMasterByName(argv[1]);
            if (!ri) return "No such master with specified name.";
            ri->failover_timeout = atoi(argv[2]);
            if (ri->failover_timeout <= 0)
                return "negative or zero time parameter.";
       } else if (!strcasecmp(argv[0],"parallel-syncs") && argc == 3) {
            /* parallel-syncs <name> <milliseconds> */
            // 根据master的name获取sri，然后给其参数parallel_syncs赋值
            ri = sentinelGetMasterByName(argv[1]);
            if (!ri) return "No such master with specified name.";
            ri->parallel_syncs = atoi(argv[2]);
       } else if (!strcasecmp(argv[0],"notification-script") && argc == 3) {
            /* notification-script <name> <path> */
            ri = sentinelGetMasterByName(argv[1]);
            if (!ri) return "No such master with specified name.";
            if (access(argv[2],X_OK) == -1)
                return "Notification script seems non existing or non executable.";
            ri->notification_script = sdsnew(argv[2]);
       } else if (!strcasecmp(argv[0],"client-reconfig-script") && argc == 3) {
            /* client-reconfig-script <name> <path> */
            ri = sentinelGetMasterByName(argv[1]);
            if (!ri) return "No such master with specified name.";
            if (access(argv[2],X_OK) == -1)
                return "Client reconfiguration script seems non existing or "
                       "non executable.";
            ri->client_reconfig_script = sdsnew(argv[2]);
       } else if (!strcasecmp(argv[0],"auth-pass") && argc == 3) {
            /* auth-pass <name> <password> */
            // 根据master的name获取sri，然后给其参数auth_pass赋值
            ri = sentinelGetMasterByName(argv[1]);
            if (!ri) return "No such master with specified name.";
            ri->auth_pass = sdsnew(argv[2]);
        } else if (!strcasecmp(argv[0],"current-epoch") && argc == 2) {
            /* current-epoch <epoch> */
            unsigned long long current_epoch = strtoull(argv[1],NULL,10);
            if (current_epoch > sentinel.current_epoch)
                sentinel.current_epoch = current_epoch;
        } else if (!strcasecmp(argv[0],"config-epoch") && argc == 3) {
            /* config-epoch <name> <epoch> */
            ri = sentinelGetMasterByName(argv[1]);
            if (!ri) return "No such master with specified name.";
            ri->config_epoch = strtoull(argv[2],NULL,10);
            /* The following update of current_epoch is not really useful as
             * now the current epoch is persisted on the config file, but
             * we leave this check here for redundancy. */
            if (ri->config_epoch > sentinel.current_epoch)
                sentinel.current_epoch = ri->config_epoch;
        } else if (!strcasecmp(argv[0],"leader-epoch") && argc == 3) {
            /* leader-epoch <name> <epoch> */
            ri = sentinelGetMasterByName(argv[1]);
            if (!ri) return "No such master with specified name.";
            ri->leader_epoch = strtoull(argv[2],NULL,10);
        } else if (!strcasecmp(argv[0],"known-slave") && argc == 4) {
            sentinelRedisInstance *slave;

            /* known-slave <name> <ip> <port> */
            // 给redis slave创建sri
            ri = sentinelGetMasterByName(argv[1]);
            if (!ri) return "No such master with specified name.";
            if ((slave = createSentinelRedisInstance(NULL,SRI_SLAVE,argv[2],
                        atoi(argv[3]), ri->quorum, ri)) == NULL)
            {
                return "Wrong hostname or port for slave.";
            }
        } else if (!strcasecmp(argv[0],"known-sentinel") &&
                   (argc == 4 || argc == 5)) {
            sentinelRedisInstance *si;

            /* known-sentinel <name> <ip> <port> [runid] */
            // 给sentinel创建sri
            ri = sentinelGetMasterByName(argv[1]);
            if (!ri) return "No such master with specified name.";
            if ((si = createSentinelRedisInstance(NULL,SRI_SENTINEL,argv[2],
                        atoi(argv[3]), ri->quorum, ri)) == NULL)
            {
                return "Wrong hostname or port for sentinel.";
            }
            if (argc == 5) si->runid = sdsnew(argv[4]);
        } else if (!strcasecmp(argv[0],"announce-ip") && argc == 2) {
            /* announce-ip <ip-address> */
            if (strlen(argv[1]))
                // 配置sentinel的announce_ip
                sentinel.announce_ip = sdsnew(argv[1]);
        } else if (!strcasecmp(argv[0],"announce-port") && argc == 2) {
            /* announce-port <port> */
            // 配置sentinel的announce_port
            sentinel.announce_port = atoi(argv[1]);
        } else {
            return "Unrecognized sentinel configuration statement.";
        }
        return NULL;
    }

</font>

###3.2 从文件读取所有的字符流###

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

###3.3 对字符流@config逐行拆分###

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
            if (!strcasecmp(argv[0],"sentinel")) {
                /* argc == 1 is handled by main() as we need to enter the sentinel
                 * mode ASAP. */
                if (argc != 1) {
                    if (!server.sentinel_mode) {
                        err = "sentinel directive while not in sentinel mode";
                        goto loaderr;
                    }
                    err = sentinelHandleConfiguration(argv+1,argc-1);
                    if (err) goto loaderr;
                }
            } else {
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

    typedef struct sentinelAddr {
        char *ip;
        int port;
    } sentinelAddr;
    // 创建sentinel的地址
    sentinelAddr *createSentinelAddr(char *hostname, int port) {
        char ip[REDIS_IP_STR_LEN];
        sentinelAddr *sa;

        if (port <= 0 || port > 65535) {
            errno = EINVAL;
            return NULL;
        }
        if (anetResolve(NULL,hostname,ip,sizeof(ip)) == ANET_ERR) {
            errno = ENOENT;
            return NULL;
        }
        sa = zmalloc(sizeof(*sa));
        sa->ip = sdsnew(ip);
        sa->port = port;
        return sa;
    }

</font>

###3.4 创建sri###

<font color=blue>

    // 创建一个sri
    /*
     * 这个函数用于创建一个sentinel instance，用于代表一个sentinel的监控或者联系对象，
     * 联系对象可以是一个redis master\redis slave\redis sentinel，下面两个参数在接到info命令时再赋值:
     * runid: 初始化的时候被赋值为nil；
     * info_refresh: 初始化的时候值为0，表示还没有接到过info命令；
     *
     * 如果flags值为SRI_MASTER，则sri被创建后，将被放在sentinel的sentinel.masters哈希表；
     * 如果flags值为SRI_SLAVE or SRI_SENTINEL，则@name无用，它将用hostname:port作为自己的name,
     *        同时@master一定不能为nil，创建的sri将被放入master->slaves or master->sentinels哈希表;
     *
     * 如果hostname不能被解析或者port溢出，则返回nil并且errno会被置为相关值；
     * 如果master和某个slave的name一样，则返回nil且errno为EBUSY。因为相关的hash表以name作为hash key。
     */

</font>

<font color=green>

    /*
     * 此处根据配置，进行创建相应的sri
     */
    sentinelRedisInstance *createSentinelRedisInstance(char *name, int flags, char *hostname, int port, int quorum, sentinelRedisInstance *master) {
        sentinelRedisInstance *ri;
        sentinelAddr *addr;
        dict *table = NULL;
        char slavename[128], *sdsname;

        redisAssert(flags & (SRI_MASTER | SRI_SLAVE | SRI_SENTINEL));
        redisAssert((flags & SRI_MASTER) || master != NULL);

        /* Check address validity. */
        addr = createSentinelAddr(hostname, port);
        if (addr == NULL) return NULL;

        /* For slaves and sentinel we use ip:port as name. */
        if (flags & (SRI_SLAVE | SRI_SENTINEL)) {
            snprintf(slavename, sizeof(slavename),
                strchr(hostname, ':') ? "[%s]:%d" : "%s:%d",
                hostname, port);
            name = slavename;
        }

        /* Make sure the entry is not duplicated. This may happen when the same
        * name for a master is used multiple times inside the configuration or
        * if we try to add multiple times a slave or sentinel with same ip/port
        * to a master. */
        // sentinel监控的所有的主都在sentinel{masters}里面，而每个master的slave
        // 以及相关的sentinel则在sentinelRedisInstance{slaves, sentinels}里面
        if (flags & SRI_MASTER) table = sentinel.masters;
        else if (flags & SRI_SLAVE) table = master->slaves;
        else if (flags & SRI_SENTINEL) table = master->sentinels;
        sdsname = sdsnew(name);
        if (dictFind(table, sdsname)) {
            releaseSentinelAddr(addr);
            sdsfree(sdsname);
            errno = EBUSY;
            return NULL;
        }

        /* Create the instance object. */
        ri = zmalloc(sizeof(*ri));
        /* Note that all the instances are started in the disconnected state,
        * the event loop will take care of connecting them. */
        ri->flags = flags | SRI_DISCONNECTED;
        ri->name = sdsname;
        ri->runid = NULL;
        ri->config_epoch = 0;
        ri->addr = addr;
        ri->cc = NULL;
        ri->pc = NULL;
        ri->pending_commands = 0;
        ri->cc_conn_time = 0;
        ri->pc_conn_time = 0;
        ri->pc_last_activity = 0;
        /*
         * 初始化的时候，即使我们没有发出一个请求或者没有发送一个PING，我们也要把
         * last_ping_time设置为当前时间。当我们判断对端是否能够联通的情况下很有用
         */
        ri->last_ping_time = mstime();
        ri->last_avail_time = mstime();
        ri->last_pong_time = mstime();
        ri->last_pub_time = mstime();
        ri->last_hello_time = mstime();
        ri->last_master_down_reply_time = mstime();
        ri->s_down_since_time = 0;
        ri->o_down_since_time = 0;
        ri->down_after_period = master ? master->down_after_period :
            SENTINEL_DEFAULT_DOWN_AFTER;
        ri->master_link_down_time = 0;
        ri->auth_pass = NULL;
        ri->slave_priority = SENTINEL_DEFAULT_SLAVE_PRIORITY;
        ri->slave_reconf_sent_time = 0;
        ri->slave_master_host = NULL;
        ri->slave_master_port = 0;
        ri->slave_master_link_status = SENTINEL_MASTER_LINK_STATUS_DOWN;
        ri->slave_repl_offset = 0;
        ri->sentinels = dictCreate(&instancesDictType, NULL);
        ri->quorum = quorum;
        ri->parallel_syncs = SENTINEL_DEFAULT_PARALLEL_SYNCS;
        ri->master = master;
        ri->slaves = dictCreate(&instancesDictType, NULL);
        ri->info_refresh = 0;

        /* Failover state. */
        ri->leader = NULL;
        ri->leader_epoch = 0;
        ri->failover_epoch = 0;
        ri->failover_state = SENTINEL_FAILOVER_STATE_NONE;
        ri->failover_state_change_time = 0;
        ri->failover_start_time = 0;
        ri->failover_timeout = SENTINEL_DEFAULT_FAILOVER_TIMEOUT;
        ri->failover_delay_logged = 0;
        ri->promoted_slave = NULL;
        ri->notification_script = NULL;
        ri->client_reconfig_script = NULL;

        /* Role */
        ri->role_reported = ri->flags & (SRI_MASTER | SRI_SLAVE);
        ri->role_reported_time = mstime();
        ri->slave_conf_change_time = mstime();

        /* Add into the right table. */
        dictAdd(table, ri->name, ri);
        return ri;
    }

</font>

##4 启动server，启动定时函数serverCron##

###4.1 sentinel模式启动流程###

<font color=green>

    // 创建event loop，监听各个端口，并启动定时函数serverCron执行定时任务，定时间隔是1ms
    void initServer(void) {
        int j;

        // 信号处理
        signal(SIGHUP, SIG_IGN);
        signal(SIGPIPE, SIG_IGN);
        setupSignalHandlers();

        if (server.syslog_enabled) {
            openlog(server.syslog_ident, LOG_PID | LOG_NDELAY | LOG_NOWAIT,
                server.syslog_facility);
        }

        // 初始化主要成员
        server.pid = getpid();
        server.monitors = listCreate();

        createSharedObjects();
        adjustOpenFilesLimit();
        // 创建event loop
        server.el = aeCreateEventLoop(server.maxclients+REDIS_EVENTLOOP_FDSET_INCR);
        // 创建root db
        server.db = zmalloc(sizeof(redisDb)*server.dbnum);

        // 监听端口
        if (server.port != 0 &&
            listenToPort(server.port,server.ipfd,&server.ipfd_count) == REDIS_ERR)
            exit(1);

        /* Abort if there are no listening sockets at all. */
        if (server.ipfd_count == 0 && server.sofd < 0) {
            redisLog(REDIS_WARNING, "Configured to not listen anywhere, exiting.");
            exit(1);
        }

        /* Create the Redis databases, and initialize other internal state. */
        // 创建db数组
        for (j = 0; j < server.dbnum; j++) {
            server.db[j].dict = dictCreate(&dbDictType,NULL);
        }
        server.pubsub_channels = dictCreate(&keylistDictType,NULL);
        server.pubsub_patterns = listCreate();
        server.dirty = 0;
        resetServerStats();
        /* A few stats we don't want to reset: server startup time, and peak mem. */
        server.stat_starttime = time(NULL);
        updateCachedTime();

        /* Create the serverCron() time event, that's our main way to process
         * background operations. */
        // 启动定时器
        if(aeCreateTimeEvent(server.el, 1, serverCron, NULL, NULL) == AE_ERR) {
            redisPanic("Can't create the serverCron time event.");
            exit(1);
        }

        /* Create an event handler for accepting new connections in TCP and Unix
         * domain sockets. */
        for (j = 0; j < server.ipfd_count; j++) {
            printf("initServer invoke acceptTcpHandler\n");
            if (aeCreateFileEvent(server.el, server.ipfd[j], AE_READABLE,
                acceptTcpHandler,NULL) == AE_ERR)
                {
                    redisPanic(
                        "Unrecoverable error creating server.ipfd file event.");
                }
        }
    }

</font>

<font color=red>

!!!!注意：上面initServer中注册定时器函数serverCron的时候，时间间隔参数是1ms，但是时间事件处理函数processTimeEvents会修改它的时间间隔，时间间隔是它的返回值：1000/server.hz[约为100ms]。

</font>

<font color=blue>
> main函数初始化流程中被调用initServer，之后执行循环流程函数aeMain，这个函数会循环调用时间事件处理函数processTimeEvents。
</font>

<font color=green>

    /* Process time events */
    static int processTimeEvents(aeEventLoop *eventLoop) {
        int processed = 0;
        aeTimeEvent *te;
        long long maxId;
        time_t now = time(NULL);

        te = eventLoop->timeEventHead;
        maxId = eventLoop->timeEventNextId-1;
        while(te) {
            long now_sec, now_ms;
            long long id;

            if (te->id > maxId) {
                te = te->next;
                continue;
            }
            aeGetTime(&now_sec, &now_ms);
            if (now_sec > te->when_sec ||
                (now_sec == te->when_sec && now_ms >= te->when_ms))
            {
                int retval;

                id = te->id;
                retval = te->timeProc(eventLoop, id, te->clientData);
                processed++;

                if (retval != AE_NOMORE) {  // 根据retval，修改定时任务的重新执行时间@te->when_sec&@te->when_ms
                    aeAddMillisecondsToNow(retval,&te->when_sec,&te->when_ms);
                } else {  // 如果返回值是-1，则删除定时任务
                    aeDeleteTimeEvent(eventLoop, id);
                }
                te = eventLoop->timeEventHead;
            } else {
                te = te->next;
            }
        }
        return processed;
    }

</font>

###4.2 定时函数serverCron流程###

<font color=green>

    /* Return the UNIX time in microseconds */
    long long ustime(void) {
        struct timeval tv;
        long long ust;

        gettimeofday(&tv, NULL);
        ust = ((long long)tv.tv_sec)*1000000;
        ust += tv.tv_usec;
        return ust;
    }

    /* Return the UNIX time in milliseconds */
    long long mstime(void) {
        return ustime()/1000;
    }

    /* We take a cached value of the unix time in the global state because with
     * virtual memory and aging there is to store the current time in objects at
     * every object access, and accuracy is not needed. To access a global var is
     * a lot faster than calling time(NULL) */
    void updateCachedTime(void) {
        server.unixtime = time(NULL);
        server.mstime = mstime();
    }

    /* Using the following macro you can run code inside serverCron() with the
     * specified period, specified in milliseconds.
     * The actual resolution depends on server.hz. */
    // 下面是sentinel默认的执行频率，即一秒内执行10次，period是100ms
    // sentinel实际的频率要比这个高，是为了防止“共振”现象
    #define REDIS_DEFAULT_HZ        10      /* Time interrupt calls/sec. */
    // 下面宏中，(1000/server.hz)为一个动作的period，由于serverCron的执行周期是100ms，所以有了宏的第一个判断条件
    // ((_ms_)/(1000/server.hz)) 则是每多少次执行一次相关的检查，等式等效为((_ms_ * server.hz) / (1000))
    // serverCron函数每100ms执行一次，server.cronloops就相应增加一次，(server.cronloops%((_ms_)/(1000/server.hz))等效于每(_ms_ / 100)个周期执行一次
    //
    // 宏的实际意义是以_ms_为一个period[单位是ms]执行一次
    #define run_with_period(_ms_) if ((_ms_ <= 1000/server.hz) || !(server.cronloops%((_ms_)/(1000/server.hz))))

    /* 这个是redis的定时任务函数，每1ms会被执行一次，一秒内执行server.hz[10]次
     * 下面列出了一些定时任务列表：
     *
     * - 过期kv的收集删除 (lazy模式).
     * - Software watchdog.
     * - 更新统计值
     * - 对所有的db进行rehashing，进行负载均衡
     * - 触发BGSAVE / AOF rewrite, and handling of terminated children.
     * - 删除过期的连接
     * - Replication，主从复制
     * - 其他任务
     *
     * 由于所有的任务都是执行同样的检查频率，为了对检查频率进行控制，
     *  这里定义了一个起到油门作用的宏run_with_period(milliseconds)
     */
    int serverCron(struct aeEventLoop *eventLoop, long long id, void *clientData) {
        int j;
        REDIS_NOTUSED(eventLoop);
        REDIS_NOTUSED(id);
        REDIS_NOTUSED(clientData);

        /* Software watchdog: deliver the SIGALRM that will reach the signal
         * handler if we don't return here fast enough. */
        if (server.watchdog_period) watchdogScheduleSignal(server.watchdog_period);

        /* Update the time cache. */
        updateCachedTime();

        /* Run the Sentinel timer if we are in sentinel mode. */
        // 如果在sentinel模式下，则执行sentinel相关的周期性任务
        run_with_period(100) {
            if (server.sentinel_mode) sentinelTimer();
        }

        server.cronloops++;
        // hz默认值为10，但是sentinel的定时函数sentinelTimer会修改这个值，原因见流程分析7,
        // 参考下面的processTimeEvents，此处返回1000/10 = 100ms会导致serverCron
        // 被定时执行时间由初始的1ms被修改为100ms，即函数的定时处理时长为100ms。
        return 1000/server.hz;
    }

</font>

##5 检查监控条件##

###5.1 检查configfile是否存在以及是否可写###

<font color=green>

    void sentinelIsRunning(void) {
        redisLog(REDIS_WARNING,"Sentinel runid is %s", server.runid);

        if (server.configfile == NULL) {
            // 检查configfile是否存在
            // configfile用于序列化sentinel的监控内容
            redisLog(REDIS_WARNING,
                "Sentinel started without a config file. Exiting...");
            exit(1);
        } else if (access(server.configfile,W_OK) == -1) {
            // 检查configfile是否可写
            redisLog(REDIS_WARNING,
                "Sentinel config file %s is not writable: %s. Exiting...",
                server.configfile,strerror(errno));
            exit(1);
        }

        /* We want to generate a +monitor event for every configured master
         * at startup. */
        sentinelGenerateInitialMonitorEvents();
    }

</font>

###5.2 记录相关事件的内容###

<font color=blue>

相关内容可以记录在log内，也可以通过hello channel发送出去，也可以在执行notification功能的时候把内容作为参数发送出去

</font>

<font color=green>

    /* 记录event log，执行pub/sub, 执行notification脚本
     *
     * 'level'是log level。只有log level是REDIS_WARNING时，才会执行notification脚本
     * 'type'是message type, 也用作pub/sub channel name.
     * 'ri'是sri，可以获取到path of the notification script
     * 'fmt' printf-alike.
     *       如果fmt以"%@"开头并且ri不是NULL，则log内容的开头以如下格式展开：
     *       <instance type> <instance name> <ip> <port>
     *       如果@ri不是master，则后面要补充上mmaster的信息:
     *       @ <master name> <master ip> <master port>
     *       剩余部分的处理跟printf类似
     */
    void sentinelEvent(int level, char *type, sentinelRedisInstance *ri,
                       const char *fmt, ...) {
        va_list ap;
        char msg[REDIS_MAX_LOGMSG_LEN];  // log内容最长1024B
        robj *channel, *payload;

        /* Handle %@ */
        // 拼写msg pre
        if (fmt[0] == '%' && fmt[1] == '@') {
            sentinelRedisInstance *master = (ri->flags & SRI_MASTER) ?
                                             NULL : ri->master;

            if (master) {
                snprintf(msg, sizeof(msg), "%s %s %s %d @ %s %s %d",
                    sentinelRedisInstanceTypeStr(ri),
                    ri->name, ri->addr->ip, ri->addr->port,
                    master->name, master->addr->ip, master->addr->port);
            } else {
                snprintf(msg, sizeof(msg), "%s %s %s %d",
                    sentinelRedisInstanceTypeStr(ri),
                    ri->name, ri->addr->ip, ri->addr->port);
            }
            fmt += 2;
        } else {
            msg[0] = '\0';
        }

        /* Use vsprintf for the rest of the formatting if any. */
        // 拼凑msg内容
        if (fmt[0] != '\0') {
            va_start(ap, fmt);
            vsnprintf(msg+strlen(msg), sizeof(msg)-strlen(msg), fmt, ap);
            va_end(ap);
        }

        /* Log the message if the log level allows it to be logged. */
        // 记录log
        if (level >= server.verbosity)
            redisLog(level,"%s %s",type,msg);

        /* Publish the message via Pub/Sub if it's not a debugging one. */
        // 执行pub/sub，例如发生了failover时候向hello channel发送通知
        if (level != REDIS_DEBUG) {
            channel = createStringObject(type,strlen(type));  // type是channel
            payload = createStringObject(msg,strlen(msg));    // msg内容
            pubsubPublishMessage(channel,payload); // pubsub
            decrRefCount(channel);
            decrRefCount(payload);
        }

        /* Call the notification script if applicable. */
        // 执行notify script，例如发生了failover时候可以执行一些script
        if (level == REDIS_WARNING && ri != NULL) {
            sentinelRedisInstance *master = (ri->flags & SRI_MASTER) ?
                                             ri : ri->master;
            if (master->notification_script) {
                sentinelScheduleScriptExecution(master->notification_script,
                    type,msg,NULL); // notify script
            }
        }
    }

</font>

###5.3 为每个能够确认的master生成一个monitor事件###

<font color=green>

    /* This function is called only at startup and is used to generate a
     * +monitor event for every configured master. The same events are also
     * generated when a master to monitor is added at runtime via the
     * SENTINEL MONITOR command. */
    // 这个函数只会被调用一次，用于为每个master生成一个+monitor事件。如果sentinel
    // 在运行的时候收到SENTINEL MONITOR命令[用于动态添加一个master]，也会生成同样的事件
    void sentinelGenerateInitialMonitorEvents(void) {
        dictIterator *di;
        dictEntry *de;

        di = dictGetIterator(sentinel.masters);
        while((de = dictNext(di)) != NULL) {
            // 获取实例
            sentinelRedisInstance *ri = dictGetVal(de);
            // 检查每个实例并把实例信息log下来
            sentinelEvent(REDIS_WARNING,"+monitor",ri,"%@ quorum %d",ri->quorum);
        }
        dictReleaseIterator(di);
    }

</font>

##6 sentinel与redis实例之间的通信##

<font color = blue>

>
>> 下面的代码块，总体说明连接一个redis实例的时候，会创建cmd和pub/sub两个链接，cmd连接创建成功时候立即发送一个ping命令，pub/sub连接创建成功的时候立即去监听hello channel。
>
>> 通过cmd连接给redis发送命令，通过pub/sub连接得到redis实例上的其他sentinel实例。

**sentinel与maste/slave**的交互主要包括：

- a.PING:sentinel向其发送PING以了解其状态（是否下线）
- b.INFO:sentinel向其发送INFO以获取replication相关的信息，通过这个命令可以获取master的slaves
- c.PUBLISH:sentinel向其监控的master/slave发布本身的信息及master相关的配置
- d.SUBSCRIBE:sentinel通过订阅master/slave的”__sentinel__:hello“频道以获取其它正在监控相同服务的sentinels

**sentinel与sentinel**的交互主要包括：

- a.PING:sentinel向slave发送PING以了解其状态（是否下线）
- b.SENTINEL is-master-down-by-addr：和其他sentinel协商master状态，如果master odown，则投票选出leader做fail over

</font>

###6.1 建立cmd连接###

<font color=blue>

详见7.2.1.2的分析过程。

</font>

###6.2 建立pub/sub连接，并处理连接上hello channel发来的消息###

<font color=green>

    /*
     * 处理Pub/Sub连接上发送来的hello message，如果message中的master名称无法识别，则不处理这条消息
     */
    void sentinelProcessHelloMessage(char *hello, int hello_len) {
        /*
         * hello message的Format，它由8个token组成:
         * 0=ip,1=port,2=runid,3=current_epoch,4=master_name,
         * 5=master_ip,6=master_port,7=master_config_epoch.
         */
        int numtokens, port, removed, master_port;
        uint64_t current_epoch, master_config_epoch;
        char **token = sdssplitlen(hello, hello_len, ",", 1, &numtokens);
        sentinelRedisInstance *si, *master;

        if (numtokens == 8) {
            // 通过master name，获取sri
            master = sentinelGetMasterByName(token[4]);
            if (!master) goto cleanup; /* Unknown master, skip the message. */

            // First, 通过其他sentinel的run id或者host:port，来获取sentinel信息
            port = atoi(token[1]);
            master_port = atoi(token[6]);
            si = getSentinelRedisInstanceByAddrAndRunID(
                            master->sentinels,token[0],port,token[2]);
            current_epoch = strtoull(token[3],NULL,10);
            master_config_epoch = strtoull(token[7],NULL,10);

            if (!si) {
                // 如果没找到，则要删除旧的，添加一个新的进去。旧的sentinel可能重启或者网络的拓扑发生的改变导致其失效了
                removed = removeMatchingSentinelsFromMaster(master,token[0],port,
                                token[2]);
                if (removed) {
                    sentinelEvent(REDIS_NOTICE,"-dup-sentinel",master,
                        "%@ #duplicate of %s:%d or %s",
                        token[0],port,token[2]);
                }

                // 添加新的sentinel实例
                si = createSentinelRedisInstance(NULL,SRI_SENTINEL,
                                token[0],port,master->quorum,master);
                if (si) {
                    sentinelEvent(REDIS_NOTICE,"+sentinel",si,"%@");
                    // 为sri赋runid，其他地方没有机会为它赋值了
                    si->runid = sdsnew(token[2]);
                    sentinelFlushConfig();
                }
            }

            // 更新 current_epoch
            if (current_epoch > sentinel.current_epoch) {
                sentinel.current_epoch = current_epoch;
                sentinelFlushConfig();
                sentinelEvent(REDIS_WARNING,"+new-epoch",master,"%llu",
                    (unsigned long long) sentinel.current_epoch);
            }

            // 更新 config_epoch
            if (master->config_epoch < master_config_epoch) {
                master->config_epoch = master_config_epoch;
                if (master_port != master->addr->port ||
                    strcmp(master->addr->ip, token[5]))
                {
                    sentinelAddr *old_addr;

                    sentinelEvent(REDIS_WARNING,"+config-update-from",si,"%@");
                    sentinelEvent(REDIS_WARNING,"+switch-master",
                        master,"%s %s %d %s %d",
                        master->name,
                        master->addr->ip, master->addr->port,
                        token[5], master_port);

                    old_addr = dupSentinelAddr(master->addr);
                    sentinelResetMasterAndChangeAddress(master, token[5], master_port);
                    sentinelCallClientReconfScript(master,
                        SENTINEL_OBSERVER,"start",
                        old_addr,master->addr);
                    releaseSentinelAddr(old_addr);
                }
            }

            // 更新hello消息时间
            if (si) si->last_hello_time = mstime();
        }

    cleanup:
        sdsfreesplitres(token,numtokens);
    }

    /*
     * 一个 Sentinel 可以与其他多个 Sentinel 进行连接， 各个 Sentinel 之间可以互相检查对方的可用性，
     * 并进行信息交换。你无须为运行的每个 Sentinel 分别设置其他 Sentinel 的地址， 因为 Sentinel 可以
     * 通过发布与订阅功能来自动发现正在监视相同主服务器的其他 Sentinel ， 这一功能是通过向频道
     * __sentinel__:hello 发送信息来实现的。
     *
     * 即在一个master的hello channel上，可以发现这个master的其他sentinel
     */
    void sentinelReceiveHelloMessages(redisAsyncContext *c, void *reply, void *privdata) {
        sentinelRedisInstance *ri = c->data;
        redisReply *r;
        REDIS_NOTUSED(privdata);

        if (!reply || !ri) return;
        r = reply;

        // 更新pc_last_activity
        ri->pc_last_activity = mstime();

        // 检查参数合法性
        if (r->type != REDIS_REPLY_ARRAY ||
            r->elements != 3 ||
            r->element[0]->type != REDIS_REPLY_STRING ||
            r->element[1]->type != REDIS_REPLY_STRING ||
            r->element[2]->type != REDIS_REPLY_STRING ||
            strcmp(r->element[0]->str,"message") != 0) return;

        // 对于runid等于自身的message不处理
        if (strstr(r->element[2]->str,server.runid) != NULL) return;
        // 处理hello message
        sentinelProcessHelloMessage(r->element[2]->str, r->element[2]->len);
    }
</font>

###6.3 sentinel建立链接的函数###

<font color=blue>

sentinelReconnectInstance会被sentinelHandleRedisInstance调用，进而会被sentinelHandleDictOfRedisInstances调用。

在 7.2这节，可以看到sentinel可以与master以及master的slave和master以及slave的有关的sentinel建立通信关系。

</font>

<font color=green>

    /*
     * 向一个redis实例发出两个异步的连接，分别是cmd和pub/sub连接，任何一个不成功，
     * sri的flag就会被置为SRI_DISCONNECTED
     */
    void sentinelReconnectInstance(sentinelRedisInstance *ri) {
        if (!(ri->flags & SRI_DISCONNECTED)) return;

        // 创建cmd连接，用于向redis server发送命令
        if (ri->cc == NULL) {
            ri->cc = redisAsyncConnectBind(ri->addr->ip,ri->addr->port,REDIS_BIND_ADDR);
            if (ri->cc->err) {
                sentinelEvent(REDIS_DEBUG,"-cmd-link-reconnection",ri,"%@ #%s",
                    ri->cc->errstr);
                sentinelKillLink(ri,ri->cc);
            } else {
                ri->cc_conn_time = mstime();
                ri->cc->data = ri;
                redisAeAttach(server.el,ri->cc);
                redisAsyncSetConnectCallback(ri->cc,
                                                sentinelLinkEstablishedCallback);
                redisAsyncSetDisconnectCallback(ri->cc,
                                                sentinelDisconnectCallback);
                sentinelSendAuthIfNeeded(ri,ri->cc);
                sentinelSetClientName(ri,ri->cc,"cmd");  // 为连接取名

                // 重新连接成功，尽快发送PING命令
                sentinelSendPing(ri);
            }
        }

        // 创建Pub / Sub连接，主要channel为"__sentinel__:hello"，获取监控同一instance的其他sentinel
        if ((ri->flags & (SRI_MASTER|SRI_SLAVE)) && ri->pc == NULL) {
            ri->pc = redisAsyncConnectBind(ri->addr->ip,ri->addr->port,REDIS_BIND_ADDR);
            if (ri->pc->err) {
                sentinelEvent(REDIS_DEBUG,"-pubsub-link-reconnection",ri,"%@ #%s",
                    ri->pc->errstr);
                sentinelKillLink(ri,ri->pc);
            } else {
                int retval;

                ri->pc_conn_time = mstime();
                ri->pc->data = ri;
                redisAeAttach(server.el,ri->pc);
                redisAsyncSetConnectCallback(ri->pc,
                                                sentinelLinkEstablishedCallback);
                redisAsyncSetDisconnectCallback(ri->pc,
                                                sentinelDisconnectCallback);
                sentinelSendAuthIfNeeded(ri,ri->pc);
                sentinelSetClientName(ri,ri->pc,"pubsub");
                // subscribe Sentinels "Hello" channel
                retval = redisAsyncCommand(ri->pc,
                    sentinelReceiveHelloMessages, NULL, "SUBSCRIBE %s",
                        SENTINEL_HELLO_CHANNEL);
                if (retval != REDIS_OK) {
                    // subscribe不成功，Pub/Sub连接便没有存在的意义，先把连接关掉，尔后再重连
                    sentinelKillLink(ri,ri->pc);
                    return;
                }
            }
        }

        // 设置连接flag，如果跟redis实例的两个连接都成功，才能说明连接成功，
        // 如果是与sentinel连接，一个cc连接成功就可以了
        if (ri->cc && (ri->flags & SRI_SENTINEL || ri->pc))
            ri->flags &= ~SRI_DISCONNECTED;
    }
</font>

##7 sentinel的定时处理任务##

<font color=blue>

 上面五个流程分析主要说明了sentinel启动过程中的执行步骤，下面分析sentinel启动之后的流程。
</font>

<font color=green>

    void sentinelTimer(void) {
        // 检测是否进入tilt模式
        sentinelCheckTiltCondition();
        // 最重要步骤:执行定期任务，如向master发送ping等命令、检查master状态、与其他sentinel交流
        sentinelHandleDictOfRedisInstances(sentinel.masters);
        // 执行脚本任务
        sentinelRunPendingScripts();
        // 清理执行完毕的script任务
        sentinelCollectTerminatedScripts();
        // 杀死执行时间过长的scripts
        sentinelKillTimedoutScripts();

        /* We continuously change the frequency of the Redis "timer interrupt"
         * in order to desynchronize every Sentinel from every other.
         * This non-determinism avoids that Sentinels started at the same time
         * exactly continue to stay synchronized asking to be voted at the
         * same time again and again (resulting in nobody likely winning the
         * election because of split brain voting). */
        // 修改定时任务执行周期，以防止发生gossip协议的一个缺陷————“共振现象”:所有的sentinel都同时发起投票选举leader导致大家得到的选票都是1
        server.hz = REDIS_DEFAULT_HZ + rand() % REDIS_DEFAULT_HZ;
    }

</font>

###7.1 tilt模式###

<font color=green>

    /* 这个函数用于判断是否进入tilt模式
    *
    * 如果两次sentinelTimer调用的时间间隔太多（SENTINEL_TILT_TRIGGER = 2s）或者
    * 时间发生错乱（当前时间比上次时间小）那么就进入tilt模式。 出现这些现象的可
    * 能原因如下：
    *
    * 1) Sentiel进程由于一些原因被阻塞住了，如load的文件太大、由于一些I/O阻塞
    * 原因os反应速度减缓、进程收到一些信号后stop住等等。
    * 2) 系统的时钟被调整了。
    *
    * 如果不加以处理，那么redis的所有的timer事件都会超时或者处理失败。redis的
    * 处理方式就是进入tilt模式，除了收集系统信息的任务外不再处理任何事物(脑子
    * 清醒一番)，直到tilt模式超时(SENTINEL_TILT_PERIOD = 30s)为止。
    */
    void sentinelCheckTiltCondition(void) {
        mstime_t now = mstime();
        mstime_t delta = now - sentinel.previous_time;

        if (delta < 0 || delta > SENTINEL_TILT_TRIGGER) {
            sentinel.tilt = 1;
            sentinel.tilt_start_time = mstime();
            // 进入tilt模式后，在+tilt channel上发出通知
            sentinelEvent(REDIS_WARNING, "+tilt", NULL, "#tilt mode entered");
        }
        sentinel.previous_time = mstime();
    }
    /*
    *在sentinel的timer处理函数sentinelHandleRedisInstance中，判断tilt模式是否超时
    */
    void sentinelHandleRedisInstance(sentinelRedisInstance *ri) {
        /* ============== ACTING HALF ============= */
        /* We don't proceed with the acting half if we are in TILT mode.
        * TILT happens when we find something odd with the time, like a
        * sudden change in the clock. */
        if (sentinel.tilt) {
            if (mstime() - sentinel.tilt_start_time < SENTINEL_TILT_PERIOD) return;
            sentinel.tilt = 0;
            sentinelEvent(REDIS_WARNING, "-tilt", NULL, "#tilt mode exited");
        }
    }

    在sentinel tilt模式下，下面两个函数的行为受到了影响：
    /*
     *不能进行master是否进入down状态的判断
     */
    void sentinelCommand(redisClient *c) {
        if (!strcasecmp(c->argv[1]->ptr, "is-master-down-by-addr")) {
            /* SENTINEL IS-MASTER-DOWN-BY-ADDR <ip> <port> <current-epoch> <runid>*/
            int isdown = 0;
            ri = getSentinelRedisInstanceByAddrAndRunID(sentinel.masters,
                c->argv[2]->ptr, port, NULL);

            /* It exists? Is actually a master? Is subjectively down? It's down.
             * Note: if we are in tilt mode we always reply with "0". */
            if (!sentinel.tilt && ri && (ri->flags & SRI_S_DOWN) &&
                (ri->flags & SRI_MASTER))
                isdown = 1;
        }
    }

    /*
     * 不能进行master到slave状态的切换
     * 不能进行slave到master状态的切换
     * 不能对slave变换新的master
     * 不能让slave处理客户端发来的config命令
     */
    void sentinelRefreshInstanceInfo(sentinelRedisInstance *ri, const char *info) {
        /* Process line by line. */
        lines = sdssplitlen(info, strlen(info), "\r\n", 2, &numlines);
        for (j = 0; j < numlines; j++) {
            /* None of the following conditions are processed when in tilt mode, so
             * return asap. */
            if (sentinel.tilt) return;

            /* Handle master -> slave role switch. */
            /* Handle slave -> master role switch. */
            /* Handle slaves replicating to a different master address. */
            /* Detect if the slave that is in the process of being reconfigured
             * changed state. */
        }
    }

</font>

###7.2 sentinel定时任务最重要的任务：检查所有的redis instance的状态###

<font color=red>

!!!! redis的定时任务函数是serverCron，sentinel模式下调用调用流程是serverCron->sentinelTimer->sentinelHandleDictOfRedisInstances->sentinelHandleRedisInstance。

</font>

> sentinel 执行failover流程：
>> 1 sentinel发现master下线，修改其状态为sdown；
>
>> 2 sentinel和其他sentinel确认master是否down掉，确认其状态为odown；
>
>> 3 对我们的当前纪元进行自增（详情请参考 Raft leader election ）， 并尝试在这个纪元中当选【即首先发现master down掉的sentinel有优先权当选为leader】；
>
>> 4 如果当选失败，那么在设定的故障迁移超时时间的两倍之后，重新尝试当选。如果当选成功，那么执行以下步骤；
>
>> 5 选出一个从服务器，并将它升级为主服务器；
>
>> 6 leader选出一个slave作为master，发送slaveof no one命令；
>
>> 7 通过发布与订阅功能，将更新后的配置传播给所有其他 Sentinel，其他 Sentinel 对它们自己的配置进行更新；
>
>> 8 并通过给其他slave发送slaveof master命令告知其他slave新的master；
>
>> 9 当所有从服务器都已经开始复制新的主服务器时，领头Sentinel终止这次故障迁移操作。

<font color=green>

    /* Perform scheduled operations for all the instances in the dictionary.
    * Recursively call the function against dictionaries of slaves. */
    // 遍历sentinel.master，检查所有的master以及其slave和sentinel的状态
    void sentinelHandleDictOfRedisInstances(dict *instances) {
        dictIterator *di;
        dictEntry *de;
        sentinelRedisInstance *switch_to_promoted = NULL;

        /* There are a number of things we need to perform against every master. */
        di = dictGetIterator(instances);
        while ((de = dictNext(di)) != NULL) {
            sentinelRedisInstance *ri = dictGetVal(de);

            // 检查sri的状态，执行相关的周期性任务
            sentinelHandleRedisInstance(ri);
            if (ri->flags & SRI_MASTER) {
                // 如果sri是master，还要检查其slave和sentinel的状态
                sentinelHandleDictOfRedisInstances(ri->slaves);
                sentinelHandleDictOfRedisInstances(ri->sentinels);
                // 如果master sri的state为SENTINEL_FAILOVER_STATE_UPDATE_CONFIG，标明要switch的sri
                if (ri->failover_state == SENTINEL_FAILOVER_STATE_UPDATE_CONFIG) {
                    switch_to_promoted = ri;
                }
            }
        }
        // 执行failover任务
        if (switch_to_promoted)
            sentinelFailoverSwitchToPromotedSlave(switch_to_promoted);
        dictReleaseIterator(di);
    }

</font>

<font color=red>

!!!! 这个函数相当于sentinel的main函数，大约每100ms被调用一次。

</font>

<font color=green>

    /* ======================== SENTINEL timer handler ==========================
    * This is the "main" our Sentinel, being sentinel completely non blocking
    * in design. The function is called every second.
    * 上面提到这个是每秒执行一次，经测试不准确，准确的period是100ms
    * -------------------------------------------------------------------------- */

    /* Perform scheduled operations for the specified Redis instance. */
    void sentinelHandleRedisInstance(sentinelRedisInstance *ri) {
        // 如果是初次执行任务[从配置文件读取配置后创建实例的时候并不执行连接动作] 或者 连接偶尔断开时，重连
        sentinelReconnectInstance(ri);
        // 与instance通信交流(PING/INFO/PUBLISH)
        sentinelSendPeriodicCommands(ri);

        // 执行完查询类的动作后，判断是否是否退出tilt模式
        if (sentinel.tilt) {
            // 如果tilt模式持续时间没有结束，就维持这个状态，退出
            if (mstime() - sentinel.tilt_start_time < SENTINEL_TILT_PERIOD) return;
            // 如果tilt模式持续时间结束，就结束这个状态
            sentinel.tilt = 0;
            // 发出通知，说明tilt模式结束
            sentinelEvent(REDIS_WARNING, "-tilt", NULL, "#tilt mode exited");
        }

        /* Every kind of instance */
        // 判断sentinel是否应该进入sdown状态
        sentinelCheckSubjectivelyDown(ri);

        /* Masters and slaves */
        if (ri->flags & (SRI_MASTER | SRI_SLAVE)) {
            /* Nothing so far. */
        }

        /* Only masters */
        // 下面的逻辑仅仅对master sri有效
        if (ri->flags & SRI_MASTER) {
            // 检查ri是否进入odown状态
            sentinelCheckObjectivelyDown(ri);
            // 判断是否需要开始执行failover动作
            // 下面这个函数调用的sentinelStartFailover中，
            // ri会增大自己的current_epoch，以使得自己当选为sentinel leader
            if (sentinelStartFailoverIfNeeded(ri))
                // 与其他sentinel协商这个实例的状态，判断其是否应该进入odown状态
                sentinelAskMasterStateToOtherSentinels(ri, SENTINEL_ASK_FORCED);
            // 对ri执行failover[故障替换]
            sentinelFailoverStateMachine(ri);
            sentinelAskMasterStateToOtherSentinels(ri, SENTINEL_NO_FLAGS);
        }
    }

</font>

####7.2.1 周期性任务：发送info命令，分析sri的状态####

<font color=green>

    // 通过Hello channel向特定的redis instance发送PING, INFO, and PUBLISH命令
    void sentinelSendPeriodicCommands(sentinelRedisInstance *ri) {
        mstime_t now = mstime();
        mstime_t info_period, ping_period;
        int retval;

        // 链接中断，则终止函数
        if (ri->flags & SRI_DISCONNECTED) return;

        // 如果有待回复的命令超过了100个，则返回
        // 超过这么多的命令没有被执行，只能说明链接已经不可用，有待重连，
        // 如果重连成功则可以基于新的连接得到回复
        if (ri->pending_commands >= SENTINEL_MAX_PENDING_COMMANDS) return;

        // 如果sri是一个slave，其master正处于odown或者failover_in_progress，则发送info
        // 周期是1s，否则就是10s
        if ((ri->flags & SRI_SLAVE) &&
            (ri->master->flags & (SRI_O_DOWN|SRI_FAILOVER_IN_PROGRESS))) {
            info_period = 1000;
        } else {
            info_period = SENTINEL_INFO_PERIOD;
        }
        // 计算ping命令发送时间间隔，最大不得超过1s
        ping_period = ri->down_after_period;
        if (ping_period > SENTINEL_PING_PERIOD) ping_period = SENTINEL_PING_PERIOD;
        // 因为下面的三种功能的周期是秒级，而sentinelTimer的执行频率是100ms，所以1s内三个分支肯定都能走到，所以这里if else大致无错，即没个周期[100ms]内只用执行一个功能
        //
        // 连接对象不是sentinel，而且没有发送过info命令或者距离上次发送info命令已经过期，就需要重新发送info命令
        // info_refresh在createSentinelRedisInstance函数里初始化值为0，
        // 在info命令的回复处理函数sentinelRefreshInstanceInfo里会被赋值为当前系统时间
        if ((ri->flags & SRI_SENTINEL) == 0 &&
            (ri->info_refresh == 0 ||
            (now - ri->info_refresh) > info_period))
        {
            /* Send INFO to masters and slaves, not sentinels. */
            // 每隔一秒向master/slave实例发送ping命令
            // info命令的结果用于更新sri的当前状态
            retval = redisAsyncCommand(ri->cc,
                sentinelInfoReplyCallback, NULL, "INFO");
            if (retval == REDIS_OK) ri->pending_commands++;
        } else if ((now - ri->last_pong_time) > ping_period) {
            // 每隔一秒向三种实例发送ping命令
            // info命令的结果用于更新sri的当前状态
            sentinelSendPing(ri);
        } else if ((now - ri->last_pub_time) > SENTINEL_PUBLISH_PERIOD) {
            // 每隔两秒向master/slave的hello channel发送消息，用于确认自己
            // 对master的状态的记载，这样每个sentinel判断master是否由sdown
            // 进入odown状态，只用检查收到的其他sentinel publish来的这个结
            // 果就可以了，能够快速地对master进行failover
            sentinelSendHello(ri);
        }
    }

</font>

#####7.2.1.1 分析info命令的返回结果，添加sri的从或者sentinel、修改sri的状态、获取replication结果etc#####


info命令查询sentinel的结果示例：

<font color=blue>

    # Server
    run_id:5bed921a1ecf7f7587bf818f3e00177e7cd11614
    tcp_port:26380
    hz:12

    # Sentinel
    sentinel_masters:2
    sentinel_tilt:0
    sentinel_running_scripts:0
    sentinel_scripts_queue_length:0
    master0:name=server0,status=ok,address=127.0.0.1:6379,slaves=1,sentinels=2
    master1:name=bae-meta1,status=ok,address=127.0.0.1:6000,slaves=1,sentinels=2

</font>

info命令查询master的结果示例：

<font color=blue>

    # Server
    run_id:f099fe413ebca4d835d9a931f4557c262db896e5
    tcp_port:6000
    hz:10

    # Replication
    role:master
    connected_slaves:1
    slave0:ip=127.0.0.1,port=6001,state=online,offset=115003206,lag=1

</font>

info命令查询slave的结果示例：

<font color=blue>

    # Server
    run_id:ad2481c9ad65946ce18385998c2bdde314b1269c
    tcp_port:6001
    hz:10

    # Replication
    role:slave
    master_host:127.0.0.1
    master_port:6000
    master_link_status:up
    slave_repl_offset:115018650
    slave_priority:100
    connected_slaves:0

</font>

<font color=green>

    // info命令的回调函数
    void sentinelInfoReplyCallback(redisAsyncContext *c, void *reply, void *privdata) {
        sentinelRedisInstance *ri = c->data;
        redisReply *r;
        REDIS_NOTUSED(privdata);

        if (ri) ri->pending_commands--; // 得到响应，减记
        if (!reply || !ri) return;
        r = reply;

        // 命令的type正确，则予以处理
        if (r->type == REDIS_REPLY_STRING) {
            sentinelRefreshInstanceInfo(ri,r->str);
        }
    }

    // 处理master对info命令的回复的结果
    void sentinelRefreshInstanceInfo(sentinelRedisInstance *ri, const char *info) {
        sds *lines;
        int numlines, j;
        int role = 0;

        /* The following fields must be reset to a given value in the case they
         * are not found at all in the INFO output. */
        ri->master_link_down_time = 0;

        // 对info的结果以"\r\n"作为delimeter进行拆分后，逐行分析之
        lines = sdssplitlen(info,strlen(info),"\r\n",2,&numlines);
        for (j = 0; j < numlines; j++) {
            sentinelRedisInstance *slave;
            sds l = lines[j];

            /* run_id:<40 hex chars>*/
            // 分析runid
            // run_id:5bed921a1ecf7f7587bf818f3e00177e7cd11614
            if (sdslen(l) >= 47 && !memcmp(l,"run_id:",7)) {
                if (ri->runid == NULL) {
                    ri->runid = sdsnewlen(l+7,40); // 如果现在的runid不存在，则拷贝info结果里面的
                } else {
                    if (strncmp(ri->runid,l+7,40) != 0) {
                        // 如果runid不一致，则释放原有的，拷贝info结果里面的
                        sentinelEvent(REDIS_NOTICE,"+reboot",ri,"%@");
                        sdsfree(ri->runid);
                        ri->runid = sdsnewlen(l+7,40);
                    }
                }
            }

            /* old versions: slave0:<ip>,<port>,<state>
             * new versions: slave0:ip=127.0.0.1,port=9999,... */
            // 分析slave
            // slave0:ip=127.0.0.1,port=6001,state=online,offset=115003206,lag=1
            if ((ri->flags & SRI_MASTER) &&
                sdslen(l) >= 7 &&
                !memcmp(l,"slave",5) && isdigit(l[5]))
            {
                char *ip, *port, *end;

                if (strstr(l,"ip=") == NULL) {
                    /* Old format. */
                    ip = strchr(l,':'); if (!ip) continue;
                    ip++; /* Now ip points to start of ip address. */
                    port = strchr(ip,','); if (!port) continue;
                    *port = '\0'; /* nul term for easy access. */
                    port++; /* Now port points to start of port number. */
                    end = strchr(port,','); if (!end) continue;
                    *end = '\0'; /* nul term for easy access. */
                } else {
                    /* New format. */
                    ip = strstr(l,"ip="); if (!ip) continue;
                    ip += 3; /* Now ip points to start of ip address. */
                    port = strstr(l,"port="); if (!port) continue;
                    port += 5; /* Now port points to start of port number. */
                    /* Nul term both fields for easy access. */
                    end = strchr(ip,','); if (end) *end = '\0';
                    end = strchr(port,','); if (end) *end = '\0';
                }

                // 如果slave不在ri的slave dict里面，把结果里面的slave放入@ri的ri->slaves中
                if (sentinelRedisInstanceLookupSlave(ri,ip,atoi(port)) == NULL) {
                    if ((slave = createSentinelRedisInstance(NULL,SRI_SLAVE,ip,
                                atoi(port), ri->quorum, ri)) != NULL)
                    {
                        sentinelEvent(REDIS_NOTICE,"+slave",slave,"%@");
                        sentinelFlushConfig();
                    }
                }
            }

            /* master_link_down_since_seconds:<seconds> */
            // 分析 master_link_down_since_seconds
            if (sdslen(l) >= 32 &&
                !memcmp(l,"master_link_down_since_seconds",30))
            {
                ri->master_link_down_time = strtoll(l+31,NULL,10)*1000;
            }

            /* role:<role> */
            // 分析replication部分的 role字段
            // role:master
            // role:slave
            if (!memcmp(l,"role:master",11)) role = SRI_MASTER;
            else if (!memcmp(l,"role:slave",10)) role = SRI_SLAVE;

            if (role == SRI_SLAVE) {
                /* master_host:<host> */
                // master_host:127.0.0.1
                if (sdslen(l) >= 12 && !memcmp(l,"master_host:",12)) {
                    if (ri->slave_master_host == NULL ||
                        strcasecmp(l+12,ri->slave_master_host))
                    {
                        sdsfree(ri->slave_master_host);
                        ri->slave_master_host = sdsnew(l+12);
                        ri->slave_conf_change_time = mstime();
                    }
                }

                /* master_port:<port> */
                // master_port:6000
                if (sdslen(l) >= 12 && !memcmp(l,"master_port:",12)) {
                    int slave_master_port = atoi(l+12);

                    if (ri->slave_master_port != slave_master_port) {
                        ri->slave_master_port = slave_master_port;
                        ri->slave_conf_change_time = mstime();
                    }
                }

                /* master_link_status:<status> */
                // master_link_status:up
                if (sdslen(l) >= 19 && !memcmp(l,"master_link_status:",19)) {
                    ri->slave_master_link_status =
                        (strcasecmp(l+19,"up") == 0) ?
                        SENTINEL_MASTER_LINK_STATUS_UP :
                        SENTINEL_MASTER_LINK_STATUS_DOWN;
                }

                /* slave_priority:<priority> */
                // slave_priority:100
                if (sdslen(l) >= 15 && !memcmp(l,"slave_priority:",15))
                    ri->slave_priority = atoi(l+15);

                /* slave_repl_offset:<offset> */
                // slave_repl_offset:115018650
                if (sdslen(l) >= 18 && !memcmp(l,"slave_repl_offset:",18))
                    ri->slave_repl_offset = strtoull(l+18,NULL,10);
            }
        }
        ri->info_refresh = mstime();
        sdsfreesplitres(lines,numlines);

        /* ---------------------------- Acting half -----------------------------
         * Some things will not happen if sentinel.tilt is true, but some will
         * still be processed. */
        // 如果sri的role发生了改变，则要发出通知
        if (role != ri->role_reported) {
            ri->role_reported_time = mstime();
            ri->role_reported = role;
            if (role == SRI_SLAVE) ri->slave_conf_change_time = mstime();
            /* Log the event with +role-change if the new role is coherent or
             * with -role-change if there is a mismatch with the current config. */
            sentinelEvent(REDIS_VERBOSE,
                ((ri->flags & (SRI_MASTER|SRI_SLAVE)) == role) ?
                "+role-change" : "-role-change",
                ri, "%@ new reported role is %s",
                role == SRI_MASTER ? "master" : "slave",
                ri->flags & SRI_MASTER ? "master" : "slave");
        }
    }

</font>

#####7.2.1.2 分析ping命令的返回结果#####

<font color = green>

    /*
     * 连接某个host ip:port, 创建连接context；
     * 创建异步连接context；
     */
    redisAsyncContext *redisAsyncConnectBind(const char *ip, int port,
                                             const char *source_addr) {
        redisContext *c = redisConnectBindNonBlock(ip,port,source_addr);
        redisAsyncContext *ac = redisAsyncInitialize(c);
        __redisAsyncCopyError(ac);
        return ac;
    }

    /*
     * 指定连接的名称，格式是：sentinel-<sentinel的runid的前8个字符>-<connection_type>
     * connection_type是cmd or pubsub，然后把这个名称通过CLIENT SETNAME name发送给对端redis实例；
     *
     * 然后在redis instance端就可以通过CLIENT LIST列出所有的sentinel实例
     */
    void sentinelSetClientName(sentinelRedisInstance *ri, redisAsyncContext *c, char *type) {
        char name[64];

        snprintf(name,sizeof(name),"sentinel-%.8s-%s",server.runid,type);
        if (redisAsyncCommand(c, sentinelDiscardReplyCallback, NULL,
            "CLIENT SETNAME %s", name) == REDIS_OK)
        {
            ri->pending_commands++;
        }
    }

    /*
     * 向某个redis实例发送ping命令，并更新last_ping_time
     * last_ping_time如果为0，则说明上一个ping命令的响应pong已经收到
     * 出错就返回0，我们并不能想当然的认为PING命令还在对端的请求队列中
     */
    int sentinelSendPing(sentinelRedisInstance *ri) {
        int retval = redisAsyncCommand(ri->cc,
            sentinelPingReplyCallback, NULL, "PING");
        if (retval == REDIS_OK) {
            ri->pending_commands++;
            /*
             * 如果last_ping_time为0，说明上一个ping命令已经得到pong响应，可以更新之
             */
            if (ri->last_ping_time == 0) ri->last_ping_time = mstime();
            return 1;
        } else {
            return 0;
        }
    }
    /*
     * ping命令的回调函数
     */
    void sentinelPingReplyCallback(redisAsyncContext *c, void *reply, void *privdata) {
        sentinelRedisInstance *ri = c->data;
        redisReply *r;
        REDIS_NOTUSED(privdata);

        if (ri) ri->pending_commands--; // 得到响应，故而减记
        if (!reply || !ri) return;
        r = reply;

        if (r->type == REDIS_REPLY_STATUS ||
            r->type == REDIS_REPLY_ERROR) {
            /* Update the "instance available" field only if this is an
             * acceptable reply. */
            if (strncmp(r->str,"PONG",4) == 0 ||
                strncmp(r->str,"LOADING",7) == 0 ||
                strncmp(r->str,"MASTERDOWN",10) == 0)
            {
                ri->last_avail_time = mstime();
                ri->last_ping_time = 0; // pong flag，为0说明已经收到了pong响应
            } else {
                /*
                 * 向redis实例发送SCRIPT KILL命令，因为redis实例已经卡住
                 */
                if (strncmp(r->str,"BUSY",4) == 0 &&
                    (ri->flags & SRI_S_DOWN) &&
                    !(ri->flags & SRI_SCRIPT_KILL_SENT))
                {
                    if (redisAsyncCommand(ri->cc,
                            sentinelDiscardReplyCallback, NULL,
                            "SCRIPT KILL") == REDIS_OK)
                        ri->pending_commands++;
                    ri->flags |= SRI_SCRIPT_KILL_SENT;
                }
            }
        }
        ri->last_pong_time = mstime();
    }

</font>
#####7.2.1.3 向master/slave上的其他sentinel发送hello消息#####

<font color=green>

    /*
     * 通过Pub/Sub连接发送“Hello”消息，与其他sentinel交流信息，说明自己记录的master状态
     * 消息的格式如下:
     *
     * sentinel_ip,sentinel_port,sentinel_runid,current_epoch,
     * master_name,master_ip,master_port,master_config_epoch.
     *
     * PUBLISH命令发送成功，就返回REDIS_OK，否则返回REDIS_ERR
     */
    int sentinelSendHello(sentinelRedisInstance *ri) {
        char ip[REDIS_IP_STR_LEN];
        char payload[REDIS_IP_STR_LEN+1024];
        int retval;
        char *announce_ip;
        int announce_port;
        sentinelRedisInstance *master = (ri->flags & SRI_MASTER) ? ri : ri->master;
        sentinelAddr *master_addr = sentinelGetCurrentMasterAddress(master);

        if (ri->flags & SRI_DISCONNECTED) return REDIS_ERR;

        /* Use the specified announce address if specified, otherwise try to
         * obtain our own IP address. */
        if (sentinel.announce_ip) {
            announce_ip = sentinel.announce_ip;
        } else {
            if (anetSockName(ri->cc->c.fd,ip,sizeof(ip),NULL) == -1)
                return REDIS_ERR;
            announce_ip = ip;
        }
        announce_port = sentinel.announce_port ?
                        sentinel.announce_port : server.port;

        /* Format and send the Hello message. */
        snprintf(payload,sizeof(payload),
            "%s,%d,%s,%llu," /* Info about this sentinel. */  // 当前的sentinel
            "%s,%s,%d,%llu", /* Info about current master. */ // 其监控的master
            announce_ip, announce_port, server.runid,
            (unsigned long long) sentinel.current_epoch,
            /* --- */
            master->name,master_addr->ip,master_addr->port,
            (unsigned long long) master->config_epoch);
        retval = redisAsyncCommand(ri->cc,
            sentinelPublishReplyCallback, NULL, "PUBLISH %s %s",
                SENTINEL_HELLO_CHANNEL,payload);
        if (retval != REDIS_OK) return REDIS_ERR;
        ri->pending_commands++;
        return REDIS_OK;
    }

</font>

####7.2.2 判断master是否进入sdown状态####

<font color=green>
    // 是否进入sdown的主要依据就是超过down_after_period没有收到的ping命令的响应
    void sentinelCheckSubjectivelyDown(sentinelRedisInstance *ri) {
        mstime_t elapsed = 0;

        if (ri->last_ping_time)
            elapsed = mstime() - ri->last_ping_time;

        /* Check if we are in need for a reconnection of one of the
         * links, because we are detecting low activity.
         *
         * 1) Check if the command link seems connected, was connected not less
         *    than SENTINEL_MIN_LINK_RECONNECT_PERIOD, but still we have a
         *    pending ping for more than half the timeout. */
        if (ri->cc &&
            (mstime() - ri->cc_conn_time) > SENTINEL_MIN_LINK_RECONNECT_PERIOD &&
            ri->last_ping_time != 0 && /* Ther is a pending ping... */
            /* The pending ping is delayed, and we did not received
             * error replies as well. */
            (mstime() - ri->last_ping_time) > (ri->down_after_period/2) &&
            (mstime() - ri->last_pong_time) > (ri->down_after_period/2))
        {
            sentinelKillLink(ri,ri->cc);
        }

        /* 2) Check if the pubsub link seems connected, was connected not less
         *    than SENTINEL_MIN_LINK_RECONNECT_PERIOD, but still we have no
         *    activity in the Pub/Sub channel for more than
         *    SENTINEL_PUBLISH_PERIOD * 3.
         */
        if (ri->pc &&
            (mstime() - ri->pc_conn_time) > SENTINEL_MIN_LINK_RECONNECT_PERIOD &&
            (mstime() - ri->pc_last_activity) > (SENTINEL_PUBLISH_PERIOD*3))
        {
            sentinelKillLink(ri,ri->pc);
        }

        // 满足如下两个条件，可以认为master进入sdown状态:
        // 1 超过down_after_period这个时间段，没有收到回复
        // 2 sentinel认为它是一个master，距离它自己上报自己是slave的时间超过了
        //     (down_after_period + 两个回复info命令的周期)
        if (elapsed > ri->down_after_period ||
            (ri->flags & SRI_MASTER &&
             ri->role_reported == SRI_SLAVE &&
             mstime() - ri->role_reported_time >
              (ri->down_after_period+SENTINEL_INFO_PERIOD*2)))
        {
            // 设置sdown flag
            if ((ri->flags & SRI_S_DOWN) == 0) {
                sentinelEvent(REDIS_WARNING,"+sdown",ri,"%@");
                ri->s_down_since_time = mstime();
                ri->flags |= SRI_S_DOWN;
            }
        } else {
            // 如果不满足sdown条件，就去掉sdown flag
            if (ri->flags & SRI_S_DOWN) {
                sentinelEvent(REDIS_WARNING,"-sdown",ri,"%@");
                ri->flags &= ~(SRI_S_DOWN|SRI_SCRIPT_KILL_SENT);
            }
        }
    }

</font>

####7.2.3 判断master是否进入odown状态####

<font color=green>

    /* Is this instance down according to the configured quorum?
     *
     * Note that ODOWN is a weak quorum, it only means that enough Sentinels
     * reported in a given time range that the instance was not reachable.
     * However messages can be delayed so there are no strong guarantees about
     * N instances agreeing at the same time about the down state. */
    // 上面的意思是odown是很难让所有sentinel打成一致的状态，它要求大多数sentinel
    // 都在同一个period内判断这个master已经unreachable。再考虑到消息延迟性，这个
    // 多数票的要求是很难得到满足的。
    void sentinelCheckObjectivelyDown(sentinelRedisInstance *master) {
        dictIterator *di;
        dictEntry *de;
        unsigned int quorum = 0, odown = 0;

        // 首先master的状态为sdown
        if (master->flags & SRI_S_DOWN) {
            // 首先，sdown表明当前sentinel已经投了一票
            quorum = 1;
            // 计算票数
            di = dictGetIterator(master->sentinels);
            while((de = dictNext(di)) != NULL) {
                sentinelRedisInstance *ri = dictGetVal(de);

                if (ri->flags & SRI_MASTER_DOWN) quorum++;
            }
            dictReleaseIterator(di);
            // 超过法定票数，则可以认为其状态为odown
            if (quorum >= master->quorum) odown = 1;
        }

        // 根据票数统计结果，修改master的flag，并计算odown起始时间
        if (odown) {
            if ((master->flags & SRI_O_DOWN) == 0) {
                sentinelEvent(REDIS_WARNING,"+odown",master,"%@ #quorum %d/%d",
                    quorum, master->quorum);
                master->flags |= SRI_O_DOWN;
                master->o_down_since_time = mstime();
            }
        } else {
            if (master->flags & SRI_O_DOWN) {
                sentinelEvent(REDIS_WARNING,"-odown",master,"%@");
                master->flags &= ~SRI_O_DOWN;
            }
        }
    }

</font>

####7.2.4 判断master是否需要进行failover####

<font color=green>

    /* This function checks if there are the conditions to start the failover,
     * that is:
     *
     * 1) Master must be in ODOWN condition.
     * 2) No failover already in progress.
     * 3) No failover already attempted recently.
     *
     * We still don't know if we'll win the election so it is possible that we
     * start the failover but that we'll not be able to act.
     *
     * Return non-zero if a failover was started. */
    int sentinelStartFailoverIfNeeded(sentinelRedisInstance *master) {
        // 如果state不为odown，则退出
        if (!(master->flags & SRI_O_DOWN)) return 0;
        // 如果已经开始执行failover动作，则退出
        if (master->flags & SRI_FAILOVER_IN_PROGRESS) return 0;
        // 确认failover动作周期还没有超时，没有超时则退出【在这个时间内进行投票操作与failover动作】
        if (mstime() - master->failover_start_time <
            master->failover_timeout*2)
        {
            if (master->failover_delay_logged != master->failover_start_time) {
                time_t clock = (master->failover_start_time +
                                master->failover_timeout*2) / 1000;
                char ctimebuf[26];

                ctime_r(&clock,ctimebuf);
                ctimebuf[24] = '\0'; /* Remove newline. */
                master->failover_delay_logged = master->failover_start_time;
                redisLog(REDIS_WARNING,
                    "Next failover delay: I will not start a failover before %s",
                    ctimebuf);
            }
            return 0;
        }

        // 开始准备执行failover动作, 修改sri的相关参数
        sentinelStartFailover(master);
        return 1;
    }

    /* Setup the master state to start a failover. */
    void sentinelStartFailover(sentinelRedisInstance *master) {
        // 确认sri的角色
        redisAssert(master->flags & SRI_MASTER);

        // 修改failover状态
        master->failover_state = SENTINEL_FAILOVER_STATE_WAIT_START;
        master->flags |= SRI_FAILOVER_IN_PROGRESS;
        // 修改epoch
        master->failover_epoch = ++sentinel.current_epoch; // 此处增大自身的epoch，以增大当选为sentinel leader的可能性
        sentinelEvent(REDIS_WARNING,"+new-epoch",master,"%llu",
            (unsigned long long) sentinel.current_epoch);
        sentinelEvent(REDIS_WARNING,"+try-failover",master,"%@");
        // 设置failover开始时间
        master->failover_start_time = mstime()+rand()%SENTINEL_MAX_DESYNC;
        master->failover_state_change_time = mstime();
    }

</font>

####7.2.5 sentinel确认master的odown状态信息后，广播master的地址和状态给其他sentinel，让其他sentinel进行投票####

 如果master->failover_state_change_time处于SENTINEL_FAILOVER_STATE_NONE时，才会要求进行投票，其标志是发出请求的@runid不为'*'

#####7.2.5.1 发出投票通知#####

<font color=red>

!!!!此处一个问题就是：发现一个master down掉的sentinel发出投票请求的时候，接收者依据epoch进行判断后并返回它认为的leader后，发现者对接收者的意见照单全收，并没有进行“反驳”。换句话说，接收者接收所有的接收者的意见后，并没有进行一番比较，就接收了每个接收者的认为的leader，结果其leader就是最后一个接收者认为的leader。

但是从另一方面说，每个接收者都会收到所有潜在的leader的选举邀请，他们会对所有这些候选者进行比较一番后，本地的leader肯定是最后比较的结果，这个结果会通过反馈给每个候选者的通知，使他们知道最终的胜出者，所以还是能选举出最后的leader。

</font>

<font color=green>

    /* If we think the master is down, we start sending
     * SENTINEL IS-MASTER-DOWN-BY-ADDR requests to other sentinels
     * in order to get the replies that allow to reach the quorum
     * needed to mark the master in ODOWN state and trigger a failover. */
    // 如果当前sentinel认为master已经down掉了，则把master信息告知其他sentinel，让他们进行投票
    #define SENTINEL_ASK_FORCED (1<<0)
    void sentinelAskMasterStateToOtherSentinels(sentinelRedisInstance *master, int flags) {
        dictIterator *di;
        dictEntry *de;

        // 遍历master的表
        di = dictGetIterator(master->sentinels);
        while((de = dictNext(di)) != NULL) {
            sentinelRedisInstance *ri = dictGetVal(de);
            mstime_t elapsed = mstime() - ri->last_master_down_reply_time;
            char port[32];
            int retval;

            /* If the master state from other sentinel is too old, we clear it. */
            // 如果其他sentinel标识的master的状态太旧，则不予以采信
            if (elapsed > SENTINEL_ASK_PERIOD*5) {
                ri->flags &= ~SRI_MASTER_DOWN;
                sdsfree(ri->leader);
                ri->leader = NULL;
            }

            /* Only ask if master is down to other sentinels if:
             *
             * 1) We believe it is down, or there is a failover in progress.
             * 2) Sentinel is connected.
             * 3) We did not received the info within SENTINEL_ASK_PERIOD ms. */
            // 只有满足下面三个条件时，才向其它sentinel发送通知：
            // 1 自己认为它已经odown，或者处于确认failover的过程中。
            // 2 sentinel处于可连接的状态。
            // 3 在强迫发送广播的flag为false条件下，上次发送ask命令有效期[SENTINEL_ASK_PERIOD]内还未收到回复
            if ((master->flags & SRI_S_DOWN) == 0) continue;
            if (ri->flags & SRI_DISCONNECTED) continue;
            if (!(flags & SENTINEL_ASK_FORCED) &&
                mstime() - ri->last_master_down_reply_time < SENTINEL_ASK_PERIOD)
                continue;

            /* Ask */
            ll2string(port,sizeof(port),master->addr->port);
            // 告知其他sentinel：SENTINEL is-master-down-by-addr master_ip master_port current_epoch runid
            // 如果已经开始failover则runid用*代替，否则发送runid
            //
            // 回调函数是sentinelReceiveIsMasterDownReply
            retval = redisAsyncCommand(ri->cc,
                        sentinelReceiveIsMasterDownReply, NULL,
                        "SENTINEL is-master-down-by-addr %s %s %llu %s",
                        master->addr->ip, port,
                        sentinel.current_epoch,
                        (master->failover_state > SENTINEL_FAILOVER_STATE_NONE) ?
                        server.runid : "*");
            if (retval == REDIS_OK) ri->pending_commands++;
        }
        dictReleaseIterator(di);
    }

</font>

#####7.2.5.2 处理其他sentinel发来的投票通知#####

<font color=red>

!!!! 全篇几乎都是在分析sentinel的时间事件处理函数sentinelTimer，不要忘了sentinel也要其他sentinel发来的网络请求，主要是sentinel命令，其处理函数就是sentinelCommand。

</font>

<font color=green>

    void sentinelCommand(redisClient *c) {
        if (!strcasecmp(c->argv[1]->ptr,"is-master-down-by-addr")) {
            /* SENTINEL IS-MASTER-DOWN-BY-ADDR <ip> <port> <current-epoch> <runid>*/
            sentinelRedisInstance *ri;
            long long req_epoch;
            uint64_t leader_epoch = 0;
            char *leader = NULL;
            long port;
            int isdown = 0;

            if (c->argc != 6) goto numargserr;
            if (getLongFromObjectOrReply(c,c->argv[3],&port,NULL) != REDIS_OK ||
                getLongLongFromObjectOrReply(c,c->argv[4],&req_epoch,NULL)
                                                                  != REDIS_OK)
                return;
            // 根据@runid，获取本地存储的对应的sri
            ri = getSentinelRedisInstanceByAddrAndRunID(sentinel.masters,
                c->argv[2]->ptr,port,NULL);

            /* It exists? Is actually a master? Is subjectively down? It's down.
             * Note: if we are in tilt mode we always reply with "0". */
            // 确认master instance确实进入down掉了
            if (!sentinel.tilt && ri && (ri->flags & SRI_S_DOWN) &&
                                        (ri->flags & SRI_MASTER))
                isdown = 1;

            /* Vote for the master (or fetch the previous vote) if the request
             * includes a runid, otherwise the sender is not seeking for a vote. */
            // 如果请求参数中有runid，则选举sentinel leader
            if (ri && ri->flags & SRI_MASTER && strcasecmp(c->argv[5]->ptr,"*")) {
                // 通过比较当前ri与@erq_epoch和@runid(argv[5])，判断出新的leader
                leader = sentinelVoteLeader(ri,(uint64_t)req_epoch,
                                                c->argv[5]->ptr,
                                                &leader_epoch);
            }

            /* Reply with a three-elements multi-bulk reply:
             * down state, leader, vote epoch. */
            // 给对端sentinel回复信息
            addReplyMultiBulkLen(c,3);
            addReply(c, isdown ? shared.cone : shared.czero); // 如果确实down掉了，state值为1，否则为0
            addReplyBulkCString(c, leader ? leader : "*");    // 如果有leader就返回leader的runid，否则返回*
            addReplyLongLong(c, (long long)leader_epoch);     // 返回leader的epoch
            if (leader) sdsfree(leader);
        }
    }

</font>

######7.2.5.2.1 投票选举leader######

<font color=blue>

Sentinel 自动故障迁移的一致性特质

>
>>Sentinel 自动故障迁移使用 Raft 算法来选举领头（leader） Sentinel ， 从而确保在一个给定的纪元（epoch）里， 只有一个领头产生。
>
>>这表示在同一个纪元中， 不会有两个 Sentinel 同时被选中为领头， 并且各个 Sentinel 在同一个纪元中只会对一个领头进行投票。
>
>>更高的配置纪元总是优于较低的纪元， 因此每个 Sentinel 都会主动使用更新的纪元来代替自己的配置。
>
>>简单来说， 我们可以将 Sentinel 配置看作是一个带有版本号的状态。 一个状态会以最后写入者胜出（last-write-wins）的方式（也即是，最新的配置总是胜出）传播至所有其他 Sentinel 。
>
>>举个例子， 当出现网络分割（network partitions）时， 一个 Sentinel 可能会包含了较旧的配置， 而当这个 Sentinel 接到其他 Sentinel 发来的版本更新的配置时， Sentinel 就会对自己的配置进行更新。
>
>>如果要在网络分割出现的情况下仍然保持一致性， 那么应该使用 min-slaves-to-write 选项， 让主服务器在连接的从实例少于给定数量时停止执行写操作， 与此同时， 应该在每个运行 Redis 主服务器或从服务器的机器上运行 Redis Sentinel 进程。
>

<font>

<font color=green>

    /* Vote for the sentinel with 'req_runid' or return the old vote if already
     * voted for the specifed 'req_epoch' or one greater.
     *
     * If a vote is not available returns NULL, otherwise return the Sentinel
     * runid and populate the leader_epoch with the epoch of the vote. */
    char *sentinelVoteLeader(sentinelRedisInstance *master, uint64_t req_epoch, char *req_runid, uint64_t *leader_epoch) {
        // 如果@req_epoch比当前sentinel的大，则更新sentinel的epoch
        if (req_epoch > sentinel.current_epoch) {
            sentinel.current_epoch = req_epoch;
            sentinelFlushConfig();
            sentinelEvent(REDIS_WARNING,"+new-epoch",master,"%llu",
                (unsigned long long) sentinel.current_epoch);
        }

        // 从下面的第一个判断条件可以看出，如果自己的epoch和对端的相等，二者是不会进行比较的
        if (master->leader_epoch < req_epoch && sentinel.current_epoch <= req_epoch)
        {
            // @req_epoch比@master的epoch大，则更新master的leader为@req_runid
            // 这就是扩散效应，最终总能选举出一个leader
            sdsfree(master->leader);
            master->leader = sdsnew(req_runid);
            master->leader_epoch = sentinel.current_epoch;
            sentinelFlushConfig();
            sentinelEvent(REDIS_WARNING,"+vote-for-leader",master,"%s %llu",
                master->leader, (unsigned long long) master->leader_epoch);
            /* If we did not voted for ourselves, set the master failover start
             * time to now, in order to force a delay before we can start a
             * failover for the same master. */
            if (strcasecmp(master->leader,server.runid))
                master->failover_start_time = mstime()+rand()%SENTINEL_MAX_DESYNC;
        }

        // 通过比较epoch，返回最新的leader sentinel
        *leader_epoch = master->leader_epoch;
        return master->leader ? sdsnew(master->leader) : NULL;
    }

</font>

#####7.2.5.3 处理其他sentinel发回来的投票结果#####

<font color=green>

    /* Receive the SENTINEL is-master-down-by-addr reply, see the
     * sentinelAskMasterStateToOtherSentinels() function for more information. */
    // 返回的结果的格式是: state runid epoch，详细解释参加 ####7.2.5.2
    void sentinelReceiveIsMasterDownReply(redisAsyncContext *c, void *reply, void *privdata) {
        sentinelRedisInstance *ri = c->data;
        redisReply *r;
        REDIS_NOTUSED(privdata);

        if (ri) ri->pending_commands--;
        if (!reply || !ri) return;
        r = reply;

        /* Ignore every error or unexpected reply.
         * Note that if the command returns an error for any reason we'll
         * end clearing the SRI_MASTER_DOWN flag for timeout anyway. */
        if (r->type == REDIS_REPLY_ARRAY && r->elements == 3 &&
            r->element[0]->type == REDIS_REPLY_INTEGER &&
            r->element[1]->type == REDIS_REPLY_STRING &&
            r->element[2]->type == REDIS_REPLY_INTEGER)
        {
            // 判断对端是否同意master down掉
            ri->last_master_down_reply_time = mstime();
            if (r->element[0]->integer == 1) {
                ri->flags |= SRI_MASTER_DOWN;
            } else {
                ri->flags &= ~SRI_MASTER_DOWN;
            }

            // 如果对端sentinel返回了选举的结果，则更新leader，形同“扩散”效应
            if (strcmp(r->element[1]->str,"*")) { // 比较结果不为0，则str不为*
                /* If the runid in the reply is not "*" the Sentinel actually
                 * replied with a vote. */
                sdsfree(ri->leader);
                if ((long long)ri->leader_epoch != r->element[2]->integer)
                    redisLog(REDIS_WARNING,
                        "%s voted for %s %llu", ri->name,
                        r->element[1]->str,
                        (unsigned long long) r->element[2]->integer);
                // 更新leader的runid和epoch
                ri->leader = sdsnew(r->element[1]->str);
                ri->leader_epoch = r->element[2]->integer;
            }
        }
    }

</font>

####7.2.6 执行failover动作####

<font color=green>

    // failover 状态机
    void sentinelFailoverStateMachine(sentinelRedisInstance *ri) {
        // 确认sri的master role
        redisAssert(ri->flags & SRI_MASTER);
        // 如果failover已经结束，则退出
        if (!(ri->flags & SRI_FAILOVER_IN_PROGRESS)) return;
        // 根据failover的状态执行不同的动作
        switch(ri->failover_state) {
            case SENTINEL_FAILOVER_STATE_WAIT_START:
                // 等待故障转移开始
                // 如果自己为leader，置state为SENTINEL_FAILOVER_STATE_SELECT_SLAVE，
                // 从slave选出一个master；否则等待fail-over完成/超时，不执行任何操作。
                sentinelFailoverWaitStart(ri);
                break;
            case SENTINEL_FAILOVER_STATE_SELECT_SLAVE:
                // 从众多slave中选出一个master，置state为SENTINEL_FAILOVER_STATE_SEND_SLAVEOF_NOONE
                sentinelFailoverSelectSlave(ri);
                break;
            case SENTINEL_FAILOVER_STATE_SEND_SLAVEOF_NOONE:
                // 向新选出的master发送slaveof no one命令，置state为SENTINEL_FAILOVER_STATE_WAIT_PROMOTION
                sentinelFailoverSendSlaveOfNoOne(ri);
                break;
            case SENTINEL_FAILOVER_STATE_WAIT_PROMOTION:
                // 等待failover动作开始后，sentinelRefreshInstanceInfo会把state置为SENTINEL_FAILOVER_STATE_RECONF_SLAVES
                sentinelFailoverWaitPromotion(ri);
                break;
            case SENTINEL_FAILOVER_STATE_RECONF_SLAVES:
                // 告知其他slave新的master
                sentinelFailoverReconfNextSlave(ri);
                break;
        }
    }

</font>

#####7.2.6.1 开始failover动作#####

<font color=blue>

注意下面计票函数sentinelGetLeader的第二个参数ri->failover_epoch，其值与当前sentinel的epoch是相等的，它是在sentinelStartFailover函数中被赋值的。

</font>

<font color=green>

    /* ---------------- Failover state machine implementation ------------------- */
    void sentinelFailoverWaitStart(sentinelRedisInstance *ri) {
        char *leader;
        int isleader;

        /* Check if we are the leader for the failover epoch. */
        leader = sentinelGetLeader(ri, ri->failover_epoch);
        // 判断自己是否是leader
        isleader = leader && strcasecmp(leader,server.runid) == 0;
        sdsfree(leader);

        /* If I'm not the leader, and it is not a forced failover via
         * SENTINEL FAILOVER, then I can't continue with the failover. */
        // 如果自己不是leader，则等待failover超时，退出
        if (!isleader && !(ri->flags & SRI_FORCE_FAILOVER)) {
            int election_timeout = SENTINEL_ELECTION_TIMEOUT;

            /* The election timeout is the MIN between SENTINEL_ELECTION_TIMEOUT
             * and the configured failover timeout. */
            if (election_timeout > ri->failover_timeout)
                election_timeout = ri->failover_timeout;
            /* Abort the failover if I'm not the leader after some time. */
            if (mstime() - ri->failover_start_time > election_timeout) {
                sentinelEvent(REDIS_WARNING,"-failover-abort-not-elected",ri,"%@");
                sentinelAbortFailover(ri);
            }
            return;
        }
        sentinelEvent(REDIS_WARNING,"+elected-leader",ri,"%@");
        ri->failover_state = SENTINEL_FAILOVER_STATE_SELECT_SLAVE;
        ri->failover_state_change_time = mstime();
        sentinelEvent(REDIS_WARNING,"+failover-state-select-slave",ri,"%@");
    }

</font>

######7.2.6.1.1 检查自己是否是sentinel leader######

<font color=blue>

这一步对于是否进行failover非常重要，因为只有leader才有资格执行failover。如果自己不是leader，待退出后可以通过info命令的结果得到master switch的结果。

</font>

<font color=green>

    struct sentinelLeader {
        char *runid;
        unsigned long votes;
    };

    /* Helper function for sentinelGetLeader, increment the counter
     * relative to the specified runid. */
    // 计算@runid的得票数，返回值就是其票数
    int sentinelLeaderIncr(dict *counters, char *runid) {
        dictEntry *de = dictFind(counters,runid);
        uint64_t oldval;

        if (de) {
            oldval = dictGetUnsignedIntegerVal(de);
            dictSetUnsignedIntegerVal(de,oldval+1);
            return oldval+1;
        } else {
            de = dictAddRaw(counters,runid);
            redisAssert(de != NULL);
            dictSetUnsignedIntegerVal(de,1);
            return 1;
        }
    }

    /* Scan all the Sentinels attached to this master to check if there
     * is a leader for the specified epoch.
     *
     * To be a leader for a given epoch, we should have the majority of
     * the Sentinels we know (ever seen since the last SENTINEL RESET) that
     * reported the same instance as leader for the same epoch. */
    // 在指定epoch的前提下，根据所有sentinel的投票结果计算是否已经选出了一个leader。
    // 投票已经结束，此时sentinel的候选者角色转换为记票员，其目的是查看自己是否是leader，
    // 成为leader就必须得到多数人的同意，并且其票数必须超过(Num(sentinel)/2 + 1) 或者 master->quorum
    char *sentinelGetLeader(sentinelRedisInstance *master, uint64_t epoch) {
        dict *counters;
        dictIterator *di;
        dictEntry *de;
        unsigned int voters = 0, voters_quorum;
        char *myvote;
        char *winner = NULL;
        uint64_t leader_epoch;
        uint64_t max_votes = 0;

        // master已经处于odown状态
        redisAssert(master->flags & (SRI_O_DOWN|SRI_FAILOVER_IN_PROGRESS));
        // 创建投票计账薄
        counters = dictCreate(&leaderVotesDictType,NULL);
        // 选民个数
        voters = dictSize(master->sentinels)+1; /* All the other sentinels and me. */

        /* Count other sentinels votes */
        // 投票统计【暂不把当前sentinel的投票结果包含在内】
        di = dictGetIterator(master->sentinels);
        while((de = dictNext(di)) != NULL) {
            sentinelRedisInstance *ri = dictGetVal(de);
            if (ri->leader != NULL && ri->leader_epoch == sentinel.current_epoch)
                sentinelLeaderIncr(counters,ri->leader);
        }
        dictReleaseIterator(di);

        /* Check what's the winner. For the winner to win, it needs two conditions:
         * 1) Absolute majority between voters (50% + 1).
         * 2) And anyway at least master->quorum votes. */
        // 计算出得票最多的候选者
        di = dictGetIterator(counters);
        while((de = dictNext(di)) != NULL) {
            uint64_t votes = dictGetUnsignedIntegerVal(de);

            if (votes > max_votes) {
                max_votes = votes;
                winner = dictGetKey(de);
            }
        }
        dictReleaseIterator(di);

        /* Count this Sentinel vote:
         * if this Sentinel did not voted yet, either vote for the most
         * common voted sentinel, or for itself if no vote exists at all. */
        // 计算当前sentinel的投票结果
        if (winner)
            myvote = sentinelVoteLeader(master,epoch,winner,&leader_epoch);
        else
            // 如果没有winner，就把它自己作为候选人
            myvote = sentinelVoteLeader(master,epoch,server.runid,&leader_epoch);

        // 把当前sentinel推举出来的leader与所有其他的sentinel推举出来的候选人进行一番比较
        if (myvote && leader_epoch == epoch) {
            // 计算当前sentinel推举出来的leader的票数
            uint64_t votes = sentinelLeaderIncr(counters,myvote);

            // 如果自己推举的候选人的票数多于其他sentinel推举出来的候选人的票数，就更新winner
            if (votes > max_votes) {
                max_votes = votes;
                winner = myvote;
            }
        }

        // 判别最终获胜者的票数是否合乎下面两个法则中的一个：
        // 1 得票数超过一半(Num(sentinel)/2 + 1)；
        // 2 得票数超过master自己定下的最少得票数
        voters_quorum = voters/2+1;
        if (winner && (max_votes < voters_quorum || max_votes < master->quorum))
            winner = NULL;

        winner = winner ? sdsnew(winner) : NULL;
        sdsfree(myvote);       // 自己推举的候选人
        dictRelease(counters); // 记账薄
        return winner;         // 最终获胜者
    }

</font>

#####7.2.6.2 从slave中选出一个master#####

**Sentinel 使用以下规则来选择新的主服务器：**

- 在失效主服务器属下的从服务器当中， 那些被标记为主观下线、已断线、或者最后一次回复 PING 命令的时间大于五秒钟的从服务器都会被淘汰。
- 在失效主服务器属下的从服务器当中， 那些与失效主服务器连接断开的时长超过 down-after 选项指定的时长十倍的从服务器都会被淘汰。
- 我们选出复制偏移量（replication offset）最大的那个从服务器作为新的主服务器； 如果复制偏移量不可用， 或者从服务器的复制偏移量相同， 那么带有最小运行 ID 的那个从服务器成为新的主服务器。

<font color=green>

    void sentinelFailoverSelectSlave(sentinelRedisInstance *ri) {
        sentinelRedisInstance *slave = sentinelSelectSlave(ri);

        /* We don't handle the timeout in this state as the function aborts
         * the failover or go forward in the next state. */
        if (slave == NULL) {
            sentinelEvent(REDIS_WARNING,"-failover-abort-no-good-slave",ri,"%@");
            sentinelAbortFailover(ri);
        } else {
            sentinelEvent(REDIS_WARNING,"+selected-slave",slave,"%@");
            slave->flags |= SRI_PROMOTED;
            ri->promoted_slave = slave;
            ri->failover_state = SENTINEL_FAILOVER_STATE_SEND_SLAVEOF_NOONE;
            ri->failover_state_change_time = mstime();
            sentinelEvent(REDIS_NOTICE,"+failover-state-send-slaveof-noone",
                slave, "%@");
        }
    }

    sentinelRedisInstance *sentinelSelectSlave(sentinelRedisInstance *master) {
        sentinelRedisInstance **instance =
            zmalloc(sizeof(instance[0])*dictSize(master->slaves));
        sentinelRedisInstance *selected = NULL;
        int instances = 0;
        dictIterator *di;
        dictEntry *de;
        mstime_t max_master_down_time = 0;

        if (master->flags & SRI_S_DOWN)
            max_master_down_time += mstime() - master->s_down_since_time;
        max_master_down_time += master->down_after_period * 10;

        di = dictGetIterator(master->slaves);
        while((de = dictNext(di)) != NULL) {
            sentinelRedisInstance *slave = dictGetVal(de);
            mstime_t info_validity_time;

            if (slave->flags & (SRI_S_DOWN|SRI_O_DOWN|SRI_DISCONNECTED)) continue;
            if (mstime() - slave->last_avail_time > SENTINEL_PING_PERIOD*5) continue;
            if (slave->slave_priority == 0) continue;

            /* If the master is in SDOWN state we get INFO for slaves every second.
             * Otherwise we get it with the usual period so we need to account for
             * a larger delay. */
            if (master->flags & SRI_S_DOWN)
                info_validity_time = SENTINEL_PING_PERIOD*5;
            else
                info_validity_time = SENTINEL_INFO_PERIOD*3;
            if (mstime() - slave->info_refresh > info_validity_time) continue;
            if (slave->master_link_down_time > max_master_down_time) continue;
            instance[instances++] = slave;
        }
        dictReleaseIterator(di);
        if (instances) {
            qsort(instance,instances,sizeof(sentinelRedisInstance*),
                compareSlavesForPromotion);
            selected = instance[0];
        }
        zfree(instance);
        return selected;
    }

</font>

#####7.2.6.3 执行failover#####

######7.2.6.3.1 向选出来作为master的slave发送slaveof no one命令######

<font color=green>

    void sentinelFailoverSendSlaveOfNoOne(sentinelRedisInstance *ri) {
        int retval;

        /* We can't send the command to the promoted slave if it is now
         * disconnected. Retry again and again with this state until the timeout
         * is reached, then abort the failover. */
        // 如果新选出来的master失联，则等待，一直到failover状态超时
        if (ri->promoted_slave->flags & SRI_DISCONNECTED) {
            if (mstime() - ri->failover_state_change_time > ri->failover_timeout) {
                sentinelEvent(REDIS_WARNING,"-failover-abort-slave-timeout",ri,"%@");
                sentinelAbortFailover(ri);
            }
            return;
        }

        /* Send SLAVEOF NO ONE command to turn the slave into a master.
         * We actually register a generic callback for this command as we don't
         * really care about the reply. We check if it worked indirectly observing
         * if INFO returns a different role (master instead of slave). */
        // 向master发送slaveof no one命令，这个函数里面的callback会被忽略，
        // 我们通过这个新的master对info命令的返回结果判断其是否已经完成了角色转换
        retval = sentinelSendSlaveOf(ri->promoted_slave,NULL,0);
        if (retval != REDIS_OK) return;
        sentinelEvent(REDIS_NOTICE, "+failover-state-wait-promotion",
            ri->promoted_slave,"%@");
        ri->failover_state = SENTINEL_FAILOVER_STATE_WAIT_PROMOTION;
        ri->failover_state_change_time = mstime();
    }

</font>

#####7.2.6.3.2 告知旧master以及其他slave新的master#####

<font color=green>

    /* Send SLAVEOF to the specified instance, always followed by a
     * CONFIG REWRITE command in order to store the new configuration on disk
     * when possible (that is, if the Redis instance is recent enough to support
     * config rewriting, and if the server was started with a configuration file).
     *
     * If Host is NULL the function sends "SLAVEOF NO ONE".
     *
     * The command returns REDIS_OK if the SLAVEOF command was accepted for
     * (later) delivery otherwise REDIS_ERR. The command replies are just
     * discarded. */
    // 让新的master完成角色转换任务
    int sentinelSendSlaveOf(sentinelRedisInstance *ri, char *host, int port) {
        char portstr[32];
        int retval;

        ll2string(portstr,sizeof(portstr),port);

        /* If host is NULL we send SLAVEOF NO ONE that will turn the instance
         * into a master. */
        if (host == NULL) {
            host = "NO";
            memcpy(portstr,"ONE",4);
        }

        /* In order to send SLAVEOF in a safe way, we send a transaction performing
         * the following tasks:
         * 1) Reconfigure the instance according to the specified host/port params.
         * 2) Rewrite the configuraiton.
         * 3) Disconnect all clients (but this one sending the commnad) in order
         *    to trigger the ask-master-on-reconnection protocol for connected
         *    clients.
         *
         * Note that we don't check the replies returned by commands, since we
         * will observe instead the effects in the next INFO output. */
        // 为了安全的完成任务，这里使用了redis的事务:
        // 1) 发出slaveof no one
        // 2) 命令新的master新的configue写进磁盘
        // 3) 断开新的master与所有client之间的连接
        retval = redisAsyncCommand(ri->cc,
            sentinelDiscardReplyCallback, NULL, "MULTI");
        if (retval == REDIS_ERR) return retval;
        ri->pending_commands++;

        retval = redisAsyncCommand(ri->cc,
            sentinelDiscardReplyCallback, NULL, "SLAVEOF %s %s", host, portstr);
        if (retval == REDIS_ERR) return retval;
        ri->pending_commands++;

        retval = redisAsyncCommand(ri->cc,
            sentinelDiscardReplyCallback, NULL, "CONFIG REWRITE");
        if (retval == REDIS_ERR) return retval;
        ri->pending_commands++;

        /* CLIENT KILL TYPE <type> is only supported starting from Redis 2.8.12,
         * however sending it to an instance not understanding this command is not
         * an issue because CLIENT is variadic command, so Redis will not
         * recognized as a syntax error, and the transaction will not fail (but
         * only the unsupported command will fail). */
        retval = redisAsyncCommand(ri->cc,
            sentinelDiscardReplyCallback, NULL, "CLIENT KILL TYPE normal");
        if (retval == REDIS_ERR) return retval;
        ri->pending_commands++;

        retval = redisAsyncCommand(ri->cc,
            sentinelDiscardReplyCallback, NULL, "EXEC");
        if (retval == REDIS_ERR) return retval;
        ri->pending_commands++;

        return REDIS_OK;
    }

</font>

#####7.2.6.4 确认新master在failover timeout时间内是否完成了角色转换任务#####

<font color=green>

    /* We actually wait for promotion indirectly checking with INFO when the
     * slave turns into a master. */
    // 检查failover时间内，通过info命令的结果确认新的master是否完成了角色转换，
    // 如果failover超时，则终止failover任务。
    void sentinelFailoverWaitPromotion(sentinelRedisInstance *ri) {
        /* Just handle the timeout. Switching to the next state is handled
         * by the function parsing the INFO command of the promoted slave. */
        if (mstime() - ri->failover_state_change_time > ri->failover_timeout) {
            sentinelEvent(REDIS_WARNING,"-failover-abort-slave-timeout",ri,"%@");
            sentinelAbortFailover(ri);
        }
    }

    /* Process the INFO output from masters. */
    void sentinelRefreshInstanceInfo(sentinelRedisInstance *ri, const char *info) {
        /* Handle slave -> master role switch. */
        if ((ri->flags & SRI_SLAVE) && role == SRI_MASTER) {
            /* If this is a promoted slave we can change state to the
             * failover state machine. */
            if ((ri->flags & SRI_PROMOTED) &&
                (ri->master->flags & SRI_FAILOVER_IN_PROGRESS) &&
                (ri->master->failover_state ==
                    SENTINEL_FAILOVER_STATE_WAIT_PROMOTION))
            {
                /* Now that we are sure the slave was reconfigured as a master
                 * set the master configuration epoch to the epoch we won the
                 * election to perform this failover. This will force the other
                 * Sentinels to update their config (assuming there is not
                 * a newer one already available). */
                ri->master->config_epoch = ri->master->failover_epoch;
                ri->master->failover_state = SENTINEL_FAILOVER_STATE_RECONF_SLAVES;
                ri->master->failover_state_change_time = mstime();
                sentinelFlushConfig();
                sentinelEvent(REDIS_WARNING,"+promoted-slave",ri,"%@");
                sentinelEvent(REDIS_WARNING,"+failover-state-reconf-slaves",
                    ri->master,"%@");
                sentinelCallClientReconfScript(ri->master,SENTINEL_LEADER,
                    "start",ri->master->addr,ri->addr);
                sentinelForceHelloUpdateForMaster(ri->master);
            } else {
                /* A slave turned into a master. We want to force our view and
                 * reconfigure as slave. Wait some time after the change before
                 * going forward, to receive new configs if any. */
                mstime_t wait_time = SENTINEL_PUBLISH_PERIOD*4;

                if (!(ri->flags & SRI_PROMOTED) &&
                     sentinelMasterLooksSane(ri->master) &&
                     sentinelRedisInstanceNoDownFor(ri,wait_time) &&
                     mstime() - ri->role_reported_time > wait_time)
                {
                    int retval = sentinelSendSlaveOf(ri,
                            ri->master->addr->ip,
                            ri->master->addr->port);
                    if (retval == REDIS_OK)
                        sentinelEvent(REDIS_NOTICE,"+convert-to-slave",ri,"%@");
                }
            }
        }
    }

</font>

#####7.2.6.5 告知其他所有的slave新的master的host&port#####

<font color=green>

    /* Send SLAVE OF <new master address> to all the remaining slaves that
     * still don't appear to have the configuration updated. */
    // 遍历所有的slave，告知其新的master
    void sentinelFailoverReconfNextSlave(sentinelRedisInstance *master) {
        dictIterator *di;
        dictEntry *de;
        int in_progress = 0;

        di = dictGetIterator(master->slaves);
        while((de = dictNext(di)) != NULL) {
            sentinelRedisInstance *slave = dictGetVal(de);

            if (slave->flags & (SRI_RECONF_SENT|SRI_RECONF_INPROG))
                in_progress++;
        }
        dictReleaseIterator(di);

        di = dictGetIterator(master->slaves);
        while(in_progress < master->parallel_syncs &&
              (de = dictNext(di)) != NULL)
        {
            sentinelRedisInstance *slave = dictGetVal(de);
            int retval;

            /* Skip the promoted slave, and already configured slaves. */
            if (slave->flags & (SRI_PROMOTED|SRI_RECONF_DONE)) continue;

            /* If too much time elapsed without the slave moving forward to
             * the next state, consider it reconfigured even if it is not.
             * Sentinels will detect the slave as misconfigured and fix its
             * configuration later. */
            if ((slave->flags & SRI_RECONF_SENT) &&
                (mstime() - slave->slave_reconf_sent_time) >
                SENTINEL_SLAVE_RECONF_TIMEOUT)
            {
                sentinelEvent(REDIS_NOTICE,"-slave-reconf-sent-timeout",slave,"%@");
                slave->flags &= ~SRI_RECONF_SENT;
                slave->flags |= SRI_RECONF_DONE;
            }

            /* Nothing to do for instances that are disconnected or already
             * in RECONF_SENT state. */
            // 如果slave失联，不做任何操作
            if (slave->flags & (SRI_DISCONNECTED|SRI_RECONF_SENT|SRI_RECONF_INPROG))
                continue;

            /* Send SLAVEOF <new master>. */
            // 发送slaveof new master命令
            retval = sentinelSendSlaveOf(slave,
                    master->promoted_slave->addr->ip,
                    master->promoted_slave->addr->port);
            if (retval == REDIS_OK) {
                slave->flags |= SRI_RECONF_SENT;
                slave->slave_reconf_sent_time = mstime();
                sentinelEvent(REDIS_NOTICE,"+slave-reconf-sent",slave,"%@");
                in_progress++;
            }
        }
        dictReleaseIterator(di);

        /* Check if all the slaves are reconfigured and handle timeout. */
        // 检验是否所有的slave都已经获取到新的master
        sentinelFailoverDetectEnd(master);
    }

</font>

######7.2.6.5.1 检验是否所有的slave都已经获取到新的master######

<font color=green>

    void sentinelFailoverDetectEnd(sentinelRedisInstance *master) {
        int not_reconfigured = 0, timeout = 0;
        dictIterator *di;
        dictEntry *de;
        mstime_t elapsed = mstime() - master->failover_state_change_time;

        /* We can't consider failover finished if the promoted slave is
         * not reachable. */
        // 如果新的master不可达，则结束failover
        if (master->promoted_slave == NULL ||
            master->promoted_slave->flags & SRI_S_DOWN) return;

        /* The failover terminates once all the reachable slaves are properly
         * configured. */
        // 如果所有的slave都确认了新的master，则结束failver动作
        di = dictGetIterator(master->slaves);
        while((de = dictNext(di)) != NULL) {
            sentinelRedisInstance *slave = dictGetVal(de);

            if (slave->flags & (SRI_PROMOTED|SRI_RECONF_DONE)) continue;
            if (slave->flags & SRI_S_DOWN) continue;
            not_reconfigured++;
        }
        dictReleaseIterator(di);

        /* Force end of failover on timeout. */
        // 如果failover超时，强制结束failover
        if (elapsed > master->failover_timeout) {
            not_reconfigured = 0;
            timeout = 1;
            sentinelEvent(REDIS_WARNING,"+failover-end-for-timeout",master,"%@");
        }

        if (not_reconfigured == 0) {
            sentinelEvent(REDIS_WARNING,"+failover-end",master,"%@");
            master->failover_state = SENTINEL_FAILOVER_STATE_UPDATE_CONFIG;
            master->failover_state_change_time = mstime();
        }

        /* If I'm the leader it is a good idea to send a best effort SLAVEOF
         * command to all the slaves still not reconfigured to replicate with
         * the new master. */
        // 如果ri是master，则再向没有连接上来的slave发送slaveof命令
        if (timeout) {
            dictIterator *di;
            dictEntry *de;

            di = dictGetIterator(master->slaves);
            while((de = dictNext(di)) != NULL) {
                sentinelRedisInstance *slave = dictGetVal(de);
                int retval;

                if (slave->flags &
                    (SRI_RECONF_DONE|SRI_RECONF_SENT|SRI_DISCONNECTED)) continue;

                retval = sentinelSendSlaveOf(slave,
                        master->promoted_slave->addr->ip,
                        master->promoted_slave->addr->port);
                if (retval == REDIS_OK) {
                    sentinelEvent(REDIS_NOTICE,"+slave-reconf-sent-be",slave,"%@");
                    slave->flags |= SRI_RECONF_SENT;
                }
            }
            dictReleaseIterator(di);
        }
    }

</font>

## 主要参考文档：##

- 1 redis/src/sentinel.c
- 2 http://redisdoc.com/topic/sentinel.html

## sentinel log:
    [14114] 28 Jan 20:22:57.136 # Sentinel runid is 5e9ad1d7938e1b40028ac0b93eb41d0b422c7421
    [14114] 28 Jan 20:22:57.136 # +monitor master mymaster 127.0.0.1 6379 quorum 2
    [14114] 28 Jan 20:22:57.136 # +monitor master server3 10.42.140.47 6379 quorum 1
    [14114] 28 Jan 20:22:57.136 # +monitor master server1 10.42.140.47 6379 quorum 1
    [14114] 28 Jan 20:23:27.170 # +sdown master mymaster 127.0.0.1 6379
    [14114] 28 Jan 20:23:27.170 # +sdown master server3 10.42.140.47 6379
    [14114] 28 Jan 20:23:27.170 # +odown master server3 10.42.140.47 6379 #quorum 1/1
    [14114] 28 Jan 20:23:27.170 # +new-epoch 2
    [14114] 28 Jan 20:23:27.170 # +try-failover master server3 10.42.140.47 6379
    [14114] 28 Jan 20:23:27.171 # +vote-for-leader 5e9ad1d7938e1b40028ac0b93eb41d0b422c7421 2
    [14114] 28 Jan 20:23:27.171 # +elected-leader master server3 10.42.140.47 6379
    [14114] 28 Jan 20:23:27.171 # +failover-state-select-slave master server3 10.42.140.47 6379
    [14114] 28 Jan 20:23:27.171 # +sdown master server1 10.42.140.47 6379
    [14114] 28 Jan 20:23:27.171 # +odown master server1 10.42.140.47 6379 #quorum 1/1
    [14114] 28 Jan 20:23:27.171 # +new-epoch 3
    [14114] 28 Jan 20:23:27.171 # +try-failover master server1 10.42.140.47 6379
    [14114] 28 Jan 20:23:27.171 # +vote-for-leader 5e9ad1d7938e1b40028ac0b93eb41d0b422c7421 3
    [14114] 28 Jan 20:23:27.171 # +elected-leader master server1 10.42.140.47 6379
    [14114] 28 Jan 20:23:27.171 # +failover-state-select-slave master server1 10.42.140.47 6379
    [14114] 28 Jan 20:23:27.229 # -failover-abort-no-good-slave master server3 10.42.140.47 6379
    [14114] 28 Jan 20:23:27.229 # -failover-abort-no-good-slave master server1 10.42.140.47 6379
    [14114] 28 Jan 20:23:27.295 # Next failover delay: I will not start a failover before Wed Jan 28 20:53:27 2015
    [14114] 28 Jan 20:23:27.295 # Next failover delay: I will not start a failover before Wed Jan 28 20:53:27 2015
