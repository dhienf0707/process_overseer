//
// Created by duchi on 9/29/2020.
//
#define _GNU_SOURCE

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <memory.h>
#include <errno.h>
#include <signal.h>
#include <helpers.h>
#include <wait.h>

pid_t pid;

void handler(int sig) {
    if (sig == SIGUSR1) {
        if (!kill(pid, SIGKILL)) perror("Killed");
        printf("Sent SIGKILL to pid %d\n", pid);
    }
}

int main(int argc, char **argv) {
    char current_time[TIME_BUFFER];

    struct timespec exec_timeout = {atoi(argv[1]), 0},
            term_timeout = {atoi(argv[2]), 0};

    int outFile = 0, logFile = 0;

    if (strcmp(argv[3], "")) {
        if ((outFile = open("test", O_CREAT | O_APPEND | O_WRONLY, 0644)) == -1) {
            perror("open");
        }
    }

    if (strcmp(argv[4], "")) {
        if ((logFile = open(argv[4], O_CREAT | O_APPEND | O_WRONLY,
                            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
            perror("open");
        }
    }


    argv += 5;
    argc -= 5;

    /* concat file arguments into string */
    char file_args[256];
    strcpy(file_args, argv[0]);

    for (int i = 1; i < argc; i++) {
        strcat(file_args, " ");
        strcat(file_args, argv[i]);
    }

    /* fork info */
    int status, result;

    /* pipe info to signal the parent of successful executed child */
    int buf, n;
    int pipe_fds[2];
    pipe2(pipe_fds, O_CLOEXEC);
    bool executed = false;

    pid = fork();
    if (pid == -1) {
        perror("fork");
    } else if (pid == 0) { /* child */
        /* set pgid to be parent */

        /* ignore sigusr1 */
        signal(SIGUSR1, SIG_IGN);

        /* duplicate outfile descriptor onto stdout and stdin if exist */
        if (outFile) {
            dup2(outFile, STDOUT_FILENO);
            dup2(outFile, STDERR_FILENO);
        }

        /* close the reading side of the pipe*/
        close(pipe_fds[0]);

        /* execute the file */
        execv(argv[0], argv);

        /* write to the pipe the error code */
        write(pipe_fds[1], &errno, sizeof(errno));

        /* send error code if exec failed */
        _exit(errno);
    } else { /* parent */
        /* add SIGCHLD handler */
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);
        sa.sa_handler = handler;
        sigaction(SIGCHLD, &sa, NULL);
        sigaction(SIGUSR1, &sa, NULL);

        /* set of signal to wait for.
         * specifically we need to wait for child termination signal */
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGCHLD);

        /* log management to logfile if exist*/
        if (logFile) dup2(logFile, STDOUT_FILENO);

        /* inform of file execution */
        printf("%s - attempting to execute %s\n", get_time(current_time), file_args);

        /* get the pipe's data to know if there's any error occurred with execv */
        close(pipe_fds[1]);
        n = read(pipe_fds[0], &buf, sizeof(buf));
        if (n == 0) { /* nothing in pipe -> successfully executed */
            executed = true;
            sleep(1);
            printf("%s - %s has been executed with pid %d\n", get_time(current_time), file_args, pid);

            /* wait for child signal until timeout */
            sigtimedwait(&set, NULL, &exec_timeout);
        } else {
            printf("%s - could not execute %s - Error: %s\n",
                   get_time(current_time), file_args, strerror(buf));
        }
        close(pipe_fds[0]);

        /* In here, the code will be continue by either timout or child signal.
         * We need to use waitpid to get the status of the child to see if it exited or not*/
        printf("PID: %d\n", pid);
        result = waitpid(pid, &status, WNOHANG);
        if (result == 0) { /* timeout, child is still running */
            printf("%s - sent SIGTERM to %d\n", get_time(current_time), pid);
            kill(pid, SIGTERM);
            printf("Result: %d\n", result);
            /* wait for child signal until timeout */
            sigtimedwait(&set, NULL, &term_timeout);

            result = waitpid(pid, &status, WNOHANG);
            if (result == 0) { /* timeout, child is still running  */
                printf("%s - sent SIGKILL to %d\n", get_time(current_time), pid);
                kill(pid, SIGKILL);

                /* final check */
                result = waitpid(pid, &status, 0);
                if (result < 0) {
                    perror("waitpid");
                }
            } else if (result > 0) { /* child has finished*/
                printf("%s - %d has terminated with status code %d\n",
                       get_time(current_time), pid, WEXITSTATUS(status));
            } else {
                perror("waitpid");
            }
        } else if (result > 0) { /* child has finished */
            if (executed) {
                printf("%s - %d has terminated with status code %d\n",
                       get_time(current_time), pid, WEXITSTATUS(status));
            }
        } else {
            perror("waitpid");
        }

        exit(EXIT_SUCCESS);
    }
}