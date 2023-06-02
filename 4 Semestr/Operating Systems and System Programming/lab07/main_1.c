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
#include <fcntl.h>

pthread_mutex_t mutex;
pthread_mutex_t cond_mutex;
pthread_cond_t cond_produser;
pthread_cond_t cond_consumer;

int *produser_flags = NULL;
int *consumer_flags = NULL;
int message_number = 10;
int deleted_message_number = 0;
int my_pause = 0;
int produser_number = 0;
int consumer_number = 0;

//структура сообщения
typedef struct {
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    uint8_t data[256];
} Message;

//структура буфера
struct Buffer {
    Message *messages;
    int head;
    int tail;
    int count;
    int extracted;
};

struct Buffer buffer;
struct Buffer deleted_buffer;

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
int push_buffer(Message message) {
    if ((buffer.tail == message_number - 1) || (buffer.tail > message_number - 1)) {
        buffer.tail = 0;
    } else {
        buffer.tail++;
    }
    buffer.messages[buffer.tail] = message;
    buffer.count = buffer.count + 1;

    return 1;
}

Message null_message;

//функция извлечения сообщения из буфера
int pop_buffer(Message* message) {
    buffer.extracted++;
    if (buffer.head == message_number - 1) {
        if (deleted_message_number != 0) {
            deleted_message_number--;
            *message = deleted_buffer.messages[deleted_message_number];
            return BUFFER_OK;
        } else {
            buffer.head = 0;
        }
    } else {
        buffer.head++;
    }
    *message = buffer.messages[buffer.head];
    buffer.messages[buffer.head] = null_message;

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

void print_proceses(int number, pid_t *proceses, const char* color){
    for (int i = 0; i < number; i++) {
        printf("%s%d%s\n", color, proceses[i], RESET);
    }
}

//функция вывода сообщения
void print_message(Message message, const char* color, int count) {
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
}

void *produser(void *arg) {
    while(true) {
        while(my_pause == 0) {
            if (*(int *)arg) {
                puts("END P");
                pthread_mutex_unlock(&cond_mutex);
                pthread_exit(NULL);
            }
            sleep(3);
            if (*(int *)arg) {
                puts("END P");
                pthread_mutex_unlock(&cond_mutex);
                pthread_exit(NULL);
            }

            pthread_mutex_lock(&cond_mutex);

            while ((buffer.count - buffer.extracted) >= message_number) {
                puts("============================== Буффер Заполнен ==============================");
                pthread_cond_wait(&cond_produser, &cond_mutex);

                if (*(int *)arg) {
                    puts("END P");
                    pthread_mutex_unlock(&cond_mutex);
                    pthread_exit(NULL);
                }
            }
            
            Message message = generate_message();
            push_buffer(message);

            if ((buffer.count - buffer.extracted) > 0) {
                pthread_cond_signal(&cond_consumer);
            }

            pthread_mutex_unlock(&cond_mutex);



            pthread_mutex_lock(&mutex);
            puts("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>PRODUSER>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
            print_message(message, GREEN, buffer.count);
            puts(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>PRODUSER>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            pthread_mutex_unlock(&mutex);
        }
    }
}

void *consumer(void *arg) {
    while(true) {
        while(my_pause == 0) {
            if (*(int *)arg) {
                puts("END C");
                pthread_mutex_unlock(&cond_mutex);
                pthread_exit(NULL);
            }
            sleep(3);
            if (*(int *)arg) {
                puts("END C");
                pthread_mutex_unlock(&cond_mutex);
                pthread_exit(NULL);
            }

            pthread_mutex_lock(&cond_mutex);

            while ((buffer.count - buffer.extracted) <= 0) {
                puts("============================== Буффер Пуст ==============================");
                pthread_cond_wait(&cond_consumer, &cond_mutex);

                if (*(int *)arg) {
                    puts("END C");
                    pthread_mutex_unlock(&cond_mutex);
                    pthread_exit(NULL);
                }
            }
            
            Message message;
            pop_buffer(&message); 

            if ((buffer.count - buffer.extracted) < message_number) {
                pthread_cond_signal(&cond_produser);
            }

            pthread_mutex_unlock(&cond_mutex);

            pthread_mutex_lock(&mutex);
            puts("\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<CONSUMER<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
            message = check(message);
            print_message(message, RED, buffer.extracted);
            puts("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<CONSUMER<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
            pthread_mutex_unlock(&mutex);
        }
    }
}

void print_threads(int threads_number, pthread_t *threads, const char *color) {
    printf("%s", color);
    for (int i = 0; i < threads_number; i++) {
        printf("%ld\n", threads[i]);
    }
    printf("%s", RESET);
}

void change_flags(int flags) {
    if (flags == 0) {
        for (int i = 0; i < consumer_number; i++) {
            consumer_flags[i] = 1;
        }
    } else {
        for (int i = 0; i < produser_number; i++) {
            produser_flags[i] = 1;
        }
    }
}

int main() {    
    srand((unsigned int)getpid());

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&cond_mutex, NULL);
    pthread_cond_init(&cond_produser, NULL);
    pthread_cond_init(&cond_consumer, NULL);

    pthread_t *produser_thread = NULL;
    pthread_t *consumer_thread = NULL;

    buffer.messages = calloc(message_number, sizeof(Message));
    buffer.head = -1;
    buffer.tail = -1;

    printf("Buffer size = %d\n", message_number);
    char* menu = calloc(10, 1);
    while(true) {
        strcpy(menu,"\0");
        fgets(menu, sizeof(menu), stdin);
        if (strstr(menu, "q")) {

            change_flags(1);
            pthread_cond_broadcast(&cond_produser);
            
            change_flags(0);
            pthread_cond_broadcast(&cond_consumer);

            while (produser_number > 0) {
                produser_number--;
                pthread_join(produser_thread[produser_number], NULL);
            } if(produser_thread != NULL){
                free(produser_thread);
                produser_thread = NULL;
            }

            while (consumer_number > 0) {
                consumer_number--;
                pthread_join(consumer_thread[consumer_number], NULL);
            } if(consumer_thread != NULL){
                free(consumer_thread);
                consumer_thread = NULL;
            }
            break;

        } else if (strstr(menu, "p")) {
            produser_number++;
            produser_thread = realloc(produser_thread, produser_number * sizeof(pthread_t));
            produser_flags = realloc(produser_flags, produser_number * sizeof(int));

            produser_flags[produser_number - 1] = 0;
            (void) pthread_create(&produser_thread[produser_number - 1], NULL, produser, &produser_flags[produser_number - 1]);

        } else if (strstr(menu, "c")) {
            consumer_number++;
            consumer_thread = realloc(consumer_thread, consumer_number * sizeof(pthread_t));
            consumer_flags = realloc(consumer_flags, consumer_number * sizeof(int));

            consumer_flags[consumer_number - 1] = 0;
            (void) pthread_create(&consumer_thread[consumer_number - 1], NULL, consumer, &consumer_flags[consumer_number - 1]);

        } else if (strstr(menu, "r")) {
            if (produser_number == 0) {
                puts("produser_number = 0");
            } else {
                produser_number--;
                produser_flags[produser_number] = 1;
                pthread_cond_signal(&cond_produser);
                pthread_join(produser_thread[produser_number], NULL);

                produser_flags = realloc(produser_flags, produser_number * sizeof(int));
                produser_thread = realloc(produser_thread, produser_number * sizeof(pthread_t));
            }
        } else if (strstr(menu, "o")) {
            if (consumer_number == 0) {
                puts("consumer_number = 0");
            } else {
                consumer_number--;
                consumer_flags[consumer_number] = 1;
                pthread_cond_signal(&cond_consumer);
                pthread_join(consumer_thread[consumer_number], NULL);

                consumer_flags = realloc(consumer_flags, consumer_number * sizeof(int));
                consumer_thread = realloc(consumer_thread, consumer_number * sizeof(pthread_t));  
            }
        } else if (strstr(menu, "l")) {
            my_pause = 1;
            sleep(3);
            printf("\n%sPRODUSERS :\n", RESET);
            print_threads(produser_number, produser_thread, GREEN);
            puts("CONSUMERS :");
            print_threads(consumer_number, consumer_thread, RED);
            puts("Enter something to continue");
            while(!getchar());
            my_pause = 0;
        } 
    }
    free(menu);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_produser);
    pthread_cond_destroy(&cond_consumer);
    return 0;
}
