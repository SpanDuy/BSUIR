#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

bool info = 1;                  //доступ
int num_00 = 0;               //количество 00
int num_01 = 0;               //количество 01
int num_10 = 0;               //количество 10
int num_11 = 0;               //количество 11

struct child_stats statistic;  //статистика
int loop_count = 0;             //количество циклов
bool get_signal = 0;            //получен ли сигнал

//структура пар int
struct child_stats{        
    int num_1;
    int num_2;
};

//обработчик будильника
void alarm_handler(int signal) {           
    if (statistic.num_1 == 0 && statistic.num_2 == 0 )      {num_00++;}
    else if (statistic.num_1 == 1 && statistic.num_2 == 0)  {num_01++;}
    else if (statistic.num_1 == 0 && statistic.num_2 == 1)  {num_10++;}
    else if (statistic.num_1 == 1 && statistic.num_2 == 1)  {num_11++;}
    alarm(1 + rand() % 5);
}

//заполнение статистики
struct child_stats fill_stat(struct child_stats stat){      
    static int counter;
    switch (counter)
    {
    case 0:
        stat.num_1 = 0;
        stat.num_2 = 0;
        counter++;
        break;
    case 1:
        stat.num_1 = 1;
        stat.num_2 = 0;
        counter++;
        break;
    case 2:
        stat.num_1 = 0;
        stat.num_2 = 1;
        counter++;    
        break;
    case 3:
        stat.num_1 = 1;
        stat.num_2 = 1;
        counter++;    
        break;
    default:
        counter = 0;
        break;
    }
    return stat;
}

//обработчик для SIRUSR1 и SIRUSR2
void usr_handler(int signal){      
    if (signal == SIGUSR1) {
        info = false;
        get_signal=true;
    }
    else if (signal == SIGUSR2) {
        info = true;
        get_signal=true;
    }
}

int main() {
    srand(time(NULL));

    signal(SIGUSR1, usr_handler);       //инициализация обработчика SIRUSR1
    signal(SIGUSR2, usr_handler);       //инициализация обработчика SIRUSR2
    signal(SIGALRM, alarm_handler);     //инициализация обработчика будильника

    alarm(1+rand() % 5);                //будильник от 0 до 6

    while (true) {
        sleep(1);
        statistic = fill_stat(statistic);               //заполнение статистики
        if (loop_count >= 10 && info) {                 //прошло обольше 9 циклов
            get_signal = 0;
            alarm(0);                                   //отключение будильника
            union sigval inform;                        //отправка информации родителю
            inform.sival_int = getpid();

            while (!get_signal) {                       //ожидание сигнала от родитесля
                sigqueue(getppid(), SIGUSR1, inform);   //отправка сигнала и данные родителю
                sleep(10);
            }

            alarm(1 + rand() % 5);                      //будильник от 0 до 6

            if (!info) {
                loop_count = 0;
                info = 1;
                continue;
            }

            printf("\nppid - %d\npid - %d\n", (int)getppid(), (int)getpid());
            printf("0_0 - %d\n0_1 - %d\n1_0 - %d\n1_1 - %d\n",
             num_00, num_01, num_10, num_11);   //вывод статистики
            loop_count = 0;
            //sigqueue(getppid(), SIGUSR2, inform);
        }
        loop_count++;
    }
    exit(0);
}
