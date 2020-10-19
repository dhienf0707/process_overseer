//
// Created by Asus on 9/12/2020.
//

#ifndef PROCESS_OVERSEER_HELPERS_H
#define PROCESS_OVERSEER_HELPERS_H
#define TIME_BUFFER 20
#define MAX_BUFFER 512

#include <netinet/in.h>

/* enum for option flag type */
enum flag_type {
    o, log, t, mem, memkill
};

/* create struct for flags */
typedef struct flag {
    enum flag_type type; /* position of the flag */
    char *value; /* value of the flag */
} flag_t;

/* enum for command type */
enum cmd_type {
    cmd1, cmd2, cmd3
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
    help, error
};

/* print usage if error or help from cli argument */
void print_usage(char *, enum usage);

/* handle commandline argument */
cmd_t *handle_args(int, char **);

/* send string over tcp/ip */
void send_str(int, char *);

/* receive string over tcp/ip */
char *recv_str(int);

/* return the current time in %Y-%m-%d %H:%M:%S format */
char *get_time(char *);

#endif //PROCESS_OVERSEER_HELPERS_H
