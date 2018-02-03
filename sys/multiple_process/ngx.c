#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/resource.h>

#define DefaultConfigFile "/opt/project/getslice/conf/getslice_lunbo.conf"
#define PID_FILENAME "/var/run/getslice_lunbo.pid"
#define VERSION "1.0.0"

#define MAX_PROCESSES 128
#define PROCESS_NORESPAWN     -1
#define PROCESS_JUST_SPAWN    -2
#define PROCESS_RESPAWN       -3
#define PROCESS_JUST_RESPAWN  -4
#define PROCESS_DETACHED      -5

const char *appname="getslice_lunbo";
char *ConfigFile = NULL;

typedef void (*spawn_proc_pt) (void *data);

typedef struct {
    pid_t           pid;
    int                 status;
    //int        channel[2];

    spawn_proc_pt   proc;
    void               *data;
    char               *name;

    unsigned            respawn:1;
    unsigned            just_spawn:1;
    unsigned            detached:1;
    unsigned            exiting:1;
    unsigned            exited:1;
} process_t;

typedef struct {
    process_t processes[MAX_PROCESSES];
    int process_slot;

    /* getopt args*/
    int opt_no_daemon;
    int opt_debug_stderr;
    int opt_parse_cfg_only;
    int opt_send_signal;

    sig_atomic_t reap;
    sig_atomic_t terminate;
    sig_atomic_t quit;
    int last_process;
    int exiting;

    char **os_argv;
    char *os_argv_last;
} config_t, *config_p, config_a[1];

config_a g_config_a = {{0}};
config_p g_conf = NULL;

void init_config(void)
{
    g_conf = g_config_a;

    g_conf->opt_debug_stderr = -1;
    g_conf->opt_send_signal = -1;
}

void init_setproctitle(void)
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

void setproctitle(char *title)
{
    char  *p;
    g_conf->os_argv[1] = NULL;

    p = strncpy((char *) g_conf->os_argv[0], (char *) title, strlen(title));
    p += strlen(title);

    if (g_conf->os_argv_last - (char *) p > 0) {
        memset(p, ' ', g_conf->os_argv_last - (char *) p);
    }
}

void worker_process_init(int worker)
{

}

void worker_process_exit(void)
{

}

void worker_process_cycle(void *data)
{
    int worker = (intptr_t) data;

    worker_process_init(worker);

    setproctitle("getslice:worker process");

    for ( ;; ) {
        if (g_conf->exiting) {
                fprintf(stderr, "g_conf->exiting");
                worker_process_exit();
        }
        sleep(1);
        fprintf(stderr, "\tworker cycle pid: %d\n", getpid());

        if (g_conf->terminate) {
            fprintf(stderr, "g_conf->exiting");
            worker_process_exit();
        }

        if (g_conf->quit) {
            g_conf->quit = 0;
            fprintf(stderr, "gracefully shutting down");
            setproctitle("worker process is shutting down");

            if (!g_conf->exiting) {
                g_conf->exiting = 1;
            }
        }
    }
}

void dispatcher_process_init(int worker)
{

}

void dispatcher_process_exit()
{

}
void dispatcher_process_cycle(void *data)
{
    int worker = (intptr_t) data;

    dispatcher_process_init(worker);

    setproctitle("getslice:dispatcher process");

    for ( ;; ) {
        if (g_conf->exiting) {
                fprintf(stderr, "g_conf->exiting\n");
                dispatcher_process_exit();
        }

        sleep(1);
        fprintf(stderr, "\tdispatcher cycle pid: %d\n", getpid());

        if (g_conf->terminate) {
            fprintf(stderr, "g_conf->exiting\n");
            dispatcher_process_exit();
        }

        if (g_conf->quit) {
            g_conf->quit = 0;
            fprintf(stderr, "gracefully shutting down\n");
            setproctitle("worker process is shutting down");

            if (!g_conf->exiting) {
                g_conf->exiting = 1;
            }
        }
    }
}

void start_worker_processes(int n, int type)
{
    int i;
    fprintf(stderr, "start worker g_conf->processes\n");

    for (i = 0; i < n; i++) {
        spawn_process(worker_process_cycle, (void *) (intptr_t) i, "worker process", type);
    }
}

void start_dispatcher_process(int type)
{
    fprintf(stderr, "start dispatcher g_conf->processes\n");
    spawn_process(dispatcher_process_cycle, (void *) (intptr_t) 0, "dispatcher process", type);
}


void save_argv(int argc, char *const *argv)
{
    g_conf->os_argv = (char **) argv;
}

void sig_child(int sig)
{
    g_conf->reap = 1;

    int status;
    int i;
    pid_t pid;

    do {
        pid = waitpid(-1, &status, WNOHANG);
        for (i = 0; i < g_conf->last_process; i++) {
            if (g_conf->processes[i].pid == pid) {
                g_conf->processes[i].status = status;
                g_conf->processes[i].exited = 1;
                //process = g_conf->processes[i].name;
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
    if (sigaction(sig, &sa, NULL) < 0)
        fprintf(stderr, "sigaction: sig=%d func=%p: %s\n", sig, func, strerror(errno));
}

void init_signals(void)
{
    signal_set(SIGCHLD, sig_child, SA_NODEFER | SA_RESTART);
}

static pid_t readPidFile(void)
{
    FILE *pid_fp = NULL;
    const char *f = PID_FILENAME;
    pid_t pid = -1;
    int i;

    if (f == NULL) {
        fprintf(stderr, "%s: error: no pid file name defined\n", appname);
        exit(1);
    }

    pid_fp = fopen(f, "r");
    if (pid_fp != NULL) {
        pid = 0;
        if (fscanf(pid_fp, "%d", &i) == 1)
            pid = (pid_t) i;
        fclose(pid_fp);
    } else {
        if (errno != ENOENT) {
            fprintf(stderr, "%s: error: could not read pid file\n", appname);
            fprintf(stderr, "\t%s: %s\n", f, strerror(errno));
            exit(1);
        }
    }
    return pid;
}

int checkRunningPid(void)
{
    pid_t pid;
    pid = readPidFile();
    if (pid < 2) {
        return 0;
    }
    if (kill(pid, 0) < 0) {
        return 0;
    }
    fprintf(stderr, "getslice_master is already running!  process id %ld\n", (long int) pid);

    return 1;
}

void writePidFile(void)
{
    FILE *fp;
    const char *f = PID_FILENAME;
    fp = fopen(f, "w+");
    if (!fp) {
        fprintf(stderr, "could not write pid file '%s': %s\n", f, strerror(errno));
        return;
    }
    fprintf(fp, "%d\n", (int) getpid());
    fclose(fp);
}

void usage(void)
{
    fprintf(stderr,
            "Usage: %s [-?hvVN] [-d level] [-c config-file] [-k signal]\n"
            "       -h        Print help message.\n"
            "       -v        Show Version and exit.\n"
            "       -N        No daemon mode.\n"
            "       -c file   Use given config-file instead of\n"
            "                 %s\n"
            "       -k reload|rotate|kill|parse\n"
            "                 kill is fast shutdown\n"
            "                 Parse configuration file, then send signal to \n"
            "                 running copy (except -k parse) and exit.\n",
            appname, DefaultConfigFile);
    exit(1);
}

static void show_version(void)
{
    fprintf(stderr, "%s version: %s\n", appname,VERSION);
    exit(1);
}

void mainParseOptions(int argc, char *argv[])
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
                ConfigFile = strdup(optarg);
                break;
            case 'k':
                if ((int) strlen(optarg) < 1)
                    usage();
                if (!strncmp(optarg, "reload", strlen(optarg)))
                    g_conf->opt_send_signal = SIGHUP;
                else if (!strncmp(optarg, "rotate", strlen(optarg)))
                    g_conf->opt_send_signal = SIGUSR1;
                else if (!strncmp(optarg, "shutdown", strlen(optarg)))
                    g_conf->opt_send_signal = SIGTERM;
                else if (!strncmp(optarg, "kill", strlen(optarg)))
                    g_conf->opt_send_signal = SIGKILL;
                else if (!strncmp(optarg, "parse", strlen(optarg)))
                    g_conf->opt_parse_cfg_only = 1;        /* parse cfg file only */
                else
                    usage();
                break;
            case '?':
            default:
                usage();
                break;
        }
    }
}

void enableCoredumps(void)
{
    /* Set Linux DUMPABLE flag */
    if (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0) != 0) {
        fprintf(stderr, "prctl: %s\n", strerror(errno));
    }

    /* Make sure coredumps are not limited */
    struct rlimit rlim;
    if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
        rlim.rlim_cur = rlim.rlim_max;
        if (setrlimit(RLIMIT_CORE, &rlim) == 0) {
            fprintf(stderr, "Enable Core Dumps OK!\n");
            return;
        }
    }
    fprintf(stderr, "Enable Core Dump failed: %s\n",strerror(errno));
}

int reap_children(void)
{
    int         i, n;
    int        live;

    live = 0;
    for (i = 0; i < g_conf->last_process; i++) {
                //child[0] 26718 e:0 t:0 d:0 r:1 j:0
        fprintf(stderr,"child[%d] %d e:%d t:%d d:%d r:%d j:%d\n",
                       i,
                       g_conf->processes[i].pid,
                       g_conf->processes[i].exiting,
                       g_conf->processes[i].exited,
                       g_conf->processes[i].detached,
                       g_conf->processes[i].respawn,
                       g_conf->processes[i].just_spawn);

        if (g_conf->processes[i].pid == -1) {
            continue;
        }

        if (g_conf->processes[i].exited) {
            if (!g_conf->processes[i].detached) {
                for (n = 0; n < g_conf->last_process; n++) {
                    if (g_conf->processes[n].exited
                        || g_conf->processes[n].pid == -1)
                    {
                        continue;
                    }

                    fprintf(stderr,"detached:%d\n", g_conf->processes[n].pid);
                }
            }

            if (g_conf->processes[i].respawn
                && !g_conf->processes[i].exiting
                && !g_conf->terminate
                && !g_conf->quit)
            {
                if (spawn_process(g_conf->processes[i].proc, g_conf->processes[i].data, g_conf->processes[i].name, i) == -1)
                {
                    fprintf(stderr, "could not respawn %s\n", g_conf->processes[i].name);
                    continue;
                }

                live = 1;

                continue;
            }

            if (i == g_conf->last_process - 1) {
                g_conf->last_process--;

            } else {
                g_conf->processes[i].pid = -1;
            }

        } else if (g_conf->processes[i].exiting || !g_conf->processes[i].detached) {
            live = 1;
        }
    }

    return live;
}

pid_t spawn_process(spawn_proc_pt proc, void *data, char *name, int respawn)
{
    long     on;
    pid_t  pid;
    int  s;

    if (respawn >= 0) {
        s = respawn;
    } else {
        for (s = 0; s < g_conf->last_process; s++) {
            if (g_conf->processes[s].pid == -1) {
                break;
            }
        }

        if (s == MAX_PROCESSES) {
            fprintf(stderr, "no more than %d g_conf->processes can be spawned", MAX_PROCESSES);
            return -1;
        }
    }

    g_conf->process_slot = s;

    pid = fork();

    switch (pid) {
    case -1:
        fprintf(stderr, "fork() failed while spawning \"%s\" :%s", name, errno);
        return -1;

    case 0:
        pid = getpid();
        proc(data);
        break;

    default:
        break;
    }

    fprintf(stderr, "start %s %d\n", name, pid);

    g_conf->processes[s].pid = pid;
    g_conf->processes[s].exited = 0;

    if (respawn >= 0) {
        return pid;
    }

    g_conf->processes[s].proc = proc;
    g_conf->processes[s].data = data;
    g_conf->processes[s].name = name;
    g_conf->processes[s].exiting = 0;

    switch (respawn) {

    case PROCESS_NORESPAWN:
        g_conf->processes[s].respawn = 0;
        g_conf->processes[s].just_spawn = 0;
        g_conf->processes[s].detached = 0;
        break;

    case PROCESS_JUST_SPAWN:
        g_conf->processes[s].respawn = 0;
        g_conf->processes[s].just_spawn = 1;
        g_conf->processes[s].detached = 0;
        break;

    case PROCESS_RESPAWN:
        g_conf->processes[s].respawn = 1;
        g_conf->processes[s].just_spawn = 0;
        g_conf->processes[s].detached = 0;
        break;

    case PROCESS_JUST_RESPAWN:
        g_conf->processes[s].respawn = 1;
        g_conf->processes[s].just_spawn = 1;
        g_conf->processes[s].detached = 0;
        break;

    case PROCESS_DETACHED:
        g_conf->processes[s].respawn = 0;
        g_conf->processes[s].just_spawn = 0;
        g_conf->processes[s].detached = 1;
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

    init_config();

    mainParseOptions(argc, argv);
    if (-1 == g_conf->opt_send_signal) {
        if (checkRunningPid()) {
            exit(1);
        }
    }

    enableCoredumps();
    writePidFile();
    save_argv(argc, argv);
    init_setproctitle();
    init_signals();
    sigemptyset(&set);

    printf("father pid1=%d\n",getpid());
    int worker_processes = 3;
    start_worker_processes(worker_processes, PROCESS_RESPAWN);

    start_dispatcher_process(PROCESS_RESPAWN);
    printf("father pid2=%d\n",getpid());

    int live = 1;
    for (;;) {
        printf("father before suspend\n");
        sigsuspend(&set);
        printf("father after suspend\n");
        if (g_conf->reap) {
            g_conf->reap = 0;
            fprintf(stderr, "reap children\n");
            live = reap_children();
        }
    }
    return 0;
}
