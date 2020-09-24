#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <memory.h>
#include <helpers.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <errno.h>

//
// Created by duchi on 9/19/2020.
//

void timer_handler(int);
void child_handler(int);

char current_time[TIME_BUFFER];

void timer_handler(int sig) {

}

void child_handler(int sig) {

}

int main(int argc, char **argv) {
    pid_t pid = fork();
    int status;
    int term_time = 4;
    int term_timeout = 5;
    int result;

    if (pid == 0) { /* child */
        signal(SIGTERM, SIG_IGN);
        char *args[] = {"./overseer/hello", "10", NULL};
        execv(args[0], args);
        exit(errno);
    } else {
        signal(SIGALRM, timer_handler);
        signal(SIGCHLD, child_handler);

        /* unpause on either child termination or sigterm timeout*/
        alarm(term_time);
        pause();

        result = waitpid(pid, &status, WNOHANG);
        if (result == 0) { /* child is still running */
            printf("%s - sent SIGTERM to %d\n", get_time(current_time), pid);
            kill(pid, SIGTERM);

            /* unpause on either child termination or sigkill timeout */
            alarm(term_timeout);
            pause();

            result = waitpid(pid, &status, WNOHANG);
            if (result == 0) { /* child is still running  */
                printf("%s - sent SIGKILL to %d\n", get_time(current_time), pid);
                kill(pid, SIGKILL);
            } else if (result == -1) {
                perror("waitpid");
            }
        } else if (result == -1) {
            perror("waitpid");
        }

        printf("exit code: %d\n", WIFEXITED(status));

        printf("status code: %d\n", WEXITSTATUS(status));

        /* By here child should have been finished. Check the status and print out result based on status */
        if (WIFEXITED(status)) { /* if exited normally */
            printf("%s - %d has terminated with status code %d\n", get_time(current_time), pid, WEXITSTATUS(status));
        }
    }
}
