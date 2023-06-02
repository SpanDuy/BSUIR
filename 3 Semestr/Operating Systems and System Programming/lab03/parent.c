#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <stdbool.h>
#include <unistd.h>

void first_func(pid_t temp_pid);
void second_func();
void l_func();
void s_func();
void k_func();
void g_func();

struct proces_struct{
    bool is_stoped;    //статус процесса
    pid_t pid;          //ID процесса
    char name[50];      //название
};

struct proces_struct* child_processes = NULL;     //массив информации о процессах
int child_pids_number = 0;                  //число процессов

//число из строки
int int_from_str(char* str){         
    int i = 0;
    int length = 0;
    char* num = NULL;

    while (str[i] != '\0') {
        if (str[i] > '0' && str[i] < '9'){      //цифра
            length++;
            num=realloc(num, length * sizeof(char));
            num[length - 1] = str[i];           //добавляем число в троку числа
        }
        i++;
    }
    if (length) {
        return atoi(num);       //переводим число в int
    }
    return 0;
}

//из числа в строку
char* itoa(int num) {       
    int i = 0;
    int lenght = 0;
    int temp = num;
    int temp_;
    char* str;

    while(temp != 0) {      // количество разрядов
        temp /= 10;
        lenght++;
    }

    str = (char*) malloc((lenght + 1) * sizeof(char));     //выделение памяти

    do {
        temp_ = num % 10;
        str[i++] = temp_ + '0';
        num = num / 10;
    }while(num != 0);       //перевод в строку

    str[i] = '\0';

    for(i = 0; i < lenght / 2; i++) {
        char temp = str[i];

        str[i] = str[lenght - i - 1];
        str[lenght - i - 1] = temp;        //реверс строки
    }

    return str;
}

//обработчик сигналов
void signal_handler(int signal, siginfo_t *info, void *ucontext){      
    if (signal == SIGUSR1) {
        char accses[3];

        //sleep(3);
        printf("%d - press '1' to print information :", (*info).si_value.sival_int);
        fgets(accses, sizeof(accses), stdin);

        if (strcmp(accses, "1\n") == 0) {
            kill((*info).si_value.sival_int, SIGUSR2);
        } else {
            kill((*info).si_value.sival_int, SIGUSR1);
        }
    } else if (signal == SIGALRM) {
        for (int i = 0; i < child_pids_number; i++) {
            kill(child_processes[i].pid, SIGUSR2);
        }
    }
}

void set_handler(){
    struct sigaction action;
    sigset_t set;

    memset(&action, 0, sizeof(action));   	//заполнение структуры
    action.sa_sigaction = signal_handler;      //установка обработчика
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGALRM);
    action.sa_mask = set;      	    //установка запретов на сигналы
    sigaction(SIGUSR1, &action, 0);
    sigaction(SIGALRM, &action, 0);        //установка сигналов
}

void concatenation(char *str1, char *str2, char *result) {
    strcpy(result, str1);   // копируем первую строку в результирующую
    strcat(result, str2);   // добавляем вторую строку к результирующей
}

void create_child_process(pid_t pid){       //создание дочернего процесса
    concatenation("C_", itoa(child_pids_number - 1), child_processes[child_pids_number - 1].name);
    child_processes[child_pids_number-1].pid = pid;
    child_processes[child_pids_number-1].is_stoped = 0;
}

void delete_process(struct proces_struct* child_processes, int num_process){      //удалить процесс
    printf("killing procces %d...\n", child_processes[num_process].pid);
    kill(child_processes[num_process].pid, SIGTERM);
}

int clean_all_process(struct proces_struct* child_processes, int count_process){      //очистить все процессы
    while (count_process) {
        delete_process(child_processes, count_process-1);
        count_process--;
    }
    if(child_processes != NULL){
        free(child_processes);
        child_processes = NULL;
    }
    return count_process;
}

void first_func(pid_t temp_pid) {
    temp_pid = fork();       
    child_pids_number++;
    child_processes = realloc(child_processes, child_pids_number * sizeof(struct proces_struct));     
    create_child_process(temp_pid);
    if (temp_pid < 0) {
        fprintf(stderr,"%s\n" ,strerror(errno));
        exit(1);
    } else if (temp_pid == 0) {
        execl("./child", "child", NULL);
    } else {
        printf("PID %d(%s) - created\n", temp_pid, child_processes[child_pids_number-1].name);
        printf("count of children proceses - %d\n", child_pids_number);
    }
}

void second_func() {
    delete_process(child_processes, child_pids_number - 1);        
    child_pids_number--;
    child_processes = realloc(child_processes, child_pids_number * sizeof(struct proces_struct));     
    printf("count of children - %d\n", child_pids_number);
}

void l_func() {
    printf("parent - %d\n", (int)getpid());
    printf("child :\n");

    for (int i = 0; i < child_pids_number; i++) {
        printf("%s - \t%d\t", child_processes[i].name,child_processes[i].pid);        
        if (child_processes[i].is_stoped) {
            printf("stoped\n");
        }
        else
            printf("running\n");
    }
}

void k_func() {
    if (child_pids_number) {
        child_pids_number = clean_all_process(child_processes, child_pids_number);
    }
}

void s_func() {
    for (int i = 0; i < child_pids_number; i++) {
        kill(child_processes[i].pid, SIGUSR1);      
        child_processes[i].is_stoped = true;
    }
}

void g_func() {
    for (int i = 0; i < child_pids_number; i++) {
        kill(child_processes[i].pid, SIGUSR2);      
        child_processes[i].is_stoped = false;
    }
}

int main(){
    set_handler();      //обработчик сигналов
    char option[20];    //опция
    pid_t temp_pid = -1;

    while (true) {
        strcpy(option,"\0");
        fgets(option, sizeof(option), stdin);       //ввод опции

        if (strstr(option, "<") && strstr(option, ">")) {        //Х<num> опции 
            int num_pid = int_from_str(option);

            if (num_pid >= child_pids_number) {
                printf("error - %d\n", child_pids_number);
                continue;
            } else if (option[0] == 's') {      //опция s<num>
                kill(child_processes[num_pid].pid, SIGUSR1);
                child_processes[num_pid].is_stoped = true;
                printf("%s is stoped\n", child_processes[num_pid].name);
            } else if (option[0] == 'g') {      //опция g<num>
                kill(child_processes[num_pid].pid, SIGUSR2);
                child_processes[num_pid].is_stoped = false;
                printf("%s is running\n", child_processes[num_pid].name);
            } else if (option[0] == 'p') {      //опция p<num>
                for (int i = 0; i < child_pids_number; i++) {
                    kill(child_processes[i].pid, SIGUSR1);
                    child_processes[i].is_stoped = true;
                }
                sleep(2);
                kill(child_processes[num_pid].pid, SIGUSR2);
                child_processes[num_pid].is_stoped = false;
                printf("%s is running\n", child_processes[num_pid].name);
            }
        }
        else 
            switch (option[0])
            {
            case '+':
                first_func(temp_pid);
                break;
            case '-':
                second_func();
                break;
            case 'q':
                if(child_pids_number) {
                    child_pids_number = clean_all_process(child_processes, child_pids_number);
                    exit(1);
                } else {
                    exit(1);
                }
                break;
            case 'l':
                l_func();
                break;
            case 'k':
                k_func();
                break;
            case 's':
                s_func();
                break;
            case 'g':
                g_func();
                break;
            default:
                break;
            }
    }
    return 0;
}