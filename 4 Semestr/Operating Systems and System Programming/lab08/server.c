// Пример сервера
#define _GNU_SOURSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <time.h>
#include <limits.h>

#define MAX_CLIENTS 4
#define BUFFER_SIZE 4096
#define MAX_NAME 255

ssize_t readlink(const char *restrict link_pathname, char *restrict buffer, size_t buffer_size);

// Структура для передачи данных в поток обработки клиента
typedef struct {
    int socket;
} client_data;

int num_clients = 0;
int is_end = 1;
char *root;

char *foo(char *first_str, char *second_str) {
	char *result = malloc(MAX_NAME);
	int i = 0;
	int j = 0;
	int k = 0;
	
	while (first_str[j]) {
		result[i] = first_str[j];
		i++;
		j++;
	}
	
	while (second_str[k]) {
		result[i] = second_str[k];
		i++;
		k++;
	}
	
	result[i] = '\0';
	
	return result;
}

char *foo_ECHO(char buffer[BUFFER_SIZE]) {
    char *foo_buffer = malloc(BUFFER_SIZE * sizeof(char));

    int i = 0;
    int j = sizeof("ECHO") - 1;
    while (buffer[i]) {
        foo_buffer[i++] = buffer[j++];
    } 
    foo_buffer[i] = '\0';

    return foo_buffer;
}

char *foo_INFO() {
    char response[] = "*************** Приветствую на Моем Сервере ***************\n";
    char *info = malloc(MAX_NAME);
    
    memcpy(info, response, sizeof(response));
    //info[85] = '\0';

    return info;
}

char* foo_LIST(char* current_dir) {
    int element_count = 0;
	DIR* directory;
    struct dirent* de;
	char *dirName = (char*)malloc(sizeof(char)*MAX_NAME);
    char *file_names = (char*)malloc(sizeof(char) * MAX_NAME);

    chdir(current_dir);
    //getcwd(current_dir, MAX_NAME);

	directory = opendir(current_dir); //открытие каталога
	if (!directory){
		return file_names;
	}

    while ((de = readdir(directory)) != NULL) {

        if (!strcmp(".", de->d_name) || !strcmp("..", de->d_name)){
            continue;
		}

        char name[MAX_NAME];
        element_count++;

        memcpy(name, (char*)de->d_name, MAX_NAME);  

        if (de->d_type == 4) {
            strcat(name, "/");
        } else if (de->d_type == 10) {
            char link_path[MAX_NAME];
            struct stat sb;

            ssize_t len = readlink( foo("./", name), link_path, sizeof(link_path)-1);
            link_path[len] = '\0';

            //printf("link_path = %s\n", link_path);

            lstat(link_path, &sb);
            if (S_ISLNK(sb.st_mode)) {
                strcat(name, "-->>");
            } else {
                strcat(name, "-->");
            }

            strcat(name, link_path);
        } else if (de->d_type == 8) {

        }

        strcat(name, "\n");

        file_names = (char*)realloc(file_names, sizeof(char) * element_count * MAX_NAME);

        strcat(file_names, name);

    }

    return file_names; // функция возвращает массив имен элементов
}

char *foo_CD(char *current_dir, char *buffer) {

    char *dir_name = calloc(MAX_NAME, sizeof(char));
    int i = 0;
    int j = sizeof("CD");
    while (buffer[i]) {
        dir_name[i++] = buffer[j++];
    } 
    dir_name[i] = '\0';

    char *buff = calloc(MAX_NAME, 1);

    chdir(current_dir);
    chdir(dir_name);
    getcwd(buff, MAX_NAME);

    if (strlen(buff) < strlen(root)) {
        puts("Попытка выйти за пределы root");
        chdir(current_dir);
        getcwd(buff, MAX_NAME);
    }

    return buff;
}

// Обработчик клиента в отдельном потоке
void* handle_client(void* arg) {
    client_data* data = (client_data*)arg;
    int client_socket = data->socket;

    char *current_dir = calloc(MAX_NAME, sizeof(char));
    char buffer[BUFFER_SIZE];
    int received_bytes;

    getcwd(current_dir, MAX_NAME);

    while ( (received_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0)) ) {
        buffer[received_bytes] = '\0';
        if ( strstr("PING", buffer) ) {
            break;
        }
    }
    send(client_socket, foo_INFO(), strlen(foo_INFO()), 0);
    send(client_socket, current_dir, strlen(current_dir), 0);

    while ((received_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[received_bytes] = '\0';

        // Обработка запроса
        if (strstr(buffer, "ECHO")) {
            // Эхо-запрос
            send(client_socket, foo_ECHO(buffer), strlen(buffer), 0);
        } else if (strstr(buffer, "QUIT")) {
            // Запрос на завершение сеанса
            num_clients--;
            is_end = 0;
            break;
        } else if (strstr(buffer, "INFO")) {
            // Запрос на получение информации о сервере
            send(client_socket, foo_INFO(), strlen(foo_INFO()), 0);
        } else if (strstr(buffer, "CD")) {
            // Запрос на изменение текущего каталога
            current_dir = foo_CD(current_dir, buffer);

            strcpy(buffer, current_dir);

            send(client_socket, buffer, strlen(buffer), 0);
        } else if (strstr(buffer, "LIST")) {
            // Запрос на получение списка файловых объектов
            send(client_socket, foo_LIST(current_dir), strlen(foo_LIST(current_dir)), 0);
        } else {
            // Неизвестный запрос
            char response[] = "Unknown request";
            send(client_socket, response, strlen(response), 0);
        }
    }

    close(client_socket);
    free(data);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    pthread_t threads[MAX_CLIENTS];

    root = calloc(MAX_NAME, 1); 
    if (argv[1]) {
        strcpy(root, argv[1]);
        chdir(root);
        getcwd(root, MAX_NAME);
    } else {
        strcpy(root, "/");
    }

    // Создание сокета
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Настройка адреса сервера
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080); // Используйте нужный порт

    // Привязка сокета к адресу
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Binding failed");
        return 1;
    }

    // Прослушивание входящих соединений
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Listening failed");
        return 1;
    }

    printf("Server started. Waiting for incoming connections...\n");

    while (is_end || num_clients) {

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        struct timeval pause;
        pause.tv_sec = 3;

        fd_set tmp_fds = read_fds;

        int res = select(server_socket + 1, &read_fds, NULL, NULL, &pause);
        if (res == -1) {
            perror("select");
            exit(1);
        } 
        else if (res == 0) {
            sleep(3);
            continue;
        }

        if (!FD_ISSET(server_socket, &tmp_fds)) {
            continue;
        }

        // Принятие входящего соединения
        socklen_t client_address_len = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Accepting failed");
            continue;
        }

        // Создание нового потока для обработки клиента
        if (num_clients < MAX_CLIENTS) {
            client_data* data = (client_data*)malloc(sizeof(client_data));
            data->socket = client_socket;

            if (pthread_create(&threads[num_clients], NULL, handle_client, (void*)data) != 0) {
                perror("Thread creation failed");
                close(client_socket);
                free(data);
            } else {
                num_clients++;
                printf("Client connected. Total clients: %d\n", num_clients);
            }
        } else {
            printf("Max clients limit reached. Rejecting new connection.\n");
            close(client_socket);
        }

    }

    close(server_socket);
    return 0;
}
