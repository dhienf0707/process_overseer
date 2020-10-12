#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <memory.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <errno.h>
#include <pthread.h>

//
// Created by duchi on 9/19/2020.
//





void *exec_hello(void *data) {
    char *time = (char *) data;
    char *args[] = {"./controller/controller", "localhost", "3000", "hello", time, NULL};
    execv(args[0], args);
}

//int main(int argc, char **argv) {
//    pid_t pid;
//    char *args[] = {"./controller/controller", "localhost", "3000", "hello", "0", NULL};
//
//    pid = fork();
//    if (pid == 0) goto fork2;
//    args[4] = "1";
//
//    fork2: { pid = fork(); };
//    if (pid == 0) goto fork3;
//    args[4] = "2";
//
//    fork3: { pid = fork(); };
//
//
//    printf("Send '%s %s' to overseer", args[3], args[4]);
//    execv(args[0], args);
//
//    exit(EXIT_SUCCESS);
//}

/* Includes */
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <sys/wait.h>   /* Wait for Process Termination */
#include <stdlib.h>     /* General Utilities */

//int main() {
//    pid_t childpid; /* variable to store the child's pid */
//    int retval;     /* child process: user-provided return code */
//    int status;     /* parent process: child's exit status */
//
//    /* only 1 int variable is needed because each process would have its
//       own instance of the variable
//       here, 2 int variables are used for clarity */
//
//    /* now create new process */
//    childpid = fork();
//
//    if (childpid >= 0) /* fork succeeded */
//    {
//        if (childpid == 0) /* fork() returns 0 to the child process */
//        {
//            printf("CHILD: I am the child process!\n");
//            printf("CHILD: Here's my PID: %d\n", getpid());
//            printf("CHILD: My parent's PID is: %d\n", getppid());
//            printf("CHILD: The value of my copy of childpid is: %d\n", childpid);
//            printf("CHILD: Sleeping for 1 second...\n");
//            sleep(1); /* sleep for 1 second */
//            printf("CHILD: Enter an exit value (0 to 255): ");
//            scanf(" %d", &retval);
//            printf("CHILD: Goodbye!\n");
//            exit(retval); /* child exits with user-provided return code */
//        } else /* fork() returns new pid to the parent process */
//        {
//            printf("PARENT: I am the parent process!\n");
//            printf("PARENT: Here's my PID: %d\n", getpid());
//            printf("PARENT: The value of my copy of childpid is %d\n", childpid);
//            printf("PARENT: I will now wait for my child to exit.\n");
//            wait(&status); /* wait for child to exit, and store its status */
//            printf("PARENT: Child's exit code is: %d\n", WEXITSTATUS(status));
//            printf("PARENT: Goodbye!\n");
//            exit(0);  /* parent exits */
//        }
//    } else /* fork returns -1 on failure */
//    {
//        perror("fork"); /* display error message */
//        exit(0);
//    }
//}

#define MAX_ARRAY_SIZE 100

int main() {

    char buf[512];
    FILE *f;
    sprintf(buf, "/proc/%d/maps", 12614);
    f = fopen(buf, "rt");
    unsigned int from[MAX_ARRAY_SIZE], to[MAX_ARRAY_SIZE], pgoff[MAX_ARRAY_SIZE], major[MAX_ARRAY_SIZE], minor[MAX_ARRAY_SIZE];
    unsigned long ino[MAX_ARRAY_SIZE] = {[0 ... MAX_ARRAY_SIZE-1] = 1};
    char flags[MAX_ARRAY_SIZE][4];
    int count = 0;
    while (fgets(buf, 512, f)) {


        sscanf(buf, "%x-%x %4c %x %x:%x %lu ", from + count, to + count,
               flags + count, pgoff + count, major + count, minor + 1, ino + count);
        printf("%s", buf);
        count++;
    }
    unsigned int total = 0;
    for (int i = 0; i < MAX_ARRAY_SIZE; i++){
        if (ino[i] == 0) {
           printf("Memory: %d Inode: %lu\n", (to[i]-from[i])/1024, ino[i]);
            total = total + (to[i]-from[i])/1024;
        }
    }
//    printf("Total: %d\n", total);

    return 1;



}