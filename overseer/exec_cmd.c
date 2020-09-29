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

void handler (int sig) {

}

int main(int argc, char **argv) {
    char current_time[TIME_BUFFER];

    int exec_timeout = atoi(argv[1]);
    int term_timeout = atoi(argv[2]);

    int outFile = 0, logFile = 0;

    if (strcmp(argv[3], "")) {
        if ((outFile = open("test",O_CREAT | O_APPEND | O_WRONLY, 0644)) == -1) {
            perror("open");
        }
    }

    if (strcmp(argv[4], "")) {
        if ((logFile = open(argv[4],O_CREAT | O_APPEND | O_WRONLY,
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
        strcat(file_args,argv[i]);
    }

    /* fork info */
    pid_t pid;
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
        /* add alarm handler */
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);
        sa.sa_handler = handler;
        sigaction(SIGALRM, &sa, NULL);

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

            /* pause the parent until either child terminated or timeout */
            alarm(exec_timeout);
        } else {
            printf("%s - could not execute %s - Error: %s\n",
                   get_time(current_time), file_args, strerror(buf));
        }
        close(pipe_fds[0]);

        /* In here, the code will be continue by either timout or child signal.
         * We need to use waitpid to get the status of the child to see if it exited or not*/
        result = waitpid(pid, &status, 0);
        if (result == -1) {
            if (errno == EINTR) { /* timeout, child is still running */
                printf("%s - sent SIGTERM to %d\n", get_time(current_time), pid);
                kill(pid, SIGTERM);

                /* alarm 5 seconds after sigterm */
                alarm(term_timeout);

                result = waitpid(pid, &status, 0);
                if (result == -1) {
                    if (errno == EINTR) { /* timeout, child is still running */
                        printf("%s - sent SIGKILL to %d\n", get_time(current_time), pid);
                        kill(pid, SIGKILL);

                        /* final test */
                        result = waitpid(pid, &status, 0);
                        if (result == -1) {
                            perror("waitpid");
                        }
                    } else {
                        perror("waitpid");
                    }
                } else {
                    printf("%s - %d has terminated with status code %d\n",
                           get_time(current_time), pid, WEXITSTATUS(status));
                }
            } else {
                perror("waitpid");
            }
        } else {
            if (executed) {
                printf("%s - %d has terminated with status code %d\n",
                       get_time(current_time), pid, WEXITSTATUS(status));
            }
        }

        exit(EXIT_SUCCESS);
    }
}