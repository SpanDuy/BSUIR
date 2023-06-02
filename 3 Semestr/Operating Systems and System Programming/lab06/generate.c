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

pthread_barrier_t barrier;
pthread_mutex_t mutex;
data_type *buffer;
int start = 0;
void *address;
int block_status[BLOCKS];

void init_file(int buffer_size) {
    FILE* file = fopen(FILE_NAME, "wb");
    data_type data;
    data.recsords = buffer_size / 16;
    data.idx = (index_record*)calloc(data.recsords, sizeof(index_record));
    fwrite(&data.recsords, sizeof(uint64_t), 1, file);
    fwrite(&data.idx, sizeof(index_record *), 1, file);

    for (int i = 0; i < buffer_size / 16; i++) {
        if (!(i % (BLOCK_SIZE / 2))) {
            puts("");
        }
        index_record tmp;
        tmp.recno = i + 1;
        tmp.time_mark = ((double)rand() / RAND_MAX) * (MAX - MIN) + MIN;
        data.idx[i] = tmp;

        printf("%.2ld)\t%lf\t", tmp.recno, tmp.time_mark);
        fwrite(&tmp, sizeof(index_record), 1, file);
    }
    puts("");
    const char end = '\0';
    fwrite(&end, 1, 32, file);
    fclose(file);
}

int main(int argc, char *agrv[]) {
    if (argc != 2 || atoi(agrv[1]) % 4096) {
        puts("Вы ввели недопустимый размер");
        puts("Размер должен быть кратен 4096");
        return 1;
    }
    srand((unsigned int)getpid());
    init_file(atoi(agrv[1]));
    return 0;
}