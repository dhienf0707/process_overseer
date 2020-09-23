#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <helpers.h>
#include <unistd.h>

//
// Created by duchi on 9/19/2020.
//

void test_func(int *);

int main(int argc, char **argv) {
    char buffer[26];
    char *args[8] = {"./controller/controller", "localhost", "3000", "-o", "outfile", "hello", "0", NULL};
    pid_t pid;

    pid = fork();
    if (pid == 0) goto fork2;
    else args[6] = "1";

    fork2: { pid = fork(); };
    if (pid == 0) goto fork3;
    else args[6] = "2";

    fork3: { fork(); };

    execv(args[0], args);

    exit(1);
}

void test_func(int *ptr) {
    ptr = (int *) malloc(sizeof(int));
    *ptr = 15;
}
