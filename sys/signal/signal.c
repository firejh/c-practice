
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int quit = 0;

void sig_handler(int sig)
{
    printf("process %d got sig %d\n", getpid(), sig);
    switch (sig) {
        case SIGINT:
            quit = 1;
            break;
    }
}

void sig_child(int sig)
{
    printf("get child signal %d %d\n", SIGCHLD, sig);
}

void sub_process_run()
{
    // signal(SIGINT, sig_handler);
    // signal(SIGTERM, sig_handler);
    // signal(SIGHUP, SIG_IGN);

    for (;;) {
        printf("loop\n");
        if (quit) {
            printf("process %d exit now\n", getpid());
            return;
        }
        sleep(15);
    }
}

int main()
{
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGCHLD, sig_child);
    signal(SIGHUP, SIG_IGN);

    pid_t pid = fork();
    if (pid < 0) {
        printf("error");
    } else if(pid == 0) {
        printf("sub process");
        sub_process_run();
        exit(0);
        return 0;
    }

    printf("parent %d, got sub process %d\n", getpid(), pid);
    int status;
    int wpid = waitpid(pid, &status, 0);
    printf("wait pid %d, status:%d\n", wpid, status);

    return 0;
}
