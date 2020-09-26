//
// Created by n10327622 on 12/09/2020.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <memory.h>
#include <helpers.h>
#include <arpa/inet.h>


void send_cmd(int, cmd_t *);
void send_flag(int, flag_t *);

int main(int argc, char **argv) {
    int sockfd; /* socket file descriptor */
    struct sockaddr_in serverAddr; /* server address's information */
    cmd_t *cmd_arg; /* store arguments and options */

    /* handle the arguments */
    cmd_arg = handle_args(argc, argv);

    /* set up the socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket\n");
        exit(EXIT_FAILURE);
    }

    /* set server's address information */
    serverAddr.sin_addr = cmd_arg->host_addr; /* server address */
    serverAddr.sin_port = htons(cmd_arg->port); /* server port */
    serverAddr.sin_family = AF_INET; /* ipv4 family */
    memset(&serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero)); /* pad 0s to sin_zero partition of the struct */

    /* connect to server */
    if (connect(sockfd, (struct sockaddr *) &serverAddr, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "Could not connect to overseer at %s %d\n",
                inet_ntoa(cmd_arg->host_addr), cmd_arg->port);
        exit(EXIT_FAILURE);
    }

    /* send command set to server */
    send_cmd(sockfd, cmd_arg);

    /* close connection and exit */
    close(sockfd);
    exit(EXIT_SUCCESS);
}

void send_cmd(int sockfd, cmd_t *cmd_arg) {
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
    /* send if flag exist or not */
    for (int i = 0; i < cmd_arg->flag_size; i++) {
        if (cmd_arg->flag_arg) send_flag(sockfd, cmd_arg->flag_arg + i);
    }

    /* send file arguments */
    /* send file size */
    uint32_t file_size = htonl(cmd_arg->file_size);
    if (send(sockfd, &file_size, sizeof(file_size), 0) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    /* send file arguments */
    for (int i = 0; i < cmd_arg->file_size; i++) {
        send_str(sockfd, cmd_arg->file_arg[i]);
    }
}

void send_flag(int sockfd, flag_t *flag_arg) {
    /* send type */
    uint32_t type = htonl(flag_arg->type);
    if (send(sockfd, &type, sizeof(type), 0) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    /* send value */
    /* send if value argument exist (in case of optional argument) */
    uint16_t value_exist = flag_arg->value ? 1 : 0;
    uint16_t netLen = htons(value_exist);
    if (send(sockfd, &netLen, sizeof(netLen), 0) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    if (value_exist) send_str(sockfd, flag_arg->value);
}