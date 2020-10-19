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
#include <stdatomic.h>
#include <sys/sysinfo.h>

#define BACKLOG 10
#define NUM_THREADS 5
#define MAX_ARRAY_SIZE 100

/* create request struct */
typedef struct request {
    cmd_t *cmd_arg;
    struct request *next;
} request_t;

/* request pool global variables */
request_t *requests = NULL;     /* head of linked list of requests */
request_t *last_request = NULL; /* pointer to the last request */
int num_request = 0;            /* number of pending requests, initially none */
pthread_mutex_t request_mutex; /* global mutex for request pool */
pthread_cond_t got_request; /* global condition variabe for our program. */

/* add request to list */
request_t *add_request(cmd_t *pCmd, pthread_mutex_t *ptr, pthread_cond_t *ptr1);

/* get 1 request from list */
request_t *get_request();

/* handle requests loop for threads */
void *handle_requests_loop(void *);

/* process cmd1 */
void process_cmd1(cmd_t *cmd_arg);

/* process cmd2 */
void process_cmd2(cmd_t *cmd_arg, int client_fd);

/* process cmd3 */
void process_cmd3(cmd_t *cmd_arg);

/* create request struct */
typedef struct entry {
    pid_t pid;
    char current_time[TIME_BUFFER];
    unsigned int mem;
    int argc;
    char **argv;
    struct entry *next;
} entry_t;

/* entry global variables */
entry_t *entry = NULL;      /* head of linked list of entries */
entry_t *last_entry = NULL; /* pointer to the last entries */
int num_entry = 0;          /* number of process entry, initially none */
pthread_mutex_t entry_mutex; /* global mutex for entry pool */

/* get available memory */
unsigned long mem_avail(void);

/* get 1 entry from list */
entry_t *get_entry();

/* add entry to list */
entry_t *add_entry(pid_t pid, unsigned int mem, cmd_t *cmd_arg);

/* Print all processes that are runing */
void send_current_process(entry_t *node, char *mem_time, int client_fd);

/* Print information of a specified process */
void send_process_info(entry_t *node, pid_t pid, int client_fd);

/* Kill process using more than threshold memory */
void kill_overhead_process(entry_t *, double);

/* Calculate total memory of a process */
unsigned int process_memory(pid_t);

cmd_t *recv_cmd(int); /* receive commands from clients */

bool recv_flag(int, flag_t *); /* receive flags from client */

void handler(int, siginfo_t *, void *); /* signal handler */

void free_cmd(cmd_t *a_request); /* free addresses for 1 request */

static atomic_bool quit = ATOMIC_VAR_INIT(false); /* atomic bool variable for quitting */

sigset_t set; /* set of signals to be blocked (and waiting on) */

void handler(int sig, siginfo_t *siginfo, void *context) {
    if (sig == SIGINT) {
        quit = true;
        printf("%s - received SIGINT\n", get_time());
        printf("%s - Cleaning up and terminating\n", get_time());

        //printf("SIGINT: Parent: %d Child:%d CoC: %d\n", getpid(), pid, pid + 1);
        //        kill(getpid(), SIGKILL);
        sleep(1);
        pthread_cond_broadcast(&got_request);

        /* free memory left if exist */
        request_t *a_request;
        while ((a_request = get_request())) {
            free_cmd(a_request->cmd_arg);
            free(a_request);
        }

        entry_t *a_entry;
        while ((a_entry = get_entry())) {
            free(a_entry);
        }
    }
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0); /* set no buffer for stdout */
    setvbuf(stderr, NULL, _IONBF, 0); /* set no buffer for stderr */
    cmd_t *cmd_arg; /* command group's information */

    /* check for arguments */
    if (argc != 2) {
        fprintf(stderr, "usage: overseer <port>\n");
        exit(EXIT_FAILURE);
    }

    /* occur when interrupt signal is sent (Ctrl + C)
     * Note: sigaction is preferred here since it would not trigger SA_RESTART
     * which will restart the accept function and won't let the program quit */
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = &handler;
    sigaction(SIGINT, &sa, NULL);

    /* block the sigusr1 signal for all threads which will be consumed by sigwaitinfo later
     * to get the pid of the grandchild (and avoid interrupting when signal is sent) */
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    /* start threads */
    pthread_t p_threads[NUM_THREADS]; /* threads */

    /* initialize the mutex and condition variable */
    pthread_mutex_init(&request_mutex, NULL);
    pthread_cond_init(&got_request, NULL);

    pthread_mutex_init(&entry_mutex, NULL);

    /* create the request-handling threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&p_threads[i], NULL, (void *(*)(void *)) handle_requests_loop, NULL);
    }

    //    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    /* setup networking */
    int server_fd, client_fd;
    uint16_t port;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;

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
    printf("%s - totalram: %lu\n", get_time(), mem_avail());

    /* repeat: accept, execute, close connection */
    while (!quit) {
        sin_size = sizeof(struct sockaddr_in);

        /* accept connection */
        if ((client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &sin_size)) == -1) {
            if (errno = EINTR) {
                continue;
            } else {
                perror("accept");
                continue;
            }
        }

        printf("%s - connection received from %s\n", get_time(), inet_ntoa(client_addr.sin_addr));

        /* receive command from client */
        if (!(cmd_arg = recv_cmd(client_fd))) {
            close(client_fd);
            continue;
        }

        if (cmd_arg->type == cmd1) { // add request cmd1 to request pool
            /* add request to the linked list */
            add_request(cmd_arg, &request_mutex, &got_request);
        } else if (cmd_arg->type == cmd2) { // process cmd2 and cmd3 and free afterwards
            process_cmd2(cmd_arg, client_fd);
            free_cmd(cmd_arg);
        } else {
            process_cmd3(cmd_arg);
            free_cmd(cmd_arg);
        }

        /* close connection */
        close(client_fd);
    }
    close(server_fd);

    /* join threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(p_threads[i], NULL);
    }

    /* exit gracefully */
    exit(EXIT_SUCCESS);
}

request_t *add_request(cmd_t *cmd_arg, pthread_mutex_t *p_mutex, pthread_cond_t *p_cond_var) {
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

    return a_request;
}

request_t *get_request() {
    request_t *a_request; /* pointer to a request */

    if (num_request > 0) {
        /* get request from the head of the list */
        a_request = requests;
        requests = a_request->next;

        /* if request is the last request on the list */
        if (requests == NULL) {
            last_request = NULL;
        }

        /* decrement the number of pending requests */
        num_request--;
    } else {
        a_request = NULL;
    }

    /* return the request to the caller */
    return a_request;
}

void *handle_requests_loop(void *data) {
    //TODO
    request_t *a_request; /* pointer to a request */

    /* do forever... */
    while (!quit) {
        /* lock the mutex, to access the requests list exclusively. */
        pthread_mutex_lock(&request_mutex);

        /* wait for a request to arrive. Note the mutex will be
         * unlocked here for other threads to access the requests list.
         * After getting request and acquire mutex, it will automatically
         * locked the mutex (require unlock explicitly) */
        if (num_request <= 0) {
            pthread_cond_wait(&got_request, &request_mutex);
        }

        /* get request */
        a_request = get_request();

        /* unlock lock other threads to get request */
        pthread_mutex_unlock(&request_mutex);

        if (a_request) {
            /* handle request */
            process_cmd1(a_request->cmd_arg);

            /* free request and its resources */
            free_cmd(a_request->cmd_arg);
            free(a_request);
        }
    }
}

void process_cmd1(cmd_t *cmd_arg) {
    int status;
    pid_t pid;

    /* fork and execute file */
    pid = fork();
    if (pid == -1) {
        perror("fork");
    } else if (pid == 0) { /* child */
        /* flag argument value */
        char *outFile = "",
                *logFile = "",
                *exec_timeout = "10",
                *term_timeout = "5";

        /* process flags */
        for (int i = 0; i < cmd_arg->flag_size; i++) {
            switch (cmd_arg->flag_arg[i].type) {
                case o:
                    outFile = cmd_arg->flag_arg[i].value;
                    break;
                case log:
                    logFile = cmd_arg->flag_arg[i].value;
                    break;
                case t:
                    exec_timeout = cmd_arg->flag_arg[i].value;
                    break;
            }
        }

        int arg_size = cmd_arg->file_size + 5;
        char *args[arg_size + 1];
        args[0] = "./exec";
        args[1] = exec_timeout;
        args[2] = term_timeout;
        args[3] = outFile;
        args[4] = logFile;

        for (int i = 5; i < arg_size; i++) {
            args[i] = cmd_arg->file_arg[i - 5];
        }
        args[arg_size] = NULL;

        execv(args[0], args);

        _exit(EXIT_SUCCESS);
    } else { /* parent */
        /* wait for the signal from the grandchild to get the grandchild pid */
        siginfo_t siginfo;
        if (sigwaitinfo(&set, &siginfo) == -1) {
            perror("sigwaitinfo");
            return;
        }

        int child_pid = siginfo.si_value.sival_int;
        unsigned int mem;
        while (!quit) {
            if ((mem = process_memory(child_pid)) > 0) {
                add_entry(child_pid, mem, cmd_arg);
                // print_entry(entry);
                sleep(1);
            }
            int result;
            result = waitpid(pid, &status, WNOHANG);
            if (result > 0) {
                break;
            } else if (result < 0) {
                perror("waitpid");
                break;
            }
        }
    }
}

void process_cmd2(cmd_t *cmd_arg, int client_fd) {
    // print_entry(entry);
    if (cmd_arg->flag_arg[0].value) {
        pid_t mem_pid = (pid_t) atoi(cmd_arg->flag_arg[0].value);
        send_process_info(entry, mem_pid, client_fd);
    } else {
        char mem_time[TIME_BUFFER];
        strcpy(mem_time, get_time());
        int year, month, date, hour, minute, second;
        sscanf(mem_time, "%d-%d-%d %d:%d:%d", &year, &month, &date, &hour, &minute, &second);
        sprintf(mem_time, "%d-%d-%d %d:%d:%d", year, month, date, hour, minute, second - 1);

        struct tm tm_info;
        strptime(mem_time, "%Y-%m-%d %H:%M:%S", &tm_info);
        strftime(mem_time, TIME_BUFFER, "%Y-%m-%d %H:%M:%S", &tm_info);

        send_current_process(entry, mem_time, client_fd);
    }
}

void process_cmd3(cmd_t *cmd_arg) {
    if (cmd_arg->flag_arg[0].value) {
        char *eptr;
        double mem_percent = strtod(cmd_arg->flag_arg[0].value, &eptr);
        kill_overhead_process(entry, mem_percent);
    }
}

unsigned long mem_avail() {
    struct sysinfo info;

    if (sysinfo(&info) < 0)
        return 0;

    return info.freeram;
}

unsigned int process_memory(pid_t pid_temp) {
    char buf[MAX_BUFFER];
    FILE *f;

    // Read the /proc/self/maps
    sprintf(buf, "/proc/%d/maps", pid_temp);
    f = fopen(buf, "rt");
    if (f == NULL) {
        return 0;
    }

    unsigned int from[MAX_ARRAY_SIZE], to[MAX_ARRAY_SIZE], pgoff[MAX_ARRAY_SIZE], major[MAX_ARRAY_SIZE], minor[MAX_ARRAY_SIZE];
    unsigned long ino[MAX_ARRAY_SIZE] = {[0 ... MAX_ARRAY_SIZE - 1] = 1};
    char flags[4];
    int count = 0;

    // Loop through the file line by line
    while (fgets(buf, MAX_BUFFER, f)) {

        // Parse data of each line
        sscanf(buf, "%x-%x %4c %x %x:%x %lu ", from + count, to + count,
               flags, pgoff + count, major + count, minor + count, ino + count);
        //        printf("%s", buf);
        count++;
    }

    // Calculate total memory for inode = 0
    unsigned int total = 0;
    for (int i = 0; i < MAX_ARRAY_SIZE; i++) {
        if (ino[i] == 0) {
            //            printf("Memory: %d Inode: %lu\n", (to[i]-from[i])/1024, ino[i]);
            total = total + (to[i] - from[i]) / 1024;
        }
    }
    //    printf("Total: %d\n", total);
    fclose(f);
    return total;
}

entry_t *add_entry(pid_t pid, unsigned int mem, cmd_t *cmd_arg) {

    entry_t *a_entry; /* pointer to newly added entry */

    /* create a new request */
    a_entry = (entry_t *) malloc(sizeof(entry_t));
    if (!a_entry) {
        fprintf(stderr, "add_entry: out of memory\n");
    }

    a_entry->pid = pid;
    a_entry->mem = mem * 1024; // Convert kilobytes to bytes
    strcpy(a_entry->current_time, get_time());
    a_entry->argc = cmd_arg->file_size;
    a_entry->argv = cmd_arg->file_arg;
    a_entry->next = NULL;

    /* modify the linked list of entries */
    pthread_mutex_lock(&entry_mutex); /* get exclusive access to the list */

    /* add the entry to the end of the list */
    if (!num_entry) { /* the entry list is empty */
        entry = a_entry;
        last_entry = a_entry;
    } else {
        last_entry->next = a_entry;
        last_entry = a_entry;
    }

    /* increase the total of pending entry */
    num_entry++;

    /* unlock the mutex */
    pthread_mutex_unlock(&entry_mutex);

    return a_entry;
}

entry_t *get_entry() {
    entry_t *a_entry; /* pointer to a request */

    if (num_entry > 0) {
        /* get request from the head of the list */
        a_entry = entry;
        entry = a_entry->next;

        /* if request is the last request on the list */
        if (entry == NULL) {
            last_entry = NULL;
        }

        /* decrement the number of pending requests */
        num_entry--;
    } else {
        a_entry = NULL;
    }

    /* return the request to the caller */
    return a_entry;
}

void send_current_process(entry_t *node, char *mem_time, int client_fd) {
    int max_buffer = (num_entry + 1) * MAX_BUFFER;
    char *buff = (char *) malloc(sizeof(char) * max_buffer);
    memset(buff, 0, max_buffer);

    for (int len = 0; node != NULL && len <= max_buffer; node = node->next) {
        if (strcmp(node->current_time, mem_time) == 0) {
            len += sprintf(buff + len, "%d %d ", node->pid, node->mem);
            for (int i = 0; i < node->argc; i++) {
                len += sprintf(buff + len, "%s ", node->argv[i]);
            }
            len += sprintf(buff + len, "\n");
        }
    }

    if (!send_str(client_fd, buff)) {
        fprintf(stderr, "error sending current process entries\n");
    }
//    printf("%s", buff);
    free(buff);
}

void send_process_info(entry_t *node, pid_t pid, int client_fd) {
    int max_buffer = (num_entry + 1) * MAX_BUFFER;
    char *buff = (char *) malloc(sizeof(char) * max_buffer);
    memset(buff, 0, max_buffer);

    for (int len = 0; node != NULL && len <= max_buffer; node = node->next) {
        if (node->pid == pid) {
            len += sprintf(buff + len, "%s- PID:%d - Mem:%d\n", node->current_time, node->pid, node->mem);
        }
    }

    if (!send_str(client_fd, buff)) {
        fprintf(stderr, "error sending %d's info\n", pid);
    }

    free(buff);
}

void kill_overhead_process(entry_t *node, double mem_percent) {
    for (; node != NULL; node = node->next) {
        double process_percent = (double) node->mem / (double) mem_avail() * 100.0;
        if (process_percent > mem_percent) {
            kill(node->pid, SIGKILL);
        }
    }
}

cmd_t *recv_cmd(int client_fd) {
    /* allocate memory for the newly created command */
    cmd_t *cmd_arg = (cmd_t *) malloc(sizeof(cmd_t));

    /* receive type of the command */
    uint32_t type;
    if (recv(client_fd, &type, sizeof(type), 0) != sizeof(type)) {
        fprintf(stderr, "recv got invalid size value\n");
        return NULL;
    }
    cmd_arg->type = ntohl(type);

    /* receive flag size */
    uint32_t flag_size;
    if (recv(client_fd, &flag_size, sizeof(flag_size), 0) != sizeof(flag_size)) {
        fprintf(stderr, "recv got invalid size value\n");
        return NULL;
    }
    cmd_arg->flag_size = ntohl(flag_size);

    /* receive all flags */
    cmd_arg->flag_arg = (flag_t *) malloc(sizeof(flag_t) * 3);
    for (int i = 0; i < cmd_arg->flag_size; i++) {
        if (!recv_flag(client_fd, cmd_arg->flag_arg + i)) {
            fprintf(stderr, "error receiving flag argument\n");
            return NULL;
        };
    }

    /* receive file arguments */
    /* receive file size */
    uint32_t file_size;
    if (recv(client_fd, &file_size, sizeof(file_size), 0) != sizeof(file_size)) {
        fprintf(stderr, "recv got invalid size value\n");
        return NULL;
    }

    /* receive file arguments */
    cmd_arg->file_size = ntohl(file_size);
    cmd_arg->file_arg = (char **) malloc(sizeof(char *) * (cmd_arg->file_size + 1));
    for (int i = 0; i < cmd_arg->file_size; i++) {
        if (!(cmd_arg->file_arg[i] = recv_str(client_fd))) {
            fprintf(stderr, "error receiving file arguments\n");
            return NULL;
        }
    }
    /* add null pointer to the end of the array */
    cmd_arg->file_arg[cmd_arg->file_size] = NULL;

    return cmd_arg;
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
        if (!(flag_arg->value = recv_str(client_fd)))
            return false;
    } else {
        flag_arg->value = NULL;
    }

    return true;
}

void free_cmd(cmd_t *cmd_arg) {
    /* free cmd_args elements */
    /* free file args if exist (mem and memkill doesn't have file specified) */
    if (cmd_arg->file_arg) {
        for (int i = 0; i < cmd_arg->file_size; i++) {
            free(cmd_arg->file_arg[i]);
        }
        free(cmd_arg->file_arg);
    }

    /* free flag args and its value if exist
     * Note that some command only has file without flag
     * Note that some flag doesn't have value (mem) */
    for (int i = 0; i < cmd_arg->flag_size; i++) {
        if (cmd_arg->flag_arg[i].value) {
            free(cmd_arg->flag_arg[i].value);
        }
    }
    free(cmd_arg->flag_arg);

    /* free cmd_arg */
    free(cmd_arg);
}
