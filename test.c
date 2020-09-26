#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <memory.h>
#include <helpers.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <errno.h>
#include <pthread.h>

//
// Created by duchi on 9/19/2020.
//


char current_time[TIME_BUFFER];

void *exec_hello(void *data) {
    char *time = (char *) data;
    char *args[] = {"./controller/controller", "localhost", "3000", "hello", time, NULL};
    execv(args[0], args);
}

int main(int argc, char **argv) {
    pid_t pid;
    char *args[] = {"./controller/controller", "localhost", "3000", "hello", "0", NULL};

    pid = fork();
    if (pid == 0) goto fork2;
    args[4] = "1";

    fork2: { pid = fork(); };
    if (pid == 0) goto fork3;
    args[4] = "2";

    fork3: { pid = fork(); };


    printf("Send '%s %s' to overseer", args[3], args[4]);
    execv(args[0], args);

    exit(EXIT_SUCCESS);
}
