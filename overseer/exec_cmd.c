//
// Created by duchi on 9/29/2020.
//
#define _GNU_SOURCE

#include <sys/prctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <signal.h>
#include <helpers.h>
#include <wait.h>
#include <pthread.h>
#include <pty.h>
#include <utmp.h>

pid_t pid = 0; /* pid to store the pid of the children */
pthread_t tid; /* store tid of the thread */
FILE *outFile, *logFile; /* outfile and logfile stream */

/**
 * signal handler
 * @param sig received signal
 * @param siginfo signal info
 * @param context stack context
 */
void handler(int sig, siginfo_t *siginfo, void *context) {
    if (sig == SIGINT) {
        /* kill and wait for child to terminate */
        if (pid) {
            int status;
            fprintf(logFile, "%s - sent SIGKILL to %d\n", get_time(), pid);
            kill(pid, SIGKILL);

            waitpid(pid, &status, 0);
            fprintf(logFile, "%s - %d has terminated with status code %d\n",
                    get_time(), pid, WEXITSTATUS(status));
        }
    }
}

/**
 * print output of child to the outFile stream
 * @param data data passed from thread
 */
void exec_output(void *data) {
    /* master file descriptor */
    int master = *(int *) data;

    /* set timeout to 1 millisecond */
    struct timeval timeout = {
            .tv_sec = 0,
            .tv_usec = 1e3
    };

    /* setup file descriptor set for reading */
    fd_set read_fds;

    /* iterate and keep writing child's output to outfile */
    while (1) {
        /* reset file_descriptor set */
        FD_ZERO(&read_fds);
        FD_SET(master, &read_fds);

        /* read from child with timeout of 1 */
        int ret = select(master + 1, &read_fds, NULL, NULL, &timeout);
        if (ret > 0) {
            /* there is something to read */
            /* create buffer and reset it */
            char buff[MAX_BUFFER];
            memset(buff, 0, sizeof(buff));
            if (read(master, buff, sizeof(buff)) > 0) {
                fputs(buff, outFile);
                continue;
            }

            /* read error */
            break;
        } else if (ret < 0) {
            /* select error */
            break;
        }
    }
}

/**
 * main function
 * @param argc number of arguments from cli
 * @param argv array of arguments from cli
 * @return exit successfully or failed
 */
int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0); /* set no buffer for stdout */
    setvbuf(stderr, NULL, _IONBF, 0); /* set no buffer for stderr */

    /* add int handler (kill child process when sigint is sent) */
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &sa, NULL);

    /* convert parent died signal to sigint in case parent died because of errors or segmentation fault.
     * This is a good way to clean all of the child processes during development */
    int r = prctl(PR_SET_PDEATHSIG, SIGINT);
    if (r == -1) {
        /* if error, print error, cleanup and exit */
        perror("prctl");
        exit(EXIT_FAILURE);
    }
    // test in case the original parent exited just
    // before the prctl() call
    if (getppid() == 1) {
        printf("Parent has died, terminating...");
        exit(EXIT_FAILURE);
    }

    /* exec and termination time out */
    struct timespec exec_timeout = {
            .tv_sec = strtol(argv[1], NULL, BASE10),
            .tv_nsec = 0
    }, term_timeout = {
            .tv_sec = strtol(argv[2], NULL, BASE10),
            .tv_nsec = 0
    };

    /* initialize outfile and logfile to be stdout by default*/
    outFile = stdout, logFile = stdout;

    /* open logfile and outfile if exist */
    if (strcmp(argv[3], "") != 0) {
        if (!(outFile = fopen(argv[3], "w"))) {
            perror("fopen outfile");
        }
    }

    if (strcmp(argv[4], "") != 0) {
        if (!(logFile = fopen(argv[4], "w"))) {
            perror("fopen logfile");
        }
    }

    /* shift the argument variable to the start of the file executable */
    argv += 5;
    argc -= 5;

    /* concat file arguments into string */
    char file_args[256];
    strcpy(file_args, argv[0]);

    for (int i = 1; i < argc; i++) {
        strcat(file_args, " ");
        strcat(file_args, argv[i]);
    }

    /* setup pty */
    int master, slave;
    openpty(&master, &slave, NULL, NULL, NULL);

    /* pipe info to signal the parent of successful executed child */
    int buf, n;
    int pipe_fds[2];
    pipe2(pipe_fds, O_CLOEXEC);

    /* block child terminating signal before forking in case the child terminated
     * before being consumed by sigtimedwait */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    /* fork info */
    int status, result;
    pid = fork();
    if (pid == -1) {
        perror("fork");
    } else if (pid == 0) { /* child */
        /* login tty as slave and close master file descriptor */
        login_tty(slave);
        close(master);

        /* close the reading side of the pipe */
        close(pipe_fds[0]);

        /* execute the file */
        execv(argv[0], argv);

        /* write to the pipe the error code */
        write(pipe_fds[1], &errno, sizeof(errno));

        /* cleanup and send error code if exec failed */
        fclose(outFile);
        fclose(logFile);
        _exit(errno);
    }
    /* parent */
    /* close the slave file descriptor */
    close(slave);

    /* inform of file execution */
    fprintf(logFile, "%s - attempting to execute %s\n", get_time(), file_args);

    /* wait 1 seconds before continuing */
    sleep(1);

    /* create a thread and pass in master file descriptor
     * for writing output for the exec process */
    pthread_create(&tid, NULL, (void *(*)(void *)) exec_output, &master);

    /* get the pipe's data to know if there's any error occurred with execv */
    close(pipe_fds[1]); /* close the writing side of the pipe */
    n = read(pipe_fds[0], &buf, sizeof(buf));
    if (n == 0) { /* nothing in pipe -> successfully executed */
        /* inform of successful execution */
        fprintf(logFile, "%s - %s has been executed with pid %d\n",
                get_time(), file_args, pid);

        /* pause the parent until either child terminated or timeout */
        sigtimedwait(&set, NULL, &exec_timeout);
        /* if interrupt by sigint, child would have been terminated, jump to clean up and exit */
        if (errno == EINTR) goto exit;
    } else {
        fprintf(logFile, "%s - could not execute %s - Error: %s\n",
                get_time(), file_args, strerror(buf));

        /* clean up and exit */
        goto exit;
    }
    close(pipe_fds[0]);

    /* In here, the code will be continue by either timeout or child signal.
    * We need to use waitpid to get the status of the child to see if it exited or not */
    result = waitpid(pid, &status, WNOHANG);
    if (result == 0) { /* timeout, child is still running */
        fprintf(logFile, "%s - sent SIGTERM to %d\n", get_time(), pid);
        kill(pid, SIGTERM);

        /* wait for child signal until timeout */
        sigtimedwait(&set, NULL, &term_timeout);
        /* if interrupt by sigint, child would have been terminated, jump to clean up and exit */
        if (errno == EINTR) goto exit;

        result = waitpid(pid, &status, WNOHANG);
        if (result == 0) { /* timeout, child is still running  */
            fprintf(logFile, "%s - sent SIGKILL to %d\n", get_time(), pid);
            kill(pid, SIGKILL);

            /* final check */
            result = waitpid(pid, &status, 0);
            if (result < 0) {
                perror("waitpid");
            } else {
                fprintf(logFile, "%s - %d has terminated with status code %d\n",
                        get_time(), pid, WEXITSTATUS(status));
            }
        } else if (result > 0) { /* child has finished*/
            fprintf(logFile, "%s - %d has terminated with status code %d\n",
                    get_time(), pid, WEXITSTATUS(status));
        } else {
            perror("waitpid");
        }
    } else if (result > 0) { /* child has finished */
            fprintf(logFile, "%s - %d has terminated with status code %d\n",
                    get_time(), pid, WEXITSTATUS(status));
    } else {
        perror("waitpid");
    }

    /* clean up and exit */
    exit:
    {
        pthread_join(tid, NULL);
        fclose(outFile);
        fclose(logFile);
        exit(EXIT_SUCCESS);
    };
}