#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char **argv) { 
    char *data;
    int bytes = (1024*1024*10);
    //data = (char *) malloc(bytes);
    printf("Hello World. In %s seconds I will terminate with a status code 5\n", argv[1]);
    printf("The list of arg: ");
    for (int i = 0; i < argc; i++) {
    	printf("%s ", argv[i]);
    }
    printf("\n");
    sleep(atoi(argv[1]));
    printf("Finished sleeping for %d\n",argv[1] );
    return 5; 
}