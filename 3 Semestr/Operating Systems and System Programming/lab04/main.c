#define _GNU_SOURCE

#define BUFFER_SIZE 10 // Размер кольцевого буфера
#define BUFFER_ERROR -1
#define BUFFER_EMPTY 0
#define BUFFER_OK 1
#define BUFFER_FULL 2

#define SHM_SIZE 1024 

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"

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
#include <malloc.h>
#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/shm.h>

//из числа в строку
char* itoa(int num) {       
    int i = 0;
    int lenght = 0;
    int temp = num;
    int temp_;
    char* str;
    while(temp != 0) {      
        temp /= 10;
        lenght++;
    }
    str = (char*) malloc((lenght + 1) * sizeof(char));    
    do {
        temp_ = num % 10;
        str[i++] = temp_ + '0';
        num = num / 10;
    }while(num != 0);       
    str[i] = '\0';
    for(i = 0; i < lenght / 2; i++) {
        char temp = str[i];

        str[i] = str[lenght - i - 1];
        str[lenght - i - 1] = temp;       
    }
    return str;
}

//структура сообщения
typedef struct {
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    uint8_t data[256];
} Message;

//структура буфера
struct Buffer {
    Message messages[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    int extracted;
};

//функция добавления производителя
int add_produser(pid_t *producer_process, int key) {
    *producer_process = fork();
    if (*producer_process < 0) {
        fprintf(stderr,"%s\n" ,strerror(errno));
        exit(1);
    } else if (*producer_process == 0) {
        execl("./produser", (const char*)itoa((int)key), NULL);
    }
    return 1;
}

//функция удаления процесса
int remove_process(pid_t *producer_process) {
    if (*producer_process > 0){
        kill(*producer_process, SIGUSR1);
    }
    return 1;
}

void pause_process(pid_t *process, int process_number) {
    for (int i = 0; i < process_number; i++) {
        kill(process[i], SIGUSR2);
    }
}

//функция добавления потреьителя
int add_consumer(pid_t *consumer_process, int key) {
    *consumer_process = fork();
    if (*consumer_process < 0) {
        fprintf(stderr,"%s\n" ,strerror(errno));
        exit(1);
    } else if (*consumer_process == 0) {
        execl("./consumer", (const char*)itoa((int)key), NULL);
    }
    return 1;
}

void print_proceses(int number, pid_t *proceses, const char* color){
    for (int i = 0; i < number; i++) {
        printf("%s%d%s\n", color, proceses[i], RESET);
    }
}

int main() {    
    
    int shmid;
    int process_status;
    struct Buffer* buffer_ptr;
    struct sembuf sops = { 0, 0, SEM_UNDO };

    key_t key = ftok("./", 2);              //создаём ключи для создания 
    key_t key_sem = key + 1;
    key_t key_produser = key + 2;
    key_t key_consumer = key + 3;

    int is_blocked_sem = semget(key_sem, 1, IPC_CREAT | 0666);      //сохдаём семафоры
    int full = semget(key_produser, 1, IPC_CREAT | 0666);
    int empty = semget(key_consumer, 1, IPC_CREAT | 0666);

    int produser_number = 0;
    int consumer_number = 0;
    pid_t *producer_process = NULL;
    pid_t *consumer_process = NULL;

    shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);        //создаём общую память
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    buffer_ptr = shmat(shmid, (void*)0, 0);                 //присоединяем общую память

    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } arg;

    arg.val = 1;            //инициализация одноместного семафора для доступа к буферу
    if (semctl(is_blocked_sem, 0, SETVAL, arg) == -1) {
        perror("semctl");
        exit(1);
    }
    arg.val = 10;           //инициализация семафора для отслеживания свободных мест в буфере
    if (semctl(empty, 0, SETVAL, arg) == -1) {
        perror("semctl");
        exit(1);
    }
    arg.val = 0;            //инициализация семафора для отслеживания сообщения в буфере
    if (semctl(full, 0, SETVAL, arg) == -1) {
        perror("semctl");
        exit(1);
    }

    char* menu = calloc(10, 1);
    while(true) {
        strcpy(menu,"\0");
        fgets(menu, sizeof(menu), stdin);
        if (strstr(menu, "q")) {
            while (produser_number) {
                remove_process(&producer_process[produser_number - 1]);
                produser_number--;
            }
            if(producer_process != NULL){
                free(producer_process);
                producer_process = NULL;
            }
            
            while (consumer_number) {
                remove_process(&consumer_process[consumer_number - 1]);
                consumer_number--;
            }
            if(consumer_process != NULL){
                free(consumer_process);
                consumer_process = NULL;
            }
            sleep(1);
            break;
        } else if (strstr(menu, "r")) {
            produser_number--;
            remove_process(&producer_process[produser_number]);
            producer_process = realloc(producer_process, produser_number * sizeof(pid_t));
        } else if (strstr(menu, "o")) {
            consumer_number--;
            remove_process(&consumer_process[consumer_number]);
            consumer_process = realloc(consumer_process, consumer_number * sizeof(pid_t));  
        } else if (strstr(menu, "p")) {
            produser_number++;
            producer_process = realloc(producer_process, produser_number * sizeof(pid_t));
            add_produser(&producer_process[produser_number - 1], key);
        } else if (strstr(menu, "c")) {
            consumer_number++;
            consumer_process = realloc(consumer_process, consumer_number * sizeof(pid_t));
            add_consumer(&consumer_process[consumer_number - 1], key);
        } else if (strstr(menu, "l")) {
            pause_process(producer_process, produser_number);
            pause_process(consumer_process, consumer_number);
            sleep(1);
            printf("\n%sPRODUSERS :\n", RESET);
            print_proceses(produser_number, producer_process, GREEN);
            puts("CONSUMERS :");
            print_proceses(consumer_number, consumer_process, RED);
            puts("Enter something to continue");
            while(!getchar());
            pause_process(producer_process, produser_number);
            pause_process(consumer_process, consumer_number);
        }
    }
    free(menu);


    if (semctl(is_blocked_sem, 0, IPC_RMID, 0) == -1) {             //удаления семафоров
        perror("Ошибка удаления набора семафора is_blocked_sem");
        exit(EXIT_FAILURE);
    }
    if (semctl(full, 0, IPC_RMID, 0) == -1) {
        perror("Ошибка удаления набора семафора full");
        exit(EXIT_FAILURE);
    }
    if (semctl(empty, 0, IPC_RMID, 0) == -1) {
        perror("Ошибка удаления набора семафора empty");
        exit(EXIT_FAILURE);
    }

    shmdt(buffer_ptr);     // Отсоединяем разделяемую память
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
