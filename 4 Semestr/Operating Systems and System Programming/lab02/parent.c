#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <locale.h>

#define NAME_MAX 255

int size = 0;
int is_child = 0;

//функция сравнения двух массивов символов
int sortCondition(const void *s1, const void *s2) {
        if(strcmp((char*)s1, (char*)s2) > 0){
                return 1;
        }
        else{
                return -1;
        }
}

 //сортировка массива строк
void sort(char** files, int n){
	for (int i = 0; i < n; i++){
		for (int j = i + 1; j < n; j++){
			if(strcmp(files[i], files[j]) > 1){
                char* temp = files[i];
                files[i] = files[j];
                files[j] = temp;
			}
		}
	}
}

int length(char* str) {
    int i = 0;
    while(str[i]) i++;
    return i;
}

char* increment_name(char* name) {
    if (name[length(name) - 1] == '9') {
        name[length(name) - 1] = '0';
        name[length(name) - 2]++; 
    } else {
        name[length(name) - 1]++;
    }
    return name;
}

char* cut(char *str, int index){ //усечение строки
	char *rez = calloc((strlen(str) - index), sizeof(char));
		for(int i = index; i < (int)strlen(str); i++) 
			strncat(rez, &str[i], 1);
    return rez;
}

void first_function(char *name, char *args[], char* env[]){
    int child_status;
    pid_t pid = fork();

    if (pid == -1) {
        printf("Error occured, error code - %d\n", errno);
        exit(errno);
    }

    if (pid == 0) {
        printf("%s process created. Please, wait...\n", name);

        if (is_child) {
            execve(getenv("CHILD_PATH"), args, env);
        } else {
            execve(strcat(getenv("CHILD_PATH"), "/child"), args, env);
        }
    }

    waitpid(-1, &child_status, 0);
    printf("Child process have ended with %d exit status\n", child_status);
}

void second_function(char *name, char *args[], char* env[]){
    int child_status = 0;
    char* path = NULL;

    path = cut(env[size - 1], strlen("CHILD_PATH") + 1); //запись пути к каталогу, где находится дочерняя программа
    pid_t pid = fork();
    
    if (pid == -1) {
        printf("Error occured, error code - %d\n", errno);
        exit(errno);
    }

    if (pid == 0) {
        printf("%s process created. Please, wait...\n", name);
        if (is_child) {
            execve(getenv("CHILD_PATH"), args, env);
        } else {
            execve(strcat(getenv("CHILD_PATH"), "/child"), args, env);
        }
    }
    waitpid(-1, &child_status, 0);
    
    printf("Child process have ended with %d exit status\n", child_status);

    free(path);
}

void third_function(char *name, char *args[], char* env[]){
    int child_status = 0;
    char* path = NULL;

    path = cut(env[size - 1], strlen("CHILD_PATH") + 1); //запись пути к каталогу, где находится дочерняя программа
    pid_t pid = fork();
    
    if (pid == -1) {
        printf("Error occured, error code - %d\n", errno);
        exit(errno);
    }

    if (pid == 0) {
        printf("%s process created. Please, wait...\n", name);
        if (is_child) {
            execve(getenv("CHILD_PATH"), args, env);
        } else {
            execve(strcat(getenv("CHILD_PATH"), "/child"), args, env);
        }
    }
    waitpid(-1, &child_status, 0);
    printf("Child process have ended with %d exit status\n", child_status);

    free(path);
}

int main(int argc, char* argv[], char* env[]) {
    setlocale(LC_COLLATE,"C");

    char* name = calloc(9, sizeof(char));
    strcat(name,"child_00");
    extern char **environ;

    char* args[] = {name, argv[1], (char*)0};
    
    if (argc != 2) {
        printf("There are no 2 params\n");
        return 0;
    }    
    
    while(env[size++]);
    sort(env, size - 1);

    for (int i = 0; i < argc; i++) {
        puts(argv[i]);
    }

    size = 0;
    while (env[size]) {
        printf("%s\n",env[size++]);
    }

    if (strstr(getenv("CHILD_PATH"), "/child")) {
        is_child = 1;
    }

    char* menu = calloc(10, 1);
    int flag = 1;
    do{
        strcpy(menu,"\0");
        fgets(menu, sizeof(menu), stdin);

        if (strstr(menu, "+")) {
            first_function(name, args, env);
            name = increment_name(name);
        } else if (strstr(menu, "*")) {
            second_function(name, args, env);
            name = increment_name(name);
        } else if (strstr(menu, "&")) {
            third_function(name, args, environ);
            name = increment_name(name);
        } else if (strstr(menu, "q")) {
            flag = 0;
        } else {
            puts("Неизвестный символ!");
        }
    } while(flag);

    exit(0);
}