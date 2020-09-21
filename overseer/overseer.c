//
// Created by n10327622 on 12/09/2020.
//

#include <z3.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <memory.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <helpers.h>

#define BACKLOG 10

bool recv_cmd(int, cmd *);
bool recv_flag(int, flag *);

int main(int argc, char **argv) {
    int server_fd, client_fd;
    uint16_t port;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;
    cmd *cmd_arg = (cmd *) malloc(sizeof(cmd)); /* command group's information */

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

        printf("Server: got connection from %s\n", inet_ntoa(client_addr.sin_addr));

        /* receive command from client */
        if (!recv_cmd(client_fd, cmd_arg));

        printf("SERVER: receive argument flag size %d\n", cmd_arg->flag_size);
        for (int i = 0; i < cmd_arg->flag_size; i++) {
            printf("flag %d: has value %s\n", cmd_arg->flag_arg[i].type, cmd_arg->flag_arg[i].value);
        }

        printf("SERVER: receive file with arguments: ");
        for (int i = 0; i < cmd_arg->file->size; i++) {
            printf("%s ", cmd_arg->file->arg[i]);
        }
        printf("\n");

        /* test string */
        char *msg = recv_str(client_fd);
        printf("%s", msg);

        /* close connection */
        close(client_fd);
    }
}

bool recv_cmd(int client_fd, cmd *cmd_arg) {
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
    cmd_arg->flag_arg = (flag *) malloc(sizeof(flag) * 3);
    for (int i = 0; i < cmd_arg->flag_size; i++) {
        if (!recv_flag(client_fd, cmd_arg->flag_arg + i)) return false;
    }

    /* receive file arguments */
    /* receive file size */
    cmd_arg->file = (file *) malloc(sizeof(file));
    uint32_t file_size;
    if (recv(client_fd, &file_size, sizeof(file_size), 0) != sizeof(file_size)) {
        fprintf(stderr, "recv got invalid size value\n");
        return false;
    }
    cmd_arg->file->size = ntohl(file_size);

    /* receive file arguments */
    cmd_arg->file->arg = (char **) calloc(cmd_arg->file->size, sizeof(char *));
    for (int i = 0; i < cmd_arg->file->size; i++) {
        if (!(cmd_arg->file->arg[i] = recv_str(client_fd))) {
            fprintf(stderr, "error receiving file arguments\n");
            return false;
        }
    }

    return true;
}

bool recv_flag(int client_fd, flag *flag_arg) {
    /* receive type of flag */
    uint32_t flag_type;
    if (recv(client_fd, &flag_type, sizeof(flag_type), 0) != sizeof(flag_type)) {
        fprintf(stderr, "recv got invalid flag type value\n");
        return false;
    }
    flag_arg->type = ntohl(flag_type);

    /* receive flag value */
    if (!(flag_arg->value = recv_str(client_fd))) return false;

    return true;
}