#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[], char* env[]) {
    fprintf(stdout, "%s process begins...\n", argv[0]);
    
    pid_t pid = getpid();
    pid_t ppid = getppid();
    FILE *File = fopen(argv[1],"r");
    char variable[256];

    printf("My pid = %d, my ppid = %d\n", (int)pid, (int)ppid);
    if (File == NULL) {
        printf("File %s not open\n", argv[1]);
        exit(-1);
    }

    while (1) {
        fscanf(File,"%s", variable);
        if (feof(File)) {
            break;
        }
        printf("%s=%s\n", variable, getenv(variable));
    }
    
    exit(0);
}