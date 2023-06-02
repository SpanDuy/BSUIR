#define DEFAULT_SOURSE
#include <stdio.h>
#include <string.h>  
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

#define NAME_MAX 255

int getopt(int argc, char *const argv[], const char* optstring);
struct dirent *readdir (DIR *dirp);

//функция возвращающая длинну массива символов
int length(const char* str){
	int i = 0;
	while(str[i]) i++;
	return i;
}

//функция сравнения двух массивов символов
int sortCondition(const void *s1, const void *s2) {
	if(strcmp((char*)s1, (char*)s2) > 0){
		return 1;
	}
	else{
		return -1;
	}
}
 //сортировка массива строк
char** sort(char** files, int n){
	for (int i = 0; i < n - 1; i++){
		for (int j = i + 1; j < n; j++){
			if(sortCondition(files[i], files[j]) > 0){
				char* temp = (char*)malloc(sizeof(char)*NAME_MAX);
				strcpy(temp, files[i]);
				strcpy(files[i], files[j]);
				strcpy(files[j], temp);
				free(temp);
			}
		}
	}
	return files;
}

//сложение двух строк
char* sumStrings(char* firstStr, char* secondStr){
	int i = 0;
	int j = 0;
	char* str = (char*)malloc(sizeof(char)*NAME_MAX);
	while(firstStr[i]){
		str[i] = firstStr[i];
		i++;
	}
	while(secondStr[j]){
		str[i] = secondStr[j];
		j++;
		i++;
	}
	str[i] = '\0';
	return str;
}

//обход по каталогу
char** findRec(const char* dirname, int flags[], int *elemsCount, char** file_names){

	DIR* directory;
    struct dirent* de;
	char* dirName = (char*)malloc(sizeof(char)*NAME_MAX);
	memcpy(dirName, dirname, NAME_MAX);

	directory = opendir(dirname); //открытие каталога
	if (!directory){
		return file_names;
	}
	while ((de = readdir(directory)) != NULL) {

        if (!strcmp(".", de->d_name) || !strcmp("..", de->d_name)){
            continue;
		}

		int pushCondition = (	//выбор флага
			(flags[0] && flags[1] && flags[2] && flags[3] == 0)  || 
			(flags[0] && de->d_type == 10) || 
			(flags[1] && de->d_type == 4) || 
			(flags[2] && de->d_type == 8)
		);

        if(pushCondition) {	//добавление элемента 
            file_names = (char**)realloc(file_names, sizeof(char*)*(*elemsCount+1)); 
            file_names[*elemsCount] = (char*)malloc(sizeof(char)*NAME_MAX);
			char* fullName = (char*)calloc(NAME_MAX, sizeof(char));
			fullName = sumStrings((char*)dirname, (char*)de->d_name);
            memcpy(file_names[*elemsCount], fullName, NAME_MAX); // создаем элемент массива с именем найденного элемента
			free(fullName);
            *elemsCount += 1;
        }  

		if(de->d_type == 4){
			file_names = findRec(sumStrings(sumStrings(dirName, de->d_name), "/"), flags, elemsCount, file_names);
		}
    }

	closedir(directory); // закрытие директории
	free(dirName);
    return file_names; // функция возвращает массив имен элементов
}

//парсинг командной строки
int* getFlags(int argc, char** argv){
	int opt;
	int* flags = (int*)calloc(4, sizeof(int));
    while ((opt = getopt(argc, argv, "ldfs")) != -1) {
		switch (opt) {
			case 'l':
				flags[0] = 1; break;
			case 'd':
				flags[1] = 1; break;
			case 'f':
				flags[2] = 1; break;
			case 's':
				flags[3] = 1; break;
		
			default:
				break;
		}
    }
	if (flags[0] == 0 && flags[1] == 0 && flags[2] == 0){
		flags[0] = 1;
		flags[1] = 1;
		flags[2] = 1;
	}
	printf("\n\nflags - (l = %d | d = %d | f = %d | s = %d)\n\n", flags[0], flags[1], flags[2], flags[3]);
	return flags;
}

int main(int argc, char** argv) {

	int *flags = getFlags(argc, argv);
    int elemsCount = 0;
	const char* dirname;

	if (argc == 1 || argv[argc - 1][0] == '-'){
		dirname = "./";
	}
	else{
		dirname = argv[argc - 1];
	}
	if (dirname[length(dirname) - 1] != '/'){
		dirname = sumStrings((char*)dirname, "/");
	}
	char** files = (char**) malloc(1*sizeof(char*));

	files = findRec( dirname, flags, &elemsCount, files);

	if(!elemsCount) {
		printf("Elements not found...\n"); // вывод в консоль имя элемента
	}

    if(flags[3]) {
		printf("Sorted: \n\n"); // вывод в консоль имя элемента
		sort(files, elemsCount);
	}

    for(int i = 0; i < elemsCount; i++) {
        printf("  -  %s\n", files[i]); // вывод в консоль имя элемента
        free(files[i]); // освобождаем память, выделенную под элемент массива
    }

    free(files); // освобождаем память, выделенную под массив
    return 0;
}