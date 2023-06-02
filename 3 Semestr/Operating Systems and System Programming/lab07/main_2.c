#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>

#define MAX_RECORDS 10
#define RECORD_SIZE sizeof(struct record_s)

char examples_name[20][80] = { "Влад", "Иван", "Никита", "Рома", "Костя",
                               "Андрей", "Миша", "Радомир", "Паша", "Дима",
                               "Даша", "Таня", "Вика", "Настя", "Вероника", 
                               "Милана", "Юлия", "Маша", "Карина", "Лена" };

char examples_address[20][80] = { "Гомель", "Минск", "Гродно", "Брест", "Могилёв",
                               "Рогочёв", "Орша", "Витебск", "Хойники", "Калинковичи",
                               "Жлобин", "Ждановичи", "Речица", "Молодечно", "Пинск", 
                               "Кривичи", "Полоцк", "Лида", "Зона 51", "Гражданин мира" };

struct record_s {
    char name[80];
    char address[80];
    uint8_t semester;
};

struct current_record {
    struct record_s record;
    int id_record;
};

struct current_record current_rec;

void print_record(struct record_s *record, int record_number) {
    printf("Record    %d:\n", record_number);
    printf("Name:     %s\n", record->name);
    printf("Address:  %s\n", record->address);
    printf("Semester: %d\n", record->semester);
    printf("----------------------------\n");
}

void read_record(int file_descriptor, int record_number, struct record_s *record) {
    lseek(file_descriptor, record_number * RECORD_SIZE, SEEK_SET);
    read(file_descriptor, record, RECORD_SIZE);
}

void write_record(int file_descriptor, int record_number, struct record_s *record) {
    lseek(file_descriptor, record_number * RECORD_SIZE, SEEK_SET);
    write(file_descriptor, record, RECORD_SIZE);
}

void lock_record(int file_descriptor, int record_number) {
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = record_number * RECORD_SIZE;
    lock.l_len = RECORD_SIZE;

    fcntl(file_descriptor, F_SETLK, &lock);

    int flags = fcntl(file_descriptor, F_GETFL); // Получение текущих флагов состояния файла
    flags |= O_NONBLOCK; // Добавление флага O_NONBLOCK
    fcntl(file_descriptor, F_SETFL, flags);
}

void unlock_record(int file_descriptor, int record_number) {
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = record_number * RECORD_SIZE;
    lock.l_len = RECORD_SIZE;

    fcntl(file_descriptor, F_SETLK, &lock);

    int flags = fcntl(file_descriptor, F_GETFL); // Получение текущих флагов состояния файла
    flags |= O_NONBLOCK; // Добавление флага O_NONBLOCK
    fcntl(file_descriptor, F_SETFL, flags);
}

struct record_s generate_record() {
    struct record_s record;
    strcpy(record.name, examples_name[rand() % 20]);
    strcpy(record.address, examples_address[rand() % 20]);
    record.semester = rand() % 8 + 1;

    return record;
}

int main() {
    srand((unsigned int)getpid());
    int file_descriptor = open("records.txt", O_RDWR | O_CREAT);
    struct record_s records[MAX_RECORDS];
    struct record_s record;

    if (file_descriptor == -1) {
        perror("Error opening file");
        return 1;
    }

    char* menu = calloc(10, 1);
    while(1) {
        strcpy(menu,"\0");
        fgets(menu, sizeof(menu), stdin);
        if (strstr(menu, "q")) {
            break;
        } else if (strstr(menu, "LST")) {      // Отображение содержимого файла

            printf("File Contents:\n");
            for (int i = 0; i < MAX_RECORDS; i++) {
                struct record_s record;
                read_record(file_descriptor, i, &record);
                print_record(&record, i);
            }  

        } else if (strstr(menu, "GET(")) {      // Получение записи с введённым порядковым номером 

            int record_number = menu[4] - '0';
            struct record_s record;
            read_record(file_descriptor, record_number, &record);
            printf("Get Record %d:\n", record_number);
            print_record(&record, record_number);

            current_rec.record = record;
            current_rec.id_record = record_number;

        } else if (strstr(menu, "init")) {      // Создание и заполнение файла записями

            for (int i = 0; i < MAX_RECORDS; i++) {
                records[i] = generate_record();
            }

            lseek(file_descriptor, 0, SEEK_SET);
            write(file_descriptor, records, sizeof(records));

        } else if (strstr(menu, "PUT()")) {

            //puts("Начало...");
            puts("Введите ENTER для записи данных в файл");
            fgets(menu, sizeof(menu), stdin);
            write_record(file_descriptor, current_rec.id_record, &record);
            unlock_record(file_descriptor, current_rec.id_record);
            puts("Запись Сохранена");
            //puts("конец...");
            
        } else if (strstr(menu, "MODIFY")) {
            struct record_s REC_SAV;
            record = generate_record();
            puts("Сгенерирована Новая Запись:");
            printf("****************************\n");
            printf("Name:     %s\n", record.name);
            printf("Address:  %s\n", record.address);
            printf("Semester: %d\n", record.semester);
            printf("****************************\n");
            puts("Нажмите ENTER для Старта Записи её в Файл...");

            fgets(menu, sizeof(menu), stdin);
            puts("Начало...");
            
            int flag = 0;
            do {
                if (flag == 1) {
                    break;;
                }

                flag = 0;
                REC_SAV = current_rec.record;
                lock_record(file_descriptor, current_rec.id_record);
                struct record_s REC_NEW;
                //puts("Введить ENTER чтобы сравнить текущую запись");
                //fgets(menu, sizeof(menu), stdin);
                read_record(file_descriptor, current_rec.id_record, &REC_NEW);

                if (memcmp(&REC_NEW, &REC_SAV, sizeof(struct record_s)) != 0) {
                    puts("Записи не Совпали!!!");
                    unlock_record(file_descriptor, current_rec.id_record);
                    current_rec.record = REC_NEW;
                    flag = 1;
                }
            } while(flag);

            if (flag == 1) {
                puts("Теперь Текущая Запись:\n");
                printf("============================\n");
                printf("Get Record %d:\n", current_rec.id_record);
                printf("Name:     %s\n", current_rec.record.name);
                printf("Address:  %s\n", current_rec.record.address);
                printf("Semester: %d\n", current_rec.record.semester);
                printf("============================\n");
                continue;
            }

            puts("Запись Готова к Импортированию в Файл");
            puts("Введите PUT() для Изменения Файла...");
        }
    }

    free(menu);    
    close(file_descriptor);

    return 0;
}
