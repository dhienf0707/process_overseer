//
// Created by Asus on 9/12/2020.
//

#ifndef PROCESS_OVERSEER_HELPERS_H
#define PROCESS_OVERSEER_HELPERS_H
#define TIME_BUFFER 20
#define MAX_BUFFER 256
#define MAX_ARRAY_SIZE 100
#define MAX_FLAG_SIZE 3
#define BASE10 10
#define BASE16 16
#define INODE_OFFSET 21 /* offset from 'address' to 'inode' in /proc/pid/maps */

#include <netinet/in.h>
#include <stdbool.h>

/* enum for option flag type */
enum flag_type {
    o = 1, log, t, mem, memkill
};

/* create struct for flags */
typedef struct flag {
    enum flag_type type; /* position of the flag */
    char *value; /* value of the flag */
} flag_t;

/* enum for command type */
enum cmd_type {
    cmd1 = 1, cmd2, cmd3
};

/* struct for command group argument */
typedef struct cmd {
    enum cmd_type type;
    uint16_t port;
    struct in_addr host_addr;
    int flag_size;
    flag_t *flag_arg;
    int file_size;
    char **file_arg;
} cmd_t;

/* enum for print usage (err vs help) */
enum usage {
    help = 1, error
};

/* print usage if error or help from cli argument */
void print_usage(char *, enum usage);

/* handle commandline argument */
void handle_args(int argc, char **argv, cmd_t *cmd_arg);

/* send string over tcp/ip */
bool send_str(int, char *);

/* receive string over tcp/ip */
char *recv_str(int);

/* return the current time in %Y-%m-%d %H:%M:%S format */
char *get_time();

/* get first child pid of given pid */
int get_child_pid(pid_t pid);

#endif //PROCESS_OVERSEER_HELPERS_H