#include "sfs.h"

/***************************************************************************/
// Main Функция

int main(int argc, char *argv[])
{
	(void)argc;
	(void)*argv;
    char in[LINESIZE];
    char *cmd;
	char *fnm;
	char *fsz;
    char dummy[] = "";

	printf("<<<<<<<<<<<<<<<<<<<< SFS Файловая Система >>>>>>>>>>>>>>>>>>>>\n");

    int n;
    char *a[LINESIZE];
  
   	while (fgets(in, LINESIZE, stdin) != NULL)	// Цикл Обработки Команд
    {
		parse(in, &n, a);		// Вызов Парсера Команд

    	cmd = (n > 0) ? a[0] : dummy;
    	fnm = (n > 1) ? a[1] : dummy;
     	fsz = (n > 2) ? a[2] : dummy;

    	if (n == 0) {	// Пустая Строка
			continue;
		}	

    	int found = 0;
     
    	for (struct action *ptr = table; ptr->cmd != NULL; ptr++){  
            if (strcmp(ptr->cmd, cmd) == 0)
                {
                    found = 1;
                    
                    int ret = (ptr->action)(fnm, fsz);	// Выполнение Команды
                    if (ret == -1)
                        { printf("  %s %s %s: failed\n", cmd, fnm, fsz); }
					break;
    			}
		}
    	if (!found) { 	// Команда не Введена 
    		printf("Введена Несуществующая Команда: %s%s%s\n", RED, cmd, RESET);
    	}
		save();		// Сохранение Файловой Системы
    }

  return 0;
}

/***************************************************************************/
// Инициализация Файловой Системы

int init(char *name, char *size)
{
	(void)*name;
	(void)*size;

	if ( disk_allocated == true ) {	// Диск Распределен
		return 0;
	}
		
	disk = (char*)calloc ( 1, DISK_PARTITION );	// Выделение Памяти для Диска


	if (open()) {	// Чтение Файловой Системы из Файла
		strcpy(current.directory, "root");	// Установление root как Текущего Каталога 
		current.directory_index = 3;
		strcpy(current.parent, "" );
		current.parent_index = -1;

		disk_allocated = true;	// Диск Распределен

		return 0;
	}


	add_descriptor("descriptor");	// Создание Дескриптора
	add_directory("root");	// Создание Корневого Каталога
	
	strcpy(current.directory, "root");	// Установление root как Текущего Каталога 
	current.directory_index = 3;
	strcpy(current.parent, "" );
	current.parent_index = -1;
	
	disk_allocated = true;	// Диск Распределен
	
 	return 0;
}

/***************************************************************************/
// Вывод Файловой Системы на экран

int print(char *name, char *size)
{
	(void)*name;
	(void)*size;
	if ( disk_allocated == false ) {
		 
		return 0;
	}
	printf("==================================================\n");

	printing("root");	// Вывод на Экран
	
	if (debug) {
		if ( debug ) {
			 printf("==================================================\n");
		}
	}
	return 0;
}

/***************************************************************************/
// Вывод Отступов для Демонстрации Вложенности

void print_tabs() {
	for (int i = 0; i < deeping; i++) {
		printf("\t");
	}
}

/***************************************************************************/
// Вывод в STDOUT Название Каталога name и Его Подэлементы

void printing(char *name) {
	dir_type *folder = calloc (1, BLOCK_SIZE);
	int block_index = find_block(name, true);

	memcpy( folder, disk + block_index*BLOCK_SIZE, BLOCK_SIZE);
	
	print_tabs();	// Вывод Вложенности
	if (!strcmp(current.directory, folder->name)) {
		printf("\x1b[7;32m%s%s:\n", folder->name, RESET);	// Особый Вывод Текущего Каталога
	} else {
		printf("%s%s%s:\n", GREEN, folder->name, RESET);
	}
	for( int i = 0; i < folder->subitem_count; i++ ) {
		if( folder->subitem_type[i] == true ) {
			deeping++;
			printing(folder->subitem[i]);
			deeping--;
		} else {
			print_tabs();
			printf("\t%s%s%s\n", BLUE, folder->subitem[i], RESET );
		}
	}
	free(folder);
}

/***************************************************************************/
// Выход из Программы

int esc(char *name, char *size)
{
	descriptor_block *descriptor = calloc( 1, BLOCK_SIZE );

	memcpy ( descriptor, disk, BLOCK_SIZE*1 );

	free(descriptor);
	free(disk);

	(void)*name;
	(void)*size;
	if (debug) printf("\tВыход\n");
	exit(0);
	return 0;
}

/***************************************************************************/
// Парсер Команд Пользователя

void parse(char *buf, int *argc, char *argv[])
{
  char *delim;          // points to first space delimiter
  int count = 0;        // number of args

  char whsp[] = " \t\n\v\f\r";          // whitespace characters

  while (1)                             // build the argv list
    {
      buf += strspn(buf, whsp);         // skip leading whitespace
      delim = strpbrk(buf, whsp);       // next whitespace char or NULL
      if (delim == NULL)                // end of line, input parsed
        { break; }
      argv[count++] = buf;              
      *delim = '\0';                    
      buf = delim + 1;                  
    }
  argv[count] = NULL;

  *argc = count;

  return;
}

/***************************************************************************/
// Добавление Дескриптора

int add_descriptor ( char * name ) {
	descriptor_block *descriptor = calloc( 2, BLOCK_SIZE );
	
	for (int i = 0; i < BLOCKS; i++ ) {
		descriptor->free[i] = true;
		descriptor->directory[i] = false;
	}
	

	int limit = (int)(sizeof(descriptor_block)/BLOCK_SIZE) + 1;
	
	for ( int i = 0; i < limit; i ++ ) {
		descriptor->free[i]= false; 
	}
	
	strcpy(descriptor->name[0], "descriptor"); 	

	memcpy ( disk, descriptor, (BLOCK_SIZE*(limit+1)));

	free(descriptor);

	return 0;	
}

/***************************************************************************/
// Добавление Каталога c Названием name

int add_directory( char * name ) {
	if ( strcmp(name,"") == 0 ) {
		if ( debug ) {
     		printf("%sОшибка%s : Пропущен Операнд\n", RED, RESET );
    	}
		return -1;
	}
	if (free_count == 0) {
		printf("%sОшибка%s : Недостаточно Памяти\n", RED, RESET );
		return -1;
	}
	
	dir_type *folder = calloc ( 1, BLOCK_SIZE);
	
	strcpy(folder->name, name);					
	strcpy(folder->top_level, current.directory);
	folder->subitem_count = 0;				
	
	int index = allocate_block(name, true);
	free_count--;
		
	memcpy( disk + index*BLOCK_SIZE, folder, BLOCK_SIZE);
	
	free(folder);
	return 0;
}

/***************************************************************************/
// Поиск Блока с Названием name и Спецификацией directory

int find_block ( char *name, int directory ) {

	descriptor_block *descriptor = calloc( 1, BLOCK_SIZE );

	memcpy ( descriptor, disk, BLOCK_SIZE*1 );
	
	for ( int i = 0; i < BLOCKS; i++ ) {
		if (descriptor->free[i] == 0) {
			if ( strcmp(descriptor->name[i], name) ==0 ){
				if ( descriptor->directory[i] == directory ) {
					free(descriptor);
					return i;
				}
			}
		}
	}
	
	free(descriptor);
	return -1;
}

/***************************************************************************/
// Распределение Блока с Названием name и Спецификацией directory

int allocate_block ( char *name, int directory ) { 

	descriptor_block *descriptor = calloc( 2, BLOCK_SIZE );

	memcpy ( descriptor, disk, BLOCK_SIZE * 2 );

	for ( int i = 0; i < BLOCKS; i++ ) {
		 if ( descriptor->free[i] ) {
			descriptor->free[i] = false;
			descriptor->directory[i] = directory;
			strcpy(descriptor->name[i], name);
			
			memcpy(disk, descriptor, BLOCK_SIZE*2);
			free(descriptor);
			return i; 
		}
	}
	free(descriptor);
	if ( debug ) {
		printf("%sОшибка%s : Нет Свободного Места\n", RED, RESET );
	}
	return -1;
}

/***************************************************************************/
// Создание Каталога с Названием name

int make_dir(char *name, char *size)
{
	(void)*size;
	if ( disk_allocated == false ) {
		return 0;
	}	

	if (find_block(name, 1) != -1) {
		printf( "Нельзя Создать Каталог '%s%s%s': Такой Блок Уже Есть\n", RED, name, RESET );
		return 0;
	}

	if ( add_directory( name ) != 0 ) {
		if (!debug ) {
			printf("%sОшибка%s : Пропущен Операнд\n", RED, RESET);
		}
		return 0;
	}

	edit_directory( current.directory, name, NULL, false, true );

	descriptor_block *descriptor = calloc( 1, BLOCK_SIZE);

	memcpy ( descriptor, disk, BLOCK_SIZE );

	if ( debug ) {
		print_directory(name);
	}

	free(descriptor);

  	return 0;
}

/***************************************************************************/
// Изменить Каталог с Названием name, Подэлентами subitem_name, 
	// с Новое Название new_name и Флагами name_change и directory

int edit_directory (char * name,  char*subitem_name, char *new_name, int name_change, int directory ) {
	
	if( strcmp(name,"") == 0 ) {
		if ( debug ) {
     		printf("%sОшибка%s : Пропущен Операнд\n", RED, RESET );
    	}
		return -1;
	}

	dir_type *folder = calloc ( 1, BLOCK_SIZE);
	
	int block_index = find_block(name, true);
	if( block_index == -1 ) {
		if ( debug ) {
			printf("Каталог %s%s%s Не Существует\n", RED, name, RESET );
		}
		return -1;
	}
	
	memcpy( folder, disk + block_index*BLOCK_SIZE, BLOCK_SIZE);

	if ( strcmp(subitem_name, "") != 0 ) {	
		
		if ( !name_change ) {    
			strcpy (folder->subitem[folder->subitem_count], subitem_name );
			folder->subitem_type[folder->subitem_count] = directory;
			folder->subitem_count++;

			memcpy( disk + block_index*BLOCK_SIZE, folder, BLOCK_SIZE);
			
			free(folder);
			return 0;
		}
		else {			
			for ( int i =0; i < folder->subitem_count; i++ ) {
				if ( strcmp(folder->subitem[i], subitem_name) == 0 ) {
					strcpy( folder->subitem[i], new_name);
					memcpy( disk + block_index*BLOCK_SIZE, folder, BLOCK_SIZE);
					free(folder);
					return 0;
				}
			}
			free(folder);
			return -1;
		}
	}
	else {					
		int block_index2 = find_block(new_name, true);
		if( block_index2 != -1 ) {
			if ( debug ) {
				printf("Каталог %s%s%s Уже Существует. Выберете Другое Название\n", RED, new_name, RESET);
			}
			return -1;
		}
	
		strcpy(folder->name, new_name );
		
		memcpy( disk + block_index*BLOCK_SIZE, folder, BLOCK_SIZE);
		
		edit_descriptor(-1, false, block_index, new_name );

		if ( debug ) {
			print_directory(folder->name);
		}
		
		edit_directory(folder->top_level, name, new_name, true, true );
		
		int child_index;

		for ( int i = 0; i < folder->subitem_count; i++) {
			file_type *child_file = calloc ( 1, BLOCK_SIZE);
			dir_type *child_folder = calloc ( 1, BLOCK_SIZE);
			
			child_index = find_block ( folder->subitem[i], folder->subitem_type[i]);
			if ( folder->subitem_type[i] ) {
				memcpy( child_folder, disk + child_index*BLOCK_SIZE, BLOCK_SIZE);
				strcpy( child_folder->top_level, new_name );
				
				memcpy( disk + child_index*BLOCK_SIZE, child_folder, BLOCK_SIZE);
				free ( child_folder );
				free ( child_file );
			}
			else {
				memcpy( child_file, disk + child_index*BLOCK_SIZE, BLOCK_SIZE);
				strcpy( child_file->top_level, new_name );
			
				memcpy( disk + child_index*BLOCK_SIZE, child_file, BLOCK_SIZE);	
				free ( child_folder );
				free ( child_file );
			} 
		}
			
		free(folder);
		return 0;
	}
		
	free ( folder );
}

/***************************************************************************/
// Вывод Данных Каталога name

void print_directory ( char *name) {
	dir_type *folder = calloc( 1, BLOCK_SIZE);
	int block_index = find_block(name, true);
	memcpy( folder, disk + block_index*BLOCK_SIZE, BLOCK_SIZE);
	
	printf("\t=============================\n");
 	printf("\tСведения О Каталоге:\n");
  	printf("\tНазвание - %s%s%s\n", GREEN, folder->name, RESET);
  	printf("\tРодительский Каталог - %s%s%s\n", GREEN, folder->top_level, RESET);
  	printf("\tСодержимое:\n");
	for (int i = 0; i < folder->subitem_count; i++) {
    	if (folder->subitem_type[i]) {
			printf( "\t\t%s%s%s\n", GREEN, folder->subitem[i], RESET);
    	} else {
    		printf( "\t\t%s%s%s\n", BLUE, folder->subitem[i], RESET);
    	}
	}
	printf("\tКоличество Элементов = %d\n", folder->subitem_count);
	printf("\t=============================\n");
	
	free(folder);
}

/***************************************************************************/
// Изменения Дескриптора 

int edit_descriptor ( int free_index, int free_, int name_index, char * name ) {

	descriptor_block *descriptor = calloc(2, BLOCK_SIZE );
	
	memcpy ( descriptor, disk, BLOCK_SIZE*2 );

	if ( free_index > 0 ) {
		descriptor->free[free_index] = free_;
	}
	if ( name_index > 0 ) {
		strcpy(descriptor->name[name_index], name );
	}
		
	memcpy(disk, descriptor, BLOCK_SIZE*2);

	free(descriptor);

	return 0;
}

/***************************************************************************/
// Изменение Текущего Каталога на Каталог с Названием name

int change(char *name, char *size)
{
	(void)*size;
	if ( disk_allocated == false ) {
		printf("%sОшибка%s ", RED, RESET);
    	puts(" : Диск Не Размещён");
		return 0;
	}
	
	if ( strcmp(name, ".." ) == 0 ) {
		
		if ( strcmp(current.directory, "root") == 0 ) {
			return 0;
		}
		
		strcpy ( current.directory, current.parent );	
		get_directory_top_level( current.parent, current.parent);
		return 0;
	}
	else
	{	
		char tmp[20];

		if ( find_block(name, 1) == -1 && strcmp( current.parent, name ) != 0 ) {
			if ( debug ) {
				printf( "Нельзя Перейти К Каталогу %s%s%s\n", RED, name, RESET );
			}
			if (!debug ) {
				printf( "%s: %s%s%s: Нет Файла или Каталога С Таким Названием\n", "chdir", RED, name, RESET);
			}
			return 0;
		}
		get_directory_name(name, tmp);
		if ( strcmp(tmp, "") == 0 ) {
			return 0;
		}
		
		if ( strcmp( tmp, name ) == 0 ) {
			strcpy ( current.directory, tmp);

			get_directory_top_level(name, current.parent);

			return 1;
		}
		return -1;
	}
  return 0;
}

/***************************************************************************/
// Получение Родительского Каталога name в tmp

void get_directory_top_level ( char*name, char tmp_[20]) {
	dir_type *folder = calloc ( 1, BLOCK_SIZE);
	char *tmp = calloc(1, sizeof(char)*MAX_STRING_LENGTH); 

	int block_index = find_block(name, true);
	if ( block_index == -1 )  {
		if ( debug ) {
			printf("Каталог %s%s%s Не Найден\n", RED, name, RESET);
		}
		strcpy ( tmp, "");
		strcpy(tmp_, tmp);
		free(folder);
		free(tmp);
	}
	
	memcpy( folder, disk + block_index*BLOCK_SIZE, BLOCK_SIZE);
	
	strcpy( tmp, folder->top_level);
	strcpy(tmp_, tmp);
	free ( folder );
	free(tmp);
}

/***************************************************************************/
// Получение Подэлемента Каталога name по Индексу subitem_index,
	// или Добавление Подэлемента subitem_name в Каталог name

char * get_directory_subitem ( char*name, int subitem_index, char*subitem_name ) {
	dir_type *folder = calloc ( 1, BLOCK_SIZE);
	char *tmp = calloc(1, sizeof(char)*MAX_STRING_LENGTH); 

	int block_index = find_block(name, true);
	if ( block_index == -1 ) {
		if ( debug ) {
    		printf("Каталог %s%s%s Не Найден\n", RED, name, RESET);
   		}
		strcpy ( tmp, "");
		free ( folder );
		return tmp;
	}

	memcpy( folder, disk + block_index*BLOCK_SIZE, BLOCK_SIZE);
	
	if ( subitem_index >= 0 ) { 
		strcpy( tmp, folder->subitem[subitem_index]);
		free(folder);
		return tmp;
	}
	else { 			 
		for ( int i =0; i < folder->subitem_count; i ++ ) {
			if ( strcmp( folder->subitem[i], subitem_name ) == 0 ) {
				free ( tmp );
				free ( folder );
				return "0";
			}
		}
		free ( tmp );
		free ( folder );
		return "-1";
	}
	free ( folder );
	return tmp;
}

/***************************************************************************/
// Получение Названия Каталога name в tmp

void get_directory_name ( char*name , char tmp_[20]) {
	dir_type *folder = calloc ( 1, BLOCK_SIZE);
	char *tmp = calloc(1, sizeof(char)*MAX_STRING_LENGTH); 
		
	int block_index = find_block(name, true);
	if ( block_index == -1 )  {
		if ( debug ) {
    		printf("Каталог %s%s%s Не Найден\n", RED, name, RESET);
    	}
		strcpy ( tmp, "");
		strcpy(tmp_, tmp);
	}
	
	memcpy( folder, disk + block_index*BLOCK_SIZE, BLOCK_SIZE);
	
	strcpy( tmp, folder->name);
		
	strcpy(tmp_, tmp);

	free(tmp);
	free ( folder );
}

/***************************************************************************/
// Изменение Названия Текущего Каталога на name 

int rename_dir(char *name, char *size) //"size" is actually the new name
{
	if ( disk_allocated == false ) {
		return 0;
	}

	if (!strcmp("root", current.directory)) {
		printf( "%s: Невозможно Переименовать Корневой Каталог '%s%s%s'\n", "rename_dir", RED, "root", RESET );
		return 0;
	}

	if( edit_directory( current.directory, "", name, true, true ) == -1 ) {
		if (!debug ) {
			printf( "%s: Невозможно Переименовать Файл Или Каталог '%s%s%s'\n", "rename_dir", RED, name, RESET );
		}
		return 0;
	}

	change(name, NULL);
	
	return 0;
}

/***************************************************************************/
// Создагие Файла с Названием name и Размером size 

int make_file(char *name, char *size)
{
	if ( disk_allocated == false ) {
		return 0;
	}
	
	if ( !strcmp(get_directory_subitem(current.directory, -1, name), "0") ) {
		if ( debug ) {
			printf( "Невозможно Создать Файл %s%s%s, Файл Или Каталог С Таким Названием Уже Существует\n", RED, name, RESET );
		}
		return 0;
	}
	
	if ( add_file ( name, atoi(size)) != 0 ) {
		return 0;
	}
	
  	edit_directory( current.directory, name, NULL, false, false);
  	
  	if ( debug ) {
		print_file(name);
	}
  	return 0;
}

/***************************************************************************/
// Добавление Файла с Названием name и Размером size

int add_file( char * name, int size ) {
	char subname[20];
	
	if ( size < 0 || strcmp(name,"") == 0 ) {
		if ( debug ) {
			printf("%s Неизвесная Команда\n", __func__);
		}
		if (!debug ) {
			printf("%s: Пропущен Операнд\n", "add_file");
		}
		return 1;
	}
	if ((free_count - size/BLOCK_SIZE == 0) || (free_count - size/BLOCK_SIZE < 0)) {
		printf("%sОшибка%s : Недостаточно Памяти\n", RED, RESET );
		return 1;
	}
	
	file_type *file = calloc ( 1, BLOCK_SIZE );
		
	strcpy( file->name, name);	
	strcpy ( file->top_level, current.directory );
	file->size = size;		
	file->data_block_count = 0;

	int index = allocate_block(name, false);

	for ( int i = 0; i < size/BLOCK_SIZE + 1; i++ ) {
		sprintf(subname, "%s->%d", name, i);
		file->data_block_index[i] = allocate_block(subname, false);
		file->data_block_count++;
		free_count--;
	}  
	memcpy( disk + index*BLOCK_SIZE, file, BLOCK_SIZE);
	
	free(file);
	return 0;
}

/***************************************************************************/
// Вывод Данных Файла с Названием name

void print_file ( char *name) {
	file_type *file = calloc( 1, BLOCK_SIZE);
	int block_index = find_block(name, false);
	memcpy( file, disk + block_index*BLOCK_SIZE, BLOCK_SIZE);
	
	printf("\t=============================\n");
  	printf("\tСведения О Файле:\n");
  	printf("\tНазвание - %s%s%s\n", BLUE, file->name, RESET);
  	printf("\tРодительский Каталог - %s%s%s\n", GREEN, file->top_level, RESET);
	printf("\tРазмер Файла - %s%d%s\n", YELLOW, file->size, RESET);  	
	printf("\t=============================\n");
	
	free(file);
}

/***************************************************************************/
// Переименование Файла с Названием name в Файл с Названием size

int rename_file(char *name, char *size)
{
	if ( disk_allocated == false ) {
		return 0;
	}
	
	if (!strcmp(size, "")) {
		printf("\t%sОшибка%s : Пропущен Операнд\n", RED, RESET );
		return -1;
	}

	if ( !strcmp(get_directory_subitem(current.directory, -1, size), "0") ) {
		if ( debug ) {
			printf( "Нельзя Переименовать Файл %s%s%s, Файл Или Каталог С Таким Названием Уже Существует\n", RED, name, RESET );
		}
		if (!debug ) {
			printf( "%s: Нельзя Переименовать Файл Или Каталог '%s%s%s'\n", "rename_file", RED, name, RESET );
		}
		return 0;
	}

	int er = edit_file( name, 0, size);
	
	if (er == -1) {
		return -1;
	}
	if (debug) {
		print_file(size);
	}

	return 0;
}

/***************************************************************************/
// Изменение Файла с Названием name и Новым Названием new_name

int edit_file ( char * name, int size, char *new_name ) {
	file_type *file = calloc ( 1, BLOCK_SIZE);
	
	int block_index = find_block(name, false);
	if ( block_index == -1 )  {
		if ( debug ) {
			printf("Файл %s%s%s Не Найден\n", BLUE, name, RESET );
		}
		return -1;
	}

	memcpy( file, disk + block_index*BLOCK_SIZE, BLOCK_SIZE);
		  
	char top_level[MAX_STRING_LENGTH];

	get_file_top_level(name, top_level);

	edit_directory_subitem(top_level, name, new_name); 

	edit_descriptor_name(block_index, new_name); 

	strcpy(file->name, new_name );
	memcpy( disk + block_index*BLOCK_SIZE, file, BLOCK_SIZE);	

	free(file);
	return 0;
}

/***************************************************************************/
// Получение Родительского Каталога Файла с Названием name
	// в top_level

void get_file_top_level ( char*name, char top_level[20]) {
	file_type *file = calloc ( 1, BLOCK_SIZE);
	char *tmp = calloc(1, sizeof(char)*MAX_STRING_LENGTH); 

	int block_index = find_block(name, false);
		
	memcpy( file, disk + block_index*BLOCK_SIZE, BLOCK_SIZE);
	
	strcpy( tmp, file->top_level);

	strcpy(top_level, tmp);
	
	free ( tmp );
	free ( file );
}

/***************************************************************************/
// Изменение Данных Подэлемента sub_name в new_sub_name
	// в Каталоге name

int edit_directory_subitem (char* name, char* sub_name, char* new_sub_name)
{
	dir_type *folder = calloc ( 1, BLOCK_SIZE);

	int block_index = find_block(name, true);
	if ( block_index == -1 ) {
		if ( debug ) {
			printf("Каталог %s%s%s Не Найден\n", RED, name, RESET );
		}
	}
	
	memcpy( folder, disk + block_index*BLOCK_SIZE, BLOCK_SIZE);

	const int cnt = folder->subitem_count;	
	int i;
	for (i=0; i < cnt; i++)
	{
		if (strcmp(folder->subitem[i], sub_name) == 0){
			strcpy(folder->subitem[i], new_sub_name);

			memcpy(disk + block_index*BLOCK_SIZE ,folder, BLOCK_SIZE);
			free(folder);
			return i;
		}
	}

	free(folder);
	return -1;
}

/***************************************************************************/
// Измененеие Названия Дескриптора с Индексом index в new_name

int edit_descriptor_name (int index, char* new_name) {
	descriptor_block *descriptor = calloc( 2, BLOCK_SIZE );

	memcpy ( descriptor, disk, BLOCK_SIZE*2 );

	strcpy(descriptor->name[index], new_name);

	memcpy(disk, descriptor, BLOCK_SIZE*2);

	free(descriptor);
	return 0;
}

/***************************************************************************/
// Открытие Файла с Файловой Системой

int open() {
	FILE *fp;

    fp = fopen(FILE_NAME, "rb");
	if(!fp) {
		return -1;
	}

	fread(disk, sizeof(char), DISK_PARTITION, fp);

    fclose(fp);
	
	return 1;
}

/***************************************************************************/
// Сохранение Файловой Системы в Файл

void save() {
	FILE *fp;

    fp = fopen(FILE_NAME, "wb");
	
	fwrite(disk, sizeof(char), DISK_PARTITION, fp);

    fclose(fp);
}

/***************************************************************************/
// Удаление Каталога с Названием name

int delete_dir(char *name, char *size)
{
	(void)*size;
	if ( disk_allocated == false ) {
		return 0;
	}
		
	if ( strcmp(name,"") == 0 ) {
		if ( debug ) {
			printf("%s%s%s Неверная Команда\n", RED, __func__, RESET );
		}
		if (!debug ) {
			printf("%s: Пропущен Операнд\n", "delete_dir");
		}
		return 0;
	}
	
	if( (strcmp(name, ".") == 0) || (strcmp(name, "..") == 0) ) {
		if ( debug ) {
			printf("%s Неверная Команда %s%s%s Каталог Не Будет Удалён\n", __func__, RED, name, RESET);
		}
		if (!debug ) {
			printf( "%s: %s%s%s: Нет Каталога Или Файла\n", "delete_dir", RED, name, RESET );
		}
		return 0;
	}
	
	if ( strcmp(get_directory_subitem(current.directory, -1, name), "-1") == 0 ) {
		if ( debug ) {
			printf( "%s Нельзя Удалить Каталог %s%s%s\n", __func__, RED, name, RESET );
		}
		if (!debug ) {
			printf( "%s: %s%s%s: Нет Каталога Или Файла\n", "delete_dir", RED, name, RESET );
		}
		return 0;
	}
	
	dir_type *folder = calloc ( 1, BLOCK_SIZE );
	int block_index = find_block(name, true);
	memcpy( folder, disk + block_index*BLOCK_SIZE, BLOCK_SIZE );

	dir_type *top_folder = calloc ( 1, BLOCK_SIZE );

	int top_block_index = find_block(folder->top_level, true);
	memcpy( disk + block_index*BLOCK_SIZE, folder, BLOCK_SIZE );
	memcpy( top_folder, disk + top_block_index*BLOCK_SIZE, BLOCK_SIZE );

	char subitem_name[MAX_STRING_LENGTH]; 
	const int subcnt = top_folder->subitem_count; 
	int j;
	int k=0;

	for(j = 0; j<subcnt; j++) {
		strcpy(subitem_name, top_folder->subitem[j]);
		if (strcmp(subitem_name, name) != 0)
		{
			strcpy(top_folder->subitem[k],subitem_name);
			k++;
		}
	}
	
	strcpy(top_folder->subitem[k], "");

	top_folder->subitem_count--;
	memcpy( disk + top_block_index * BLOCK_SIZE, top_folder, BLOCK_SIZE );
	free(top_folder);
	
	if( remove_directory( name ) == -1 ) {
		free ( folder );
		return 0;
	}
	free ( folder );
	return 0;
}

/***************************************************************************/
// Удаление Каталога с Названием name

int remove_directory( char * name ) {
	
	dir_type *folder = calloc (1, BLOCK_SIZE);
	int block_index = find_block(name, true);
	
	if( block_index == -1 ) {
		if ( debug ) {
			printf("%s Каталог %s%s%s Не Существует В Текущем Каталоге %s%s%s\n", __func__, RED, name, RESET, GREEN, current.directory, RESET);
		}
		return -1;
	}

	memcpy( folder, disk + block_index*BLOCK_SIZE, BLOCK_SIZE );

	for( int i = 0; i < folder->subitem_count; i++ ) {
		if (folder->subitem_type[i] == true) {
			remove_directory(folder->subitem[i]);
		} else {
			remove_file(folder->subitem[i]);
		}
	}
	unallocate_block(block_index);
	free(folder);
	
	return 0;
}

/***************************************************************************/
// Удаление Файла с Названием name

int remove_file (char* name)
{
	file_type *file = calloc ( 1, BLOCK_SIZE);
	dir_type *folder = calloc ( 1, BLOCK_SIZE);
	
	int file_index = find_block(name, false);

	memcpy( file, disk + file_index*BLOCK_SIZE, BLOCK_SIZE);
	
	int folder_index = find_block(file->top_level, true);
	
	memcpy( folder, disk + folder_index*BLOCK_SIZE, BLOCK_SIZE);

	char subitem_name[MAX_STRING_LENGTH]; 
	const int subcnt = folder->subitem_count; 
	int j;
	int k=0;
	for(j = 0; j<subcnt; j++) {
		strcpy(subitem_name, folder->subitem[j]);
		if (strcmp(subitem_name, name) != 0) {
			strcpy(folder->subitem[k],subitem_name);
			k++;
		}
	}
	strcpy(folder->subitem[k], "");
	folder->subitem_count--;

	memcpy(disk + folder_index*BLOCK_SIZE, folder, BLOCK_SIZE);

	int i = 0;
	while(file->data_block_count != 0) {
		unallocate_block(file->data_block_index[i]);
		file->data_block_count--;
		i++;
	}
	
	unallocate_block(file_index); 
	
	free(folder);
	free(file);
	return 0;
}

/***************************************************************************/
// Освобождение Блока с Индексом offset

void unallocate_block ( int offset ) { 
	descriptor_block *descriptor = calloc( 2, BLOCK_SIZE );
	
	memcpy ( descriptor, disk, BLOCK_SIZE*2 );
	
	descriptor->free[offset] = true;
	strcpy( descriptor->name[offset], "" );

	memcpy ( disk, descriptor, BLOCK_SIZE*2 );	
	
	free(descriptor);
}

/***************************************************************************/
// Удаление Файла с Названием name

int delete_file(char *name, char *size)
{
	if ( disk_allocated == false ) {
		return 0;
	}
	
	(void)*size;

	if ( !strcmp(get_directory_subitem(current.directory, -1, name), "0") ) {
		remove_file(name);
		return 0;
	} else {
		if ( debug ) {
			printf( "%s Недьзя Удалить Файл %s%s%s, Этого Файла Нет В Этом Каталоге\n", __func__, RED, name, RESET );
		}
		if (!debug ) {
			printf( "%s: %s%s%s: Нет Файла Или Каталога\n", "delete_file", RED, name, RESET );
		}
		return 0;
	}
}

/***************************************************************************/
// Очистка Файловой Системы

int clean(char *name, char *size) {
	(void)*name;
	(void)*size;

	free(disk);
	
	disk = (char*)calloc ( 1, DISK_PARTITION );

	add_descriptor("descriptor");
	add_directory("root");
	
	strcpy(current.directory, "root");
	current.directory_index = 3;
	strcpy(current.parent, "" );
	current.parent_index = -1;
	
	disk_allocated = true;
	
 	return 0;

}