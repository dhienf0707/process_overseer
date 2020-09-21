//
// Created by Asus on 9/12/2020.
//

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <netdb.h>
#include <getopt.h>
#include <helpers.h>

void print_usage(char *msg, enum usage type) {
    char *usage = "Usage: controller <address> <port> "
                  "{[-o out_file] [-log log_file] [-t seconds] <file> [arg...] | "
                  "mem [pid] | memkill <percent>}";

    if (type == help) {
        printf("\n%s\n%s\n", msg, usage);
    } else if (type == error) {
        fprintf(stderr, "\n%s\n%s\n", msg, usage);
    }
}

cmd *handle_args(int argc, char **argv) {
    struct hostent *he; /* host entry */
    uint16_t port; /* host's port */

    /* create flags for 3 command sets */
    flag *flag_arg = (flag *) malloc(sizeof(flag) * 3);

    /* variable to track what command set is used */
    cmd *cmd_arg = (cmd *) malloc(sizeof(cmd));
    int flag_size = 0;

    /* if there is only 2 argument and it's --help */
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        print_usage("Help Menu:", help);
        exit(EXIT_SUCCESS);
    }

    /* if not help there must be at least 4 arguments */
    if (argc < 4) {
        print_usage("Too few arguments", error);
        exit(EXIT_FAILURE);
    }

    /* get the address */
    if ((he = gethostbyname(argv[1])) == NULL) {
        herror("gethostbyname\n");
        exit(EXIT_FAILURE);
    }
    cmd_arg->host_addr = *((struct in_addr *) he->h_addr);

    /* get the port from second argument */
    port = atoi(argv[2]);
    cmd_arg->port = port;

    /* check if third argument is memkill or mem */
    if (strcmp(argv[3], "mem") == 0) {
        /* set up mem flag's value */
        flag_arg->type = mem;

        if (argv[4]) { /* get the optional argument */
            flag_arg->value = argv[4];
            flag_size++;
        }

        /* return the command */
        cmd_arg->type = cmd2;
        cmd_arg->flag_arg = flag_arg;
        cmd_arg->flag_size = flag_size;

        if (argc < 6) return cmd_arg;
        else {
            print_usage("Too many arguments", error);
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(argv[3], "memkill") == 0) {
        /* setup memkill flag */
        flag_arg->type = memkill;

        if (argv[4]) { /* get the required argument */
            flag_arg->value = argv[4];
            flag_size++;
        } else {
            print_usage("Please specify percentage for memkill", error);
            exit(EXIT_FAILURE);
        }

        /* return the command */
        cmd_arg->type = cmd3;
        cmd_arg->flag_arg = flag_arg;
        cmd_arg->flag_size = flag_size;

        if (argc < 6) return cmd_arg;
        else {
            print_usage("Too many arguments", error);
            exit(EXIT_FAILURE);
        }
    }

    /* When we get here we know that cmd set 2 and 3 is not set, we only consider cmd set 1 */

    opterr = 0; /* disable error message for getopt in case there is argument from the executable file */
    int ch, /* character value when iterating through argv */
    isFlag = 0; /* track if any flag in the first command group is set */
    int cmd1_args = 0; /* arguments counter for first command set to determine the position of the file */
    int oFlag = 0, lFlag = 0, tFlag = 0; /* position of flags in first command set */

    /* Executable file pointer */
    file *exec_file = (file *) malloc(sizeof(file));
    exec_file->size = 0;

    flag *first_arg = flag_arg; /* head of flag_arg array */

    /* A copy of argv (to reserve the order of the original argv) */
    char *argv_cpy[argc];
    for (int i = 0; i < argc; i++) {
        argv_cpy[i] = malloc(strlen(argv[i]) + 1);
        argv_cpy[i] = memcpy(argv_cpy[i], argv[i], strlen(argv[i]) + 1);
    }


    /* optstring for getopt */
    const char *const short_options = "o:t:";
    static struct option long_options[] = {
            {"log", required_argument, NULL, 'l'},
            {NULL, 0,                  NULL, 0}
    };

    while ((ch = getopt_long_only(argc, argv_cpy, short_options, long_options, NULL)) != -1) {
        switch (ch) {
            case 'o':
                /* create flag for output */
                flag_arg->type = out;
                flag_arg->value = optarg;
                flag_arg++;
                flag_size++;

                isFlag = 1; /* set the command set 1 to true */
                oFlag = optind - 1; /* set the position of output flag */
                cmd1_args += 2; /* increment argument counter for first command set */

                /* if log flag or time flag exist before o flag return error */
                if (lFlag || tFlag) {
                    print_usage("Wrong command syntax", error);
                    exit(EXIT_FAILURE);
                }

                break;
            case 'l':
                /* create flag for log */
                flag_arg->type = log;
                flag_arg->value = optarg;
                flag_arg++;
                flag_size++;

                isFlag = 1; /* set command set 1 to true */
                lFlag = optind - 1;  /* set the position of lflag */
                cmd1_args += 2; /* increment argument counter for command set 1*/

                /* if out flag is already created before its next flag is going to be log flag*/


                /* check if time flag exists or if
                 * there is anything between log flag and output flag if output flag exists*/
                if (tFlag || (oFlag && oFlag != lFlag - 2)) {
                    print_usage("Wrong command syntax", error);
                    exit(EXIT_FAILURE);
                }

                break;
            case 't':
                /* create flag for time */
                flag_arg->type = time;
                flag_arg->value = optarg;
                flag_arg++;
                flag_size++;

                isFlag = 1; /* set the first command set to true */
                tFlag = optind - 1; /* store the position of the time flag */
                cmd1_args += 2; /* increment the argument counter of first command set */

                /* check if there is anything between the time flag and any other previous flags */
                if (oFlag && lFlag) { /* if output flag and log flag exist */
                    if (oFlag != tFlag - 4 && lFlag != tFlag - 2) { /* check if it's in right order */
                        print_usage("Wrong command syntax", error);
                        exit(EXIT_FAILURE);
                    }
                } else if (oFlag && oFlag != tFlag - 2) { /* if only output flag exist */
                    print_usage("Wrong command syntax", error);
                    exit(EXIT_FAILURE);
                } else if (lFlag && lFlag != tFlag - 2) { /* if only log flag exist */
                    print_usage("Wrong command syntax", error);
                    exit(EXIT_FAILURE);
                }

                break;
            default:
                break;
        }
    }

    int file_index = cmd1_args + 3; /* index of file after retrieving all of the arguments */

    if (isFlag && oFlag != 4 && lFlag != 4 &&
        tFlag != 4) { /* if cmd 1 is set, flags must be in right position */
        print_usage("Wrong command syntax", error);
        exit(EXIT_FAILURE);
    }

    if (file_index == argc) { /* there must be file passing in the end of all option arguments */
        print_usage("Please specify file to run", error);
        exit(EXIT_FAILURE);
    }

    /* set up the file arguments */
    exec_file->size = argc - file_index;
    exec_file->arg = argv + file_index;


    /* set up first command group and return */
    cmd_arg->type = cmd1;
    cmd_arg->flag_arg = first_arg;
    cmd_arg->flag_size = flag_size;
    cmd_arg->file = exec_file;
}

void send_str(int sockfd, char *msg) {
    /* send the length of the string */
    int msgLen = strlen(msg) + 1;
    uint32_t netLen = htonl(msgLen);
    if (send(sockfd, &netLen, sizeof(netLen), 0) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    /* send the message */
    if (send(sockfd, msg, msgLen, 0) != msgLen) {
        fprintf(stderr, "CLIENT: send did not send all data\n");
        exit(EXIT_FAILURE);
    }
}

char *recv_str(int client_fd) {
    /* get the length of the message*/
    uint32_t netLen;
    if (recv(client_fd, &netLen, sizeof(netLen), 0) != sizeof(netLen)) {
        fprintf(stderr, "recv got invalid len value\n");
        return NULL;
    }
    int msgLen = ntohl(netLen);

    /* get the message */
    char *msg = (char *) malloc(sizeof(char) * msgLen);
    if (recv(client_fd, msg, msgLen, 0) != msgLen) {
        fprintf(stderr, "recv got invalid message\n");
        return NULL;
    }

    return msg;
}