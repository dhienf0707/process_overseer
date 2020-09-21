//
// Created by n10327622 on 12/09/2020.
//

#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <memory.h>
#include <zconf.h>
#include <helpers.h>
#include <arpa/inet.h>


void send_cmd(int, cmd *);
void send_flag(int, flag *);
void send_file(int, file *);

int main(int argc, char **argv) {
    int sockfd; /* socket file descriptor */
    struct sockaddr_in serverAddr; /* server address's information */
    cmd *cmd_arg; /* store arguments and options */

    /* handle the arguments */
    cmd_arg = handle_args(argc, argv);

    /* set up the socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket\n");
        exit(EXIT_FAILURE);
    }
    printf("CLIENT: got connection from %s\n", inet_ntoa(cmd_arg->host_addr));

    /* set server's address information */
    serverAddr.sin_addr = cmd_arg->host_addr; /* server address */
    serverAddr.sin_port = htons(cmd_arg->port); /* server port */
    serverAddr.sin_family = AF_INET; /* ipv4 family */
    memset(&serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero)); /* pad 0s to sin_zero partition of the struct */

    /* connect to server */
    if (connect(sockfd, (struct sockaddr *) &serverAddr, sizeof(struct sockaddr)) == -1) {
        perror("connect\n");
        exit(EXIT_FAILURE);
    }

    /* send command set to server */
    send_cmd(sockfd, cmd_arg);

    /* test string */
    char *msg = "CLIENT: hi this is client, how are you?\n";
    send_str(sockfd, msg);

    /* close connection and exit */
    close(sockfd);
    exit(EXIT_SUCCESS);
}

void send_cmd(int sockfd, cmd *cmd_arg) {
    /* send command type */

    uint32_t type = htonl(cmd_arg->type);
    if (send(sockfd, &type, sizeof(type), 0) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    /* send flags */
    /* send flag size */
    uint32_t flag_size = htonl(cmd_arg->flag_size);
    if (send(sockfd, &flag_size, sizeof(flag_size), 0) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    /* send flags arguments */
    for (int i = 0; i < cmd_arg->flag_size; i++) {
        send_flag(sockfd, cmd_arg->flag_arg + i);
    }

    /* send file arguments */
    /* send file size */
    uint32_t file_size = htonl(cmd_arg->file->size);
    if (send(sockfd, &file_size, sizeof(file_size), 0) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    /* send file arguments */
    for (int i = 0; i < cmd_arg->file->size; i++) {
        send_str(sockfd, cmd_arg->file->arg[i]);
    }
}

void send_flag(int sockfd, flag *flag_arg) {
    /* send type */
    uint32_t type = htonl(flag_arg->type);
    if (send(sockfd, &type, sizeof(type), 0) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    /* send value */
    /* send length of the value first */
    send_str(sockfd, flag_arg->value);
}