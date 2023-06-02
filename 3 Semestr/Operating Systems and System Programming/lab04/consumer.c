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

Message null_message;

//функция извлечения сообщения из буфера
int pop_buffer(Message* message, struct Buffer *buffer) {
    buffer->extracted++;
    *message = buffer->messages[buffer->head];
    buffer->messages[buffer->head] = null_message;
        if (buffer->head == 9) {
        buffer->head = 0;
    } else {
        buffer->head++;
    }

    return BUFFER_OK;
}

//функция проверки сообщения
Message check(Message message) {
    uint16_t check_hash = 0;
    int n = 0;
    if (message.size == 0) {
        n = 256;
    } else {
        n = message.size;
    }
    for (int i = 0; i < n; i++) {
        check_hash += message.data[i];
    } 
    check_hash += message.size;
    if (message.type == 1) {
        check_hash *= 10;
    }
    if (message.hash != check_hash) {
        //message.size = 0;
        printf("ERROR(check_hash = %d) \n", check_hash);
    }
    return message;
}

//функция вывода сообщения
void print_message(Message message, const char* color, int count) {
    puts("\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<CONSUMER<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
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
    puts("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<CONSUMER<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
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
    srand(time(NULL));
    signal(SIGUSR1, usr_handler);
    signal(SIGUSR2, usr_handler);

    int shmid;
    struct Buffer* buffer_ptr;
    struct sembuf sops = { 0, 0, 0 };
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

        sops.sem_num = 0;       //уменьшение семафора заполненности буфера
        sops.sem_op = -1;
        sops.sem_flg = 0;
        semop(full, &sops, 1);

        if (end_flag) {         //проверка флага окончания
            shmdt(buffer_ptr);     
            exit(1);
        }

        sops.sem_num = 0;       //ограничение доступа к буферу для других процессов
        sops.sem_op = -1;
        sops.sem_flg = 0;
        semop(is_blocked_sem, &sops, 1);
        
        
        pop_buffer(&message, buffer_ptr);       //извлечени сообщения
        message = check(message); 
        print_message(message, RED, buffer_ptr->extracted);

        sops.sem_num = 0;       //возобновление доступа к буферу
        sops.sem_op = 1;
        sops.sem_flg = 0;
        semop(is_blocked_sem, &sops, 1);

        sops.sem_num = 0;       //увеличение семафора пустоты буфера
        sops.sem_op = 1;
        sops.sem_flg = 0;
        semop(empty, &sops, 1);

        if (buffer_ptr->count - buffer_ptr->extracted == 0) {
            puts("============================== Буффер Пуст ==============================");
        }

        if (end_flag) {         //проверка флага окончания
            shmdt(buffer_ptr);     
            exit(1);
        }
    }
    }

    exit(1);
}