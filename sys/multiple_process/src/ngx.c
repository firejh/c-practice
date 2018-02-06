#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>   // daemon

#include <sys/time.h>
#include <sys/resource.h>

#include "log.h"

#define DefaultConfigFile "/opt/project/getslice/conf/getslice_lunbo.conf"
#define PID_FILENAME "/var/run/getslice_lunbo.pid"
#define VERSION "1.0.0"

#define MAX_PROCESSES         128
#define PROCESS_NORESPAWN     -1 // 子进程退出时,父进程不会再次重启
#define PROCESS_RESPAWN       -3 // 子进程异常退出时,父进程需要重启

#define PROCESS_NAME_LEN      64

typedef void (*spawn_proc_pt) (void *data);

typedef struct {
    pid_t           pid;    //该进程pid
    int             status; //进程状态，通过sig_child获取
    spawn_proc_pt   proc;  //创建进程时候的回调
    void            *data; //创建进程时候传递的参数
    char            name[PROCESS_NAME_LEN]; //进程名

    unsigned        respawn:1; //PROCESS_RESPAWN 创建进程的时候释放指定需要master拉起一个新的进程，如果进程挂了
    unsigned        exited:1;
} process_t;

typedef struct {
    process_t processes[MAX_PROCESSES];

    sig_atomic_t reap;
    int last_process; // 每创建一个进程就++

    /* getopt args*/
    int opt_no_daemon;

    char **os_argv;
    char *os_argv_last;
    char app_name[PROCESS_NAME_LEN];
    int spawn_type;
    int worker_process_num; // 子进程数目

    int exit;
    int quit;
} config_t, *config_p, config_a[1];

config_a g_config_a = {{0}};
char* g_conf_file = NULL;
config_p g_conf = NULL;

void init_config(void)
{
    g_conf->worker_process_num = 3;
    g_conf->spawn_type = PROCESS_RESPAWN;

    g_conf->exit = 0;
    g_conf->quit = 0;
}

void init_set_proc_title(void)
{
    char   *p = NULL;
    size_t size = 0;
    int    i = 0;

    g_conf->os_argv_last = g_conf->os_argv[0];
    for (i = 0; g_conf->os_argv[i]; i++) {
        if (g_conf->os_argv_last == g_conf->os_argv[i]) {
            g_conf->os_argv_last = g_conf->os_argv[i] + strlen(g_conf->os_argv[i]) + 1;
        }
    }

    g_conf->os_argv_last += strlen(g_conf->os_argv_last);
}

void set_proc_title(char *title)
{
    char  *p;
    g_conf->os_argv[1] = NULL;

    size_t len = strlen(title);
    // if (strlen((char *)g_conf->os_argv[0]) < len) {
    //     len = strlen((char *)g_conf->os_argv[0]);
    // }
    p = strncpy((char *)g_conf->os_argv[0], (char *)title, len);
    // p += strlen(title);
    p += len;

    if (g_conf->os_argv_last - (char *) p > 0) {
        memset(p, ' ', g_conf->os_argv_last - (char *) p);
    }
}

// SIGTTOU  //stdout
// SIGTTIN  //stdin
// SIGINT   //ctrl--c
// SIGQUIT  //ctrl--\  /
// SIGTSTP  //ctrl--z
// SIGHUP   //session leader, shell terminal
// SIGCONT  //shell terminal
// SIGSTOP  //shell terminal

void sig_handler(int sig)
{
    switch (sig) {
        case SIGTERM:
            g_conf->quit = 1;
            break;

        default:
            g_conf->exit = 1;
            break;
    }
}

void worker_process_init(int worker)
{
    LOG_INFO("init worker process, worker pid:%ld, worker id:%d", getpid(), worker);
}

int worker_process(int worker)
{
    LOG_INFO("run worker process, worker pid:%ld, worker id:%d", getpid(), worker);
    sleep(10);
    return 0;
}

void worker_process_exit(int worker)
{
    LOG_INFO("destroy worker process, worker pid:%ld, worker id:%d", getpid(), worker);
}

void worker_process_cycle(void *data)
{
    int ret = 0;
    char process_name[PROCESS_NAME_LEN] = {0};
    int worker = (intptr_t)data;
    LOG_INFO("worker process cycle worker id %d", worker);

    snprintf(process_name, sizeof(process_name), "%s-%s%d", g_conf->app_name, "worker", worker);
    set_proc_title(process_name);
    worker_process_init(worker);

    for (;;) {
        if (g_conf->quit) {
            break;
        }
        if (g_conf->exit) {
            LOG_WARN("worker id %d, worker process id %d, exiting", worker, getpid());
            worker_process_exit(worker);
            break;
        }
        // LOG_INFO("worker cycle pid: %d\n", getpid());
        ret = worker_process(worker);
        if (ret == -1) {
            g_conf->exit = 1;
        }
    }
}

void dispatcher_process_init(int worker)
{
    LOG_INFO("init dispatcher process, process id:%ld, dispatcher id:%d", getpid(), worker);
}

int dispatcher_process(int worker)
{
    LOG_INFO("run dispatcher process, process id:%ld, dispatcher id:%d", getpid(), worker);
    sleep(10);
    return 0;
}

void dispatcher_process_exit(int worker)
{
    LOG_INFO("destroy dispatcher process, process id:%ld, dispatcher id:%d", getpid(), worker);
}

void dispatcher_process_cycle(void *data)
{
    int worker = (intptr_t) data;

    char process_name[PROCESS_NAME_LEN] = {0};
    // snprintf(process_name, sizeof(process_name), "%s-%s-%d", g_conf->app_name, "dispatcher-process", worker);
    snprintf(process_name, sizeof(process_name), "%s-%s%d", g_conf->app_name, "dispatcher", worker);
    set_proc_title(process_name);
    pline("dispatcher process id %d, process name %s", getpid(), process_name);

    dispatcher_process_init(worker);

    for (;;) {
        if (g_conf->quit) {
            break;
        }
        if (g_conf->exit) {
            LOG_WARN("dispatcher id %d, dispatcher process id %d, exiting", worker, getpid());
            worker_process_exit(worker);
            break;
        }

        // LOG_WARN("dispatcher cycle pid: %d\n", getpid());
        int ret = dispatcher_process(worker);
        if (ret == -1) {
            g_conf->exit = 1;
        }
    }
}

void start_worker_processes(int n, int type)
{
    int i;
    LOG_INFO("start worker processes");

    char process_name[PROCESS_NAME_LEN];
    for (i = 0; i < n; i++) {
        snprintf(process_name, sizeof(process_name), "worker-process-%d", i);
        spawn_process(worker_process_cycle, (void*)(intptr_t)(i), process_name, type);
    }
}

void start_dispatcher_process(int type)
{
    LOG_INFO("start dispatcher processes\n");
    spawn_process(dispatcher_process_cycle, (void *)(intptr_t)(0), "dispatcher process", type);
}

void save_argv(int argc, char *const *argv)
{
    g_conf->os_argv = (char **) argv;
}

char* get_process_name()
{
    int i;
    pid_t pid = getpid();

    for (i = 0; i < g_conf->last_process; i++) {
        if (g_conf->processes[i].pid == pid) {
            return g_conf->processes[i].name;
        }
    }

    return "";
}

void sig_child(int sig)
{
    int status;
    int i;
    pid_t pid;

    LOG_INFO("pid:%ld, process name:%s, get signal:%d", getpid(), get_process_name(), sig);
    g_conf->reap = 1;
    do {
        pid = waitpid(-1, &status, WNOHANG);
        for (i = 0; i < g_conf->last_process; i++) {
            if (g_conf->processes[i].pid == pid) {
                g_conf->processes[i].status = status;
                g_conf->processes[i].exited = 1;
                break;
            }
        }
    } while (pid > 0);
    signal(sig, sig_child);
}

typedef void SIGHDLR(int sig);
void signal_set(int sig, SIGHDLR * func, int flags)
{
    struct sigaction sa;
    sa.sa_handler = func;
    sa.sa_flags = flags;
    sigemptyset(&sa.sa_mask);
    if (sigaction(sig, &sa, NULL) < 0) {
        LOG_WARN("sigaction: sig=%d func=%p: %s\n", sig, func, strerror(errno));
    }
}

void init_signals(void)
{
    signal_set(SIGCHLD, sig_child, SA_NODEFER | SA_RESTART);
}

static pid_t read_pid_file(void)
{
    FILE *pid_fp = NULL;
    const char *f = PID_FILENAME;
    pid_t pid = -1;
    int i;

    if (f == NULL) {
        LOG_WARN("%s: error: no pid file name defined\n", g_conf->app_name);
        exit(1);
    }

    pid_fp = fopen(f, "r");
    if (pid_fp != NULL) {
        pid = 0;
        if (fscanf(pid_fp, "%d", &i) == 1) {
            pid = (pid_t) i;
        }
        fclose(pid_fp);
    } else {
        if (errno != ENOENT) {
            LOG_WARN("%s: error: could not read pid file\n", g_conf->app_name);
            LOG_WARN("%s: %s\n", f, strerror(errno));
            exit(1);
        }
    }
    return pid;
}

int check_running_pid(void)
{
    pid_t pid;
    pid = read_pid_file();
    if (pid < 2) {
        return 0;
    }
    if (kill(pid, 0) < 0) {
        return 0;
    }
    LOG_WARN("%s_master is already running!  process id %ld\n", g_conf->app_name, (long int)(pid));

    return 1;
}

void write_pidfile(void)
{
    FILE *fp;
    const char *f = PID_FILENAME;
    fp = fopen(f, "w+");
    if (!fp) {
        LOG_WARN("could not write pid file '%s': %s\n", f, strerror(errno));
        return;
    }
    fprintf(fp, "%d\n", (int) getpid());
    fclose(fp);
}

void usage(void)
{
    pline("Usage: %s [-?hvVN] [-d level] [-c config-file] [-k signal]\n"
          "       -h        Print help message.\n"
          "       -v        Show Version and exit.\n"
          "       -N        No daemon mode.\n"
          "       -c file   Use given config-file instead of %s\n",
          g_conf->app_name, DefaultConfigFile);

    exit(1);
}

static void show_version(void)
{
    LOG_WARN("%s version: %s\n", g_conf->app_name, VERSION);
    exit(1);
}

void parse_options(int argc, char *argv[])
{
    extern char *optarg;
    int c;

    while ((c = getopt(argc, argv, "hvNc:k:?")) != -1) {
        switch (c) {
            case 'h':
                usage();
                break;
            case 'v':
                show_version();
                break;
            case 'N':
                g_conf->opt_no_daemon = 1;
                break;
            case 'c':
                g_conf_file = strdup(optarg);
                break;
           case '?':
            default:
                usage();
                break;
        }
    }
}

void enable_coredump(void)
{
    /* Set Linux DUMPABLE flag */
    if (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0) != 0) {
        LOG_WARN("prctl: %s\n", strerror(errno));
    }

    /* Make sure coredumps are not limited */
    struct rlimit rlim;
    if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
        rlim.rlim_cur = rlim.rlim_max;
        if (setrlimit(RLIMIT_CORE, &rlim) == 0) {
            LOG_INFO("Enable Core Dumps OK!\n");
            return;
        }
    }
    LOG_WARN("Enable Core Dump failed: %s\n",strerror(errno));
}

int reap_children(void)
{
    int   i, n;
    int   live;

    live = 0;
    for (i = 0; i < g_conf->last_process; i++) { //之前挂掉的进程
                //child[0] 26718 e:0 t:0 d:0 r:1 j:0
        LOG_WARN("child[%d] %d t:%d r:%d\n",
                       i,
                       g_conf->processes[i].pid,
                       g_conf->processes[i].exited,
                       g_conf->processes[i].respawn);

        if (g_conf->processes[i].pid == -1) {
            continue;
        }

        if (g_conf->processes[i].exited) {
            if (g_conf->processes[i].respawn) {  //需要重启进程
                if (spawn_process(g_conf->processes[i].proc, g_conf->processes[i].data, g_conf->processes[i].name, i) == -1) {
                    LOG_WARN("could not respawn %s\n", g_conf->processes[i].name);
                    continue;
                }

                live = 1;

                continue;
            }

            if (i == g_conf->last_process - 1) {
                g_conf->last_process--;
            } else {
                g_conf->processes[i].pid = -1; // 标记空位，spawn_process的时候会使用这个位置
            }
        }
    }

    return live;
}

pid_t spawn_process(spawn_proc_pt proc, void *data, char *name, int respawn)
{
    long  on;
    pid_t pid;
    int   s;

    if (respawn >= 0) {
        s = respawn;
    } else {
        for (s = 0; s < g_conf->last_process; s++) {
            if (g_conf->processes[s].pid == -1) {
                break;
            }
        }

        if (s == MAX_PROCESSES) {
            LOG_WARN("no more than %d g_conf->processes can be spawned", MAX_PROCESSES);
            return -1;
        }
    }

    pid = fork();

    switch (pid) {
    case -1:
        LOG_WARN("fork() failed while spawning \"%s\" :%s", name, errno);
        return -1;

    case 0:
        pid = getpid();
        proc(data);
        exit(EXIT_SUCCESS);
        break;

    default:
        break;
    }

    LOG_INFO("start %s %d\n", name, pid);

    g_conf->processes[s].pid = pid;
    g_conf->processes[s].exited = 0;

    if (respawn >= 0) {
        return pid;
    }

    g_conf->processes[s].proc = proc;
    g_conf->processes[s].data = data;
    strncpy(g_conf->processes[s].name, name, sizeof(g_conf->processes[s].name));

    switch (respawn) {
    case PROCESS_NORESPAWN:
        g_conf->processes[s].respawn = 0;
        break;

    case PROCESS_RESPAWN:
        g_conf->processes[s].respawn = 1;
        break;
    }

    if (s == g_conf->last_process) {
        g_conf->last_process++;
    }

    return pid;
}

int main(int argc, char **argv)
{
    int fd;
    int i;
    pid_t pid;
    sigset_t set;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGHUP, SIG_IGN);

    g_conf = g_config_a;
    parse_options(argc, argv);
    if (check_running_pid()) {
        exit(1);
    }

    init_config();
    strncpy(g_conf->app_name, argv[0], sizeof(g_conf->app_name));

    enable_coredump();
    if (g_conf->opt_no_daemon) {
#ifdef __linux
        pline("daemon\n");
        daemon(1, 1);
#endif
    }
    write_pidfile();
    save_argv(argc, argv);
    init_set_proc_title();
    init_signals();
    sigemptyset(&set);

    start_dispatcher_process(PROCESS_RESPAWN);
    LOG_DEBUG("father pid2=%d", getpid());

    LOG_DEBUG("father pid1=%d", getpid());
    start_worker_processes(g_conf->worker_process_num, g_conf->spawn_type);

    int live = 1;
    for (;;) {
        LOG_DEBUG("father before suspend");
        sigsuspend(&set);
        LOG_DEBUG("father after suspend");
        if (g_conf->reap) {
            g_conf->reap = 0;
            live = reap_children();
            LOG_INFO("reap children, result:%d", live);
        }
    }

    return 0;
}

