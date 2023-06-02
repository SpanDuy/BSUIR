#define _GNU_SOURCE

#define FILE_NAME "file"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"
#define BUFFER_SIZE 64
#define BLOCK_SIZE 8
#define BLOCKS 8
#define THREADS 2
#define MAX 60081.21597
#define MIN 15020.0

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
#include <sys/mman.h>   
#include <fcntl.h>

typedef struct index_s {
    double time_mark;   // временная метка (модифицированная юлианская дата)
    uint64_t recno;     // первичный индекс в таблице БД
} index_record;

typedef struct index_hdr_s {
    uint64_t recsords;      // количество записей
    index_record *idx;   // массив записей в количестве records
} data_type;

void print();

int PageSize;
int memsize;
int blocks;
int threads;
int block_size;
char *filename;

pthread_barrier_t barrier;
pthread_mutex_t mutex;
pthread_mutex_t number_mutex;
index_record *buffer;
uint64_t count = 0;
int start = 0;
void *address;
int *block_status;

int is_sub_2(int number) {
    int sub = 1;
    while (number >= sub) {
        if (number == sub) {
            return 1;
        } else {
            sub *= 2;
        }
    }
    return 0;
}

int init() {
    if (memsize % PageSize != 0) {
        return -1;
    } if (!is_sub_2(blocks) || blocks > 256) {
        return -2;
    } if (threads > 32 || threads < 4 || blocks < threads) {
        return -4;
    } if (strcmp(filename, "file")) {
        return -8;
    }
    return 1;
}

int compare(const void* a, const void* b) {
    index_record num1 = *((index_record*)a);
    index_record num2 = *((index_record*)b);
    if (num1.time_mark < num2.time_mark)
        return -1;
    else if (num1.time_mark > num2.time_mark)
        return 1;
    else
        return 0;
}

void print() {
    for (int i = 0; i < (int)count; i++) {
        printf("%ld)\t%lf\t", buffer[i].recno, buffer[i].time_mark);
        if (!((i + 1) % (BLOCK_SIZE / 2))) {
            puts("");
        }
        if (!((i + 1) % block_size)) {
        }
    }
    puts("\n");
}

void print_it(index_record *it) {
    for (int i = 0; i < (int)count; i++) {
        printf("%.2ld) %lf\t", it[i].recno, it[i].time_mark);
        if (!((i + 1) % BLOCK_SIZE)) {
            puts("");
        }
        if (!((i + 1) % block_size)) {
        }
    }
    puts("\n");
}

int next_block() {
    int number = -1;
    pthread_mutex_lock(&mutex);

    for (int i = 0; i < blocks; i++) {
        if (!block_status[i]) {
            block_status[i] = 1;
            number = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    return number;
}

int merger_blocks(int iteration) {
    int number = -1;

    pthread_mutex_lock(&mutex);

    for (int i = 0; i < blocks / iteration; i++) {
        if (block_status[i]) {
            block_status[i] = 0;
            number = i;
            break;
        }
    }    

    pthread_mutex_unlock(&mutex);

    return number;
}

void *sort(void *arg) {
    int argument = *(int*)arg;
    int number = argument;
    int iteration = 1;

    pthread_barrier_wait(&barrier);

    while (number >= 0) {
        index_record* block = calloc(block_size, sizeof(index_record));
        for (int i = 0; i < block_size; i++) {
            block[i] = buffer[i + block_size * number];
        }

        qsort(block, block_size, sizeof(index_record), compare);

        for (int i = 0; i < block_size; i++) {
            buffer[i + block_size * number] = block[i];
        }

        number = next_block();
        if (number == -1) {
            break;
            free(block);
        }
        free(block);
    }

    int new_block_size = block_size;
    pthread_barrier_wait(&barrier);

    if (argument == 0) {
        puts("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< После Сортировки Блоков >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        for (int i = 0; i < (int)count; i++) {
            printf("%ld)\t%lf\t", buffer[i].recno, buffer[i].time_mark);
            if (!((i + 1) % (BLOCK_SIZE / 2))) {
                puts("");
            }
            if (!((i + 1) % block_size)) {
                puts("");
            }
        }
        puts("\n");

        //print();
    }

    while (iteration != (int)blocks) {
        number = argument;
        iteration *= 2;
        new_block_size = new_block_size * 2;

        pthread_barrier_wait(&barrier);

        while (number >= 0) {
            number = merger_blocks(iteration);
            if (number == -1) {
                break;
            }

            index_record* block = calloc(new_block_size, sizeof(index_record));
            for (int i = 0; i < new_block_size; i++) {
                block[i] = buffer[i + new_block_size * number];
            }

            int i = 0, j = new_block_size / 2, k = 0;
            for (; (i < new_block_size / 2) && (j < new_block_size);) {
                if (block[i].time_mark > block[j].time_mark) {
                    buffer[k + new_block_size * number] = block[j];
                    j++;
                } else {
                    buffer[k + new_block_size * number] = block[i];
                    i++;
                }
                k++;
            }
            while (j < new_block_size) {
                buffer[k + new_block_size * number] = block[j++];
                k++;
            }
            while (i < new_block_size / 2) {
                buffer[k + new_block_size * number] = block[i++];
                k++;
            }

            free(block);
        }

        pthread_barrier_wait(&barrier);

        for (int i = 0; i < blocks; i++) {block_status[i] = 1;}
    }
    
    pthread_barrier_wait(&barrier);

    if (argument == 0) {
        puts("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< После Слияния Блоков >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        //print();
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {    
    if (argc != 5) {
        puts("Вы ввели не все параметры");
        return 1;
    }
    PageSize = getpagesize();
    memsize = atoi(argv[1]);
    blocks = atoi(argv[2]);
    threads = atoi(argv[3]);
    filename = calloc(1, strlen(argv[4]));
    strcpy(filename, argv[4]);

    int status = init();
    if (status == -1) {
        puts("memsize");
        puts("Размер должен быть кратен 4096");
        free(filename);
        return 0;
    } if (status == -2) {
        puts("blocks");
        puts("Количество Блоков не Должно Быть Больше 256");
        free(filename);
        return 0;
    } if (status == -4) {
        puts("threads");
        puts("Количество Потоков Должно Быть в Пределе от 4 до 32 и не Больше, чем Количество Блоков");
        free(filename);
        return 0;
    } if (status == -8) {
        puts("filename");
        puts("Название файла должно быть \"file\"");
        free(filename);
        return 0;
    }
    srand((unsigned int)getpid());
    
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&number_mutex, NULL);
    pthread_barrier_init(&barrier, NULL, threads);

    int file;
    file = open(filename, O_RDWR);
    if ( file < 0 ) {
        perror("open() error");
    }
    block_status = (int *)calloc(blocks, sizeof(int));

    uint64_t count_;
    int end = 0;
    read(file, &count_, 8);
    //memcpy(&count_, address, sizeof(uint64_t));

    while (count <= count_) {
        count = memsize / 16;
        address = mmap(NULL, memsize + 1, PROT_READ | PROT_WRITE, MAP_SHARED, file, memsize * end );    

        buffer = calloc(1, memsize);

        memcpy(buffer, (char*)(address) + 16, memsize);

        block_size = count / blocks;

        for (int i = 0; i < threads; i++) {
            block_status[i] = 1;
        }
        for (int i = threads; i < blocks; i++) {
            block_status[i] = 0;
        }

        pthread_t *thread = calloc(threads, sizeof(pthread_t));

        int *mass = calloc(threads, sizeof(int));
        for (int i = 0; i < threads; i++) {
            mass[i] = i;
            (void) pthread_create(&thread[i], NULL, sort, &mass[i]);
        }

        for (int i = 0; i < threads; i++) {
            pthread_join(thread[i], 0);
        }

        print();
        int i = 0;
        for (; i < (int)count; i++) {
            memcpy((char*)address + 16 + i * 16, &buffer[i], 16);
        }

        free(mass);
        free(thread);
        end++;
        count_ -= count;
        
        munmap(address, memsize);
        free(buffer);
    }
    
    
    close(file);
    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&number_mutex);
    
    return 0;
}