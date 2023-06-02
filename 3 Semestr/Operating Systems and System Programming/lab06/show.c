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
#include <sys/stat.h>

typedef struct index_s {
    double time_mark;   // временная метка (модифицированная юлианская дата)
    uint64_t recno;     // первичный индекс в таблице БД
} index_record;

typedef struct index_hdr_s {
    uint64_t recsords;      // количество записей
    index_record *idx;   // массив записей в количестве records
} data_type;

pthread_barrier_t barrier;
pthread_mutex_t mutex;
data_type *buffer;
int start = 0;
void *address;
int block_status[BLOCKS];

int main() {
    FILE *file;
    file = fopen(FILE_NAME, "rb");
    uint64_t count;

    fread(&count, sizeof(uint64_t), 1, file);
    fseek(file, sizeof(index_record *), SEEK_CUR);

    double temp_1;
    uint64_t temp_2;

    for (int i = 0; i < (int)count; i++) {
        fread(&temp_1, sizeof(double), 1, file);
        fread(&temp_2, sizeof(uint64_t), 1, file);
        printf("%.2ld)\t%lf;\t", temp_2, temp_1);
        if (!((i + 1) % BLOCK_SIZE)) {
            puts("");
        }
        if (!((i + 1) % 256)) {
            puts("");
        }
    }
    puts("");

    return 0;
}