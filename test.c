#include <getopt.h>
#include <zconf.h>
#include <z3.h>
#include <stdlib.h>
#include <memory.h>

void print_usage(char *);

//
// Created by duchi on 9/19/2020.
//
typedef struct flag {
    int i; /* position of the flag */
    char *value; /* value of the flag */
} flag;

void test_func(int *);

int main(int argc, char **argv) {

    int *a = (int *) malloc(sizeof(int));
    *a = 10;
    test_func(a);
    printf("%d\n", *a);
}

void test_func(int *ptr) {
    ptr = (int *) malloc(sizeof(int));
    *ptr = 15;
}

void print_usage(char *type) {
    char *msg = "Usage: controller <address> <port> "
                "{[-o out_file] [-log log_file] [-t seconds] <file> [arg...] | "
                "mem [pid] | memkill <percent>}\n";
    if (strcmp(type, "help") == 0) {
        printf("%s", msg);
    } else {
        fprintf(stderr, "%s", msg);
    }
}
