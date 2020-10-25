#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <memory.h>

#define MB 1e6

int main(int argc, char **argv) {
//    signal(SIGTERM, SIG_IGN);

//    printf("%s\n", argv[2]);
    int megs = argc >= 3 ? atoi(argv[2]) : 1;
    char *data = (char *) malloc(megs * MB);
    memset(data, 1, MB);

    int sleep_time = argc >= 2 ? atoi(argv[1]) : 0;
    printf("Hello World. In %d seconds I will terminate with a status code 5\n", sleep_time);
    sleep(1);
    printf("The list of arg: ");
    for (int i = 0; i < argc; i++) {
    	printf("%s ", argv[i]);
    }
    printf("\n");
    sleep(sleep_time);
    printf("Finished sleeping for %d seconds\n", sleep_time);
    return 5;
}