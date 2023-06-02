/***************************************************************************/
// Подключаемые Стандартные Библиотеки

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

/***************************************************************************/
// Набор Констант для Работы с Файловай Системой

#define LINESIZE 128
#define BLOCK_SIZE 5000
#define DISK_PARTITION 400000
#define BLOCKS DISK_PARTITION/BLOCK_SIZE
#define MAX_STRING_LENGTH 20
#define MAX_SUBDIRECTORIES  (BLOCK_SIZE - 136)/MAX_STRING_LENGTH
#define MAX_FILE_DATA_BLOCKS (BLOCK_SIZE-64*59)
#define FILE_NAME "file.bin"
#define false 0
#define true 1

/***************************************************************************/
// Набор Цветов

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define PURPLE "\x1b[35m"
#define CYAN "\x1b[36m"
#define GRAY "\x1b[37m"
#define RESET "\x1b[0m"
#define LIGHT_RED "\x1b[1;31m"
#define LIGHT_GREEN "\x1b[1;32m"
#define LIGHT_YELLOW "\x1b[1;33m"
#define LIGHT_BLUE "\x1b[1;34m"
#define LIGHT_PURPLE "\x1b[1;35m"
#define LIGHT_CYAN "\x1b[1;36m"
#define LIGHT_GRAY "\x1b[1;37m"

/***************************************************************************/
// Функции с Валидациями 

int init (char *name, char *size);			// Инициализация Файловой Системы
int print(char *name, char *size);			// Вывод Файловой Системы на экран
int esc (char *name, char *size);			// Выход из Программы
int make_dir(char *name, char *size);		// Создание Каталога с Названием name
int change(char *name, char *size);			// Изменение Текущего Каталога на Каталог с Названием name
int rename_dir(char *name, char *size);		// Изменение Названия Текущего Каталога на name 
int make_file(char *name, char *size);		// Создагие Файла с Названием name и Размером size 
int rename_file(char *name, char *size);	// Переименование Файла с Названием name в Файл с Названием size
int delete_dir(char *name, char *size);		// Удаление Каталога с Названием name
int delete_file(char *name, char *size);	// Удаление Файла с Названием name
int clean(char *name, char *size);			// Очистка Файловой Системы

/***************************************************************************/
// Функции с Основным Выполнением Работы 

void print_tabs();											// Вывод Отступов для Демонстрации Вложенности
void printing(char *name);									// Вывод в STDOUT Название Каталога name и Его Подэлементы
void parse(char *buf, int *argc, char *argv[]);				// Парсер Команд Пользователя
int add_descriptor ( char * name );							// Добавление Дескриптора
int add_directory( char * name );							// Добавление Каталога c Названием name
int find_block ( char *name, int directory );				// Поиск Блока с Названием name и Спецификацией directory
int allocate_block ( char *name, int directory );			// Распределение Блока с Названием name и Спецификацией directory
int edit_directory (char * name,  char*subitem_name, 		// Изменить Каталог с Названием name, Подэлентами subitem_name,
	char *new_name, int name_change, int directory );			// с Новое Название new_name и Флагами name_change и directory
void print_directory ( char *name);							// Вывод Данных Каталога name
int edit_descriptor ( int free_index, int free, 			// Изменения Дескриптора 
	int name_index, char * name );
void get_directory_top_level ( char*name,					// Получение Родительского Каталога name в tmp
	char tmp[MAX_STRING_LENGTH]);	
char * get_directory_subitem ( char*name, 					// Получение Подэлемента Каталога name по Индексу subitem_index,
	int subitem_index, char*subitem_name );						// или Добавление Подэлемента subitem_name в Каталог name
void get_directory_name ( char*name , 						// Получение Названия Каталога name в tmp
	char tmp[MAX_STRING_LENGTH]);		
int add_file( char * name, int size );						// Добавление Файла с Названием name и Размером size
void print_file ( char *name);								// Вывод Данных Файла с Названием name
int edit_file ( char * name, int size, char *new_name );	// Изменение Файла с Названием name и Новым Названием new_name
void get_file_top_level ( char*name, 						// Получение Родительского Каталога Файла с Названием name
	char top_level[MAX_STRING_LENGTH]);							// в top_level
int edit_directory_subitem (char* name, 					// Изменение Данных Подэлемента sub_name в new_sub_name
	char* sub_name, char* new_sub_name);						// в Каталоге name
int edit_descriptor_name (int index, char* new_name);		// Измененеие Названия Дескриптора с Индексом index в new_name
void save();												// Сохранение Файловой Системы в Файл
int open();													// Открытие Файла с Файловой Системой
int remove_directory( char * name );						// Удаление Каталога с Названием name
int remove_file (char* name);								// Удаление Файла с Названием name
void unallocate_block ( int offset );						// Освобождение Блока с Индексом offset

/***************************************************************************/
// Структуры Данных

// Структура Текущего Каталога
typedef struct {						
	char directory[MAX_STRING_LENGTH];			// Название Каталога
	int directory_index;						// Индекс Каталога
	char parent[MAX_STRING_LENGTH];				// Название Родительского Каталога
	int parent_index;							// Индекс Родительского Каталога
} working_directory;

// Структура Каталога
typedef struct dir_type {
	char name[MAX_STRING_LENGTH];				// Название Каталога
	char top_level[MAX_STRING_LENGTH];			// Название Родительского Каталога
	char subitem[BLOCKS][MAX_STRING_LENGTH];	// Массив Названий Подэлементов
	int subitem_type[MAX_SUBDIRECTORIES];		// Массив Типов Подэлементов
	int subitem_count;							// Количество Подэлементов
} dir_type;

// Структура Дескриптора Блоков
typedef struct {
	int free[BLOCKS];							// Массив Статуса Блоков
	int directory[BLOCKS];						// Массив Спецификаций Блоков
	char name[BLOCKS][MAX_STRING_LENGTH];		// Массив Названий Блоков
} descriptor_block;

// Структура Файла
typedef struct file_type {
	char name[MAX_STRING_LENGTH];				// Название Файла
	char top_level[MAX_STRING_LENGTH];			// Название Родительского Каталога
	int data_block_index[MAX_FILE_DATA_BLOCKS];	// Индекс Блока
	int data_block_count;						// Количество Блоков
	int size;									// Размер Файла
} file_type;

// Структура Команды
struct action {
  char *cmd;									// Команда Для Выполнения
  int (*action)(char *name, char *size);		// Соответствующая Функция Для Вызова
} table[] = {									// Список Пар Команд и Функций
    { "init" , init  },							// Инициализация Файловой Системы
    { "print", print },							// Вывод Файловой Системы на экран
	{ "make_dir", make_dir },					// Создание Каталога
	{ "change", change},						// Изменение Текущего Каталога
	{ "rename_dir", rename_dir},				// Изменение Названия Текущего Каталога
	{ "make_file", make_file},					// Создагие Файла
	{ "rename_file", rename_file},				// Переименование Файла
	{ "delete_dir", delete_dir },				// Удаление Каталога
	{ "delete_file", delete_file },				// Удаление Файла
	{ "clean", clean },							// Очистка Файловой Системы
    { "esc" , esc  },							// Выход из Программы
    { NULL, NULL }								// Метка Конца Пар
};

/***************************************************************************/
// Глобальные Переменные

int debug = 1;					// Флаг Вывода Сообщений об Ошибках
char *disk;						// Диск, Который Хранит Данные о Файловой Системе
working_directory current;		// Текущий Каталог
int disk_allocated = false;		// Флаг Инициализованности Диска
int deeping = 0;				// Вложенность Каталога
int free_count = BLOCKS;		// Количество Свободных Блоков