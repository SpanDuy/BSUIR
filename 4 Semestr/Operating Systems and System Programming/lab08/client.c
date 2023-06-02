// Пример клиента

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096
#define MAX_NAME 255

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];
    int received_bytes;
    char *current_dir = calloc(MAX_NAME, sizeof(char));
    int change = 0;

    // Создание сокета
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Настройка адреса сервера
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Укажите IP-адрес сервера
    server_address.sin_port = htons(8080); // Укажите порт сервера

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Connection failed");
        return 1;
    }

    send(client_socket, "PING", strlen("PING"), 0);
    sleep(1);
    while ( (received_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0)) ) {
        buffer[received_bytes] = '\0';
        for (int i = 84, j = 0; buffer[i]; i++, j++) {
            current_dir[j] = buffer[i];
        }
        buffer[84] = '\0';
        printf("%s", buffer);
        break;
    }

    while (1) {
        do {
            printf("\n%s>", current_dir);
            fgets(buffer, BUFFER_SIZE, stdin);
        } while (strcmp(buffer, "\n") == 0);

        buffer[strcspn(buffer, "\n")] = '\0'; // Удаление символа новой строки

        if (strstr(buffer, "@")) {
            char *file_name = calloc(MAX_NAME, 1);
            int i = 0;
            int j = 1;
            while (buffer[j]) {
                file_name[i] = buffer[j];
                i++;
                j++;
            }
            file_name[i] = '\0';
            FILE *file = fopen(file_name, "r");
            if (file == NULL) {
                puts("ERROR : file dosn't exist");
                continue;
            }
            while (fgets(buffer, sizeof(buffer), file)) {
                printf("\n%s>%s\n", current_dir, buffer);

                if (strstr(buffer, "\n")) {
                    int i = 0;
                    while (buffer[i] != '\n') i++;
                    buffer[i] = '\0';
                }

                send(client_socket, buffer, strlen(buffer), 0);

                if (strstr(buffer, "CD")) {
                    change = 1;
                }

                received_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);

                buffer[received_bytes] = '\0';

                if (change) {
                    int i = 0;
                    int j = 0;
                    while (buffer[j]) {
                        current_dir[i] = buffer[j];
                        i++;
                        j++;
                    }
                    current_dir[i] = '\0';
                    change = 0;
                } else {
                    printf("%s", buffer);
                }

                sleep(1);
            }
            fclose(file);
            continue;
        } 

        // Отправка запроса на сервер
        send(client_socket, buffer, strlen(buffer), 0);

        if (strstr(buffer, "CD")) {
            change = 1;
        }

        received_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (received_bytes <= 0) {
            perror("Connection closed by server");
            break;
        }
        
        

        buffer[received_bytes] = '\0';

        if (change) {
            int i = 0;
            int j = 0;
            while (buffer[j]) {
                current_dir[i] = buffer[j];
                i++;
                j++;
            }
            current_dir[i] = '\0';
            change = 0;
        } else {
            printf("%s", buffer);
        }

        // Проверка условия завершения сеанса
        if (strcmp(buffer, "QUIT") == 0) {
            break;
        }
        
    }

    close(client_socket);
    free(current_dir);
    return 0;
}
