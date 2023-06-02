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

int end_flag = 0;
int my_pause = 0;

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

//функция генерации сообщения
Message generate_message() {
    Message message;
    message.hash = 0;
    int n = 0;
    int i = 0;

    message.type = rand() % 2;

    message.size = rand() % 256 + 1; // получение случайного числа от 1 до 256
    if (message.size == 0) {
        n = 256;
    } else {
        n = message.size;
    }

    for (; i < n; i++) {
        message.data[i] = rand() % 256;
        message.hash += message.data[i];
    }
    message.hash += message.size;
    //message.data[i] = '\0';
    if (message.type == 1) {
        message.hash *= 10;
    }
    return message;
}

//функция добавления сообщения в буфер
int push_buffer(Message message, struct Buffer* buffer) {
    buffer->messages[buffer->tail] = message;
    if (buffer->tail == 9) {
        buffer->tail = 0;
    } else {
        buffer->tail++;
    }
    buffer->count = buffer->count + 1;

    return 1;
}

//функция вывода сообщения
void print_message(Message message, const char* color, int count) {
    puts("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>PRODUSER>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    printf("%s", color);
    printf("count - %d\n", count);
    printf("type - %d\n", message.type);
    printf("hash - %d\n", message.hash);
    printf("size - %d\n", message.size);
    //puts("data :");
    //for (int i = 0; i < message.size; i++) {
    //    printf("%d\t", message.data[i]);
    //}
    printf("%s\n", RESET);
    puts(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>PRODUSER>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
}

//обработчик сигналов
void usr_handler(int signal){      
    if (signal == SIGUSR1) {
        end_flag = true;
    }
    if (signal == SIGUSR2) {
        my_pause = (my_pause + 1) % 2;
    }
} 

int main(int argc, char *argv[]) 
{
    srand((unsigned int)getpid());
    signal(SIGUSR1, usr_handler);
    signal(SIGUSR2, usr_handler);

    int shmid;
    struct sembuf sops = { 0, 0, 0 };
    struct Buffer* buffer_ptr;
    Message message;

    key_t key = atoi(argv[0]);
    key_t key_sem = key + 1;
    key_t key_produser = key + 2;
    key_t key_consumer = key + 3;

    int is_blocked_sem = semget(key_sem, 1, IPC_CREAT | 0666);
    int full = semget(key_produser, 1, IPC_CREAT | 0666);
    int empty = semget(key_consumer, 1, IPC_CREAT | 0666);

    shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    buffer_ptr = shmat(shmid, (void*)0, 0);

    while (true) {
    while(!my_pause) {
        sleep(3);               //проверка флага окончания
        if (end_flag) {
            shmdt(buffer_ptr);   
            exit(1);
        }

        message = generate_message();       //генерация сообщения

        sops.sem_num = 0;       //уменьшение семафора пустоты буфера
        sops.sem_op = -1;
        sops.sem_flg = 0;
        semop(empty, &sops, 1);

        if (end_flag) {         //проверка флага окончания
            shmdt(buffer_ptr);   
            exit(1);
        }

        sops.sem_num = 0;       //ограничение доступа к буферу для других процессов
        sops.sem_op = -1;
        sops.sem_flg = 0;
        semop(is_blocked_sem, &sops, 1);

        
        push_buffer(message, buffer_ptr);       //заполнение буфера
        print_message(message, GREEN, buffer_ptr->count);
          
        sops.sem_num = 0;       //возобновление доступа к буферу
        sops.sem_op = 1;
        sops.sem_flg = 0;
        semop(is_blocked_sem, &sops, 1);    

        sops.sem_num = 0;       //увеличение семафора заполненности буфера
        sops.sem_op = 1;
        sops.sem_flg = 0;
        semop(full, &sops, 1);
 
        if (buffer_ptr->count - buffer_ptr->extracted == 10) {
            puts("============================== Буффер Заполнен ==============================");
        }

        if (end_flag) {             //проверка флага окончания
            shmdt(buffer_ptr);   
            exit(1);
        }
    }
    }

    exit(1);
}