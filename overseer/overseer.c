//
// Created by n10327622 on 12/09/2020.
//

#define _GNU_SOURCE

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <netinet/in.h>
#include <memory.h>
#include <arpa/inet.h>
#include <helpers.h>
#include <wait.h>
#include <errno.h>
#include <pthread.h>

#define BACKLOG 10
#define NUM_THREADS 5
#define MAX_BUFFER 256

bool recv_cmd(int, cmd_t *);

bool recv_flag(int, flag_t *);

void process_cmd(cmd_t *);

void exec_cmd1(cmd_t *);

/* global mutex for our program. */
pthread_mutex_t request_mutex;

/* global condition variable for our program. */
pthread_cond_t got_request;

/* create request struct */
typedef struct request {
    cmd_t *cmd_arg;
    struct request *next;
} request_t;

void handle_request(request_t *);

void *handle_requests_loop(void *);

void add_request(cmd_t *pCmd, pthread_mutex_t *ptr, pthread_cond_t *ptr1);

char current_time[TIME_BUFFER];

request_t *requests = NULL; /* head of linked list of requests */
request_t *last_request = NULL; /* pointer to the last request */
int num_request = 0; /* number of pending requests, initially none */

int main(int argc, char **argv) {
    int server_fd, client_fd;
    uint16_t port;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;
    cmd_t *cmd_arg = (cmd_t *) malloc(sizeof(cmd_t)); /* command group's information */
    pthread_t p_threads[NUM_THREADS];

    /* initialize the mutex and condition variable */
    pthread_mutex_init(&request_mutex, NULL);
    pthread_cond_init(&got_request, NULL);


    /* create the request-handling threads */
    for (int i = 0; i < NUM_THREADS; i++) {

        pthread_create(&p_threads[i], NULL, (void *(*)(void *)) handle_requests_loop, NULL);
    }

    /* check for arguments */
    if (argc != 2) {
        fprintf(stderr, "usage: overseer <port>\n");
        exit(EXIT_FAILURE);
    }

    /* get port number to listen on */
    port = atoi(argv[1]);

    /* set up socket */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* enable address and port reuse */
    int opt_enable = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
               &opt_enable, sizeof(opt_enable));

    /* setup endpoint */
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;
    memset(&server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    /* bind the socket to the end point */
    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    /* start listening */
    if (listen(server_fd, BACKLOG)) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server starts listening on port %u...\n", port);

    /* repeat: accept, execute, close connection */
    while (1) {
        sin_size = sizeof(struct sockaddr_in);

        /* accept connection */
        if ((client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }

        printf("%s - connection received from %s\n", get_time(current_time), inet_ntoa(client_addr.sin_addr));

        /* receive command from client */
        if (!recv_cmd(client_fd, cmd_arg));

        /* add request to the linked list */
        add_request(cmd_arg, &request_mutex, &got_request);

        /* close connection */
        close(client_fd);
    }
}

void add_request(cmd_t *cmd_arg, pthread_mutex_t *p_mutex, pthread_cond_t *p_cond_var) {
    //TODO
    request_t *a_request; /* pointer to newly added request */

    /* create a new request */
    a_request = (request_t *) malloc(sizeof(request_t));
    if (!a_request) {
        fprintf(stderr, "add_request: out of memory\n");
    }

    a_request->cmd_arg = cmd_arg;
    a_request->next = NULL;

    /* modify the linked list of requests */
    pthread_mutex_lock(p_mutex); /* get exclusive access to the list */

    /* add the request to the end of the list */
    if (!num_request) { /* the request list is empty */
        requests = a_request;
        last_request = a_request;
    } else {
        last_request->next = a_request;
        last_request = a_request;
    }

    /* increase the total of pending requests */
    num_request++;

    /* unlock the mutex */
    pthread_mutex_unlock(p_mutex);

    /* signal the condition variable */
    pthread_cond_signal(p_cond_var);

}

request_t *get_request() {
    request_t *a_request; /* pointer to a request */

    /* get request from the head of the list */
    a_request = requests;
    requests = a_request->next;

    /* if request is the last request on the list */
    if (requests == NULL) {
        last_request = NULL;
    }

    /* decrement the number of pending requests */
    num_request--;

    /* return the request to the caller */
    return a_request;
}

void handle_request(request_t *a_request) {
    process_cmd(a_request->cmd_arg);
}

void *handle_requests_loop(void *data) {
    //TODO
    request_t *a_request; /* pointer to a request */

    /* do forever... */
    while (1) {
        pthread_mutex_lock(&request_mutex); /* get exclusive access to the list */

        /* this combine with while (1) will create waiting loop if num_request is 0 */
        if (num_request == 0) {
            pthread_cond_wait(&got_request, &request_mutex);
        }

        /* get request */
        a_request = get_request();

        /* unlock for other threads to get request */
        pthread_mutex_unlock(&request_mutex);

        /* handle request and free request pointer */
        handle_request(a_request);

        /* free cmd_args elements */
        /* free file args if exist (mem and memkill doesn't have file specified) */
        if (a_request->cmd_arg->file_size) {
            for (int i = 0; i < a_request->cmd_arg->file_size; i++) {
                free(a_request->cmd_arg->file_arg[i]);
            }
            free(a_request->cmd_arg->file_arg);
        }


        /* free flag args and its value if exist
         * Note that some flag doesn't have value (mem and memkill) */
        if (a_request->cmd_arg->flag_arg->value) free(a_request->cmd_arg->flag_arg->value);
        free(a_request->cmd_arg->flag_arg);

        /* free cmd_arg */
        free(a_request->cmd_arg);

        /* free a_request */
        free(a_request);
    }

}

void process_cmd(cmd_t *cmd_arg) {
    switch (cmd_arg->type) {
        case cmd1:
            exec_cmd1(cmd_arg);
            break;
        case cmd2:
            break;
        case cmd3:
            break;
    }
}

void exec_cmd1(cmd_t *cmd_arg) {
    pid_t pid;
    int status;
    int outFile = 0, logFile = 0, term_time = 10;

    /* inform to execute the file */
    char *file_args = (char *) malloc(sizeof(char) * MAX_BUFFER);
    strcpy(file_args, cmd_arg->file_arg[0]);

    for (int i = 1; i < cmd_arg->file_size; i++) {
        strcat(file_args, " ");
        strcat(file_args, cmd_arg->file_arg[i]);
    }

    /* process flags */
    for (int i = 0; i < cmd_arg->flag_size; i++) {
        switch (cmd_arg->flag_arg[i].type) {
            case o:
                if ((outFile = open(cmd_arg->flag_arg[i].value,
                                    O_CREAT | O_APPEND | O_WRONLY,
                                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
                    perror("open");
                }
                break;
            case log:
                if ((logFile = open(cmd_arg->flag_arg[i].value,
                                    O_CREAT | O_TRUNC | O_WRONLY,
                                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
                    perror("open");
                }
                break;
            case t:
                term_time = atoi(cmd_arg->flag_arg[i].value);
                break;
        }
    }


    /* create pipe to signal the parent of successful executed program */
    int buf, n;
    int pipe_fds[2];
    pipe2(pipe_fds, O_CLOEXEC);

    /* log management to logfile if exist*/
    int saved_stdout = dup(STDOUT_FILENO);
    if (logFile) dup2(logFile, STDOUT_FILENO);

    /* inform of file execution */
    printf("%s - attempting to execute %s\n", get_time(current_time), file_args);

    /* fork and execute file */
    switch (pid = fork()) {
        case -1:
            perror("fork");
            break;
        case 0:
            /* duplicate outfile descriptor onto stdout and stdin if exist */
            if (outFile) {
                dup2(outFile, STDOUT_FILENO);
                dup2(outFile, STDERR_FILENO);
            }

            /* close the reading side of the pipe*/
            close(pipe_fds[0]);

            /* execute the file */
            execv(cmd_arg->file_arg[0], cmd_arg->file_arg);

            /* write to the pipe the error code */
            write(pipe_fds[1], &errno, sizeof(errno));

            /* send error code if exec failed */
            exit(errno);
            break;
        default:
            close(pipe_fds[1]);
            n = read(pipe_fds[0], &buf, sizeof(buf));
            if (n == 0) { /* nothing in pipe */
                sleep(1);
                printf("%s - %s has been executed with pid %d\n", get_time(current_time), file_args, pid);
            }
            close(pipe_fds[0]);

            if (waitpid(pid, &status, 0) > 0) {
                if (WIFEXITED(status) && !WEXITSTATUS(status)) { /* terminated normally */
                    printf("%s - %d has terminated with status code %d\n",
                           get_time(current_time), pid, WEXITSTATUS(status));
                } else {
                    printf("%s - could not execute %s - Terminated with error: %s\n",
                           get_time(current_time), file_args, strerror(WEXITSTATUS(status)));
                }
            } else {
                perror("waitpid");
            }

            /* restore the log management to normal stdout */
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);

            /* free the file_args string*/
            free(file_args);

            break;
    }
}

bool recv_cmd(int client_fd, cmd_t *cmd_arg) {
    /* receive type of the command */
    uint32_t type;
    if (recv(client_fd, &type, sizeof(type), 0) != sizeof(type)) {
        fprintf(stderr, "recv got invalid size value\n");
        return false;
    }
    cmd_arg->type = ntohl(type);

    /* receive flag size */
    uint32_t flag_size;
    if (recv(client_fd, &flag_size, sizeof(flag_size), 0) != sizeof(flag_size)) {
        fprintf(stderr, "recv got invalid size value\n");
        return false;
    }
    cmd_arg->flag_size = ntohl(flag_size);

    /* receive all flags */
    cmd_arg->flag_arg = (flag_t *) malloc(sizeof(flag_t) * 3);
    for (int i = 0; i < cmd_arg->flag_size; i++) {
        if (!recv_flag(client_fd, cmd_arg->flag_arg + i)) return false;
    }

    /* receive file arguments */
    /* receive file size */
    uint32_t file_size;
    if (recv(client_fd, &file_size, sizeof(file_size), 0) != sizeof(file_size)) {
        fprintf(stderr, "recv got invalid size value\n");
        return false;
    }

    /* receive file arguments */
    cmd_arg->file_size = ntohl(file_size);
    cmd_arg->file_arg = (char **) malloc(sizeof(char *) * (cmd_arg->file_size + 1));
    for (int i = 0; i < cmd_arg->file_size; i++) {
        if (!(cmd_arg->file_arg[i] = recv_str(client_fd))) {
            fprintf(stderr, "error receiving file arguments\n");
            return false;
        }
    }
    /* add null pointer to the end of the array */
    cmd_arg->file_arg[cmd_arg->file_size] = NULL;

    return true;
}

bool recv_flag(int client_fd, flag_t *flag_arg) {
    /* receive type of flag */
    uint32_t flag_type;
    if (recv(client_fd, &flag_type, sizeof(flag_type), 0) != sizeof(flag_type)) {
        fprintf(stderr, "recv got invalid flag type value\n");
        return false;
    }
    flag_arg->type = ntohl(flag_type);

    /* receive flag value */
    /* receive if value exist first */
    uint16_t value_exist;
    if (recv(client_fd, &value_exist, sizeof(value_exist), 0) != sizeof(value_exist)) {
        fprintf(stderr, "recv got invalid exist flag's exist value");
        return false;
    }
    value_exist = ntohs(value_exist);

    /* receive the flag's value if value exist */
    if (value_exist) {
        if (!(flag_arg->value = recv_str(client_fd))) return false;
    } else {
        flag_arg->value = NULL;
    }

    return true;
}