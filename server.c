#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdarg.h>

#define CONF "config"

int state = 0;
FILE* log_file;

void write_log(char* format, ...) {
    va_list va;
    va_start(va, format);
    int log_len = vsnprintf(NULL, 0, format, va);
    va_end(va);
    char* log_line = malloc(log_len + 1);
    va_start(va, format);
    vsnprintf(log_line, log_len + 1, format, va);
    va_end(va);
    fwrite(log_line, 1, log_len, log_file);
    fflush(log_file);
    free(log_line);
}

void open_log() {
    log_file = fopen("/tmp/server.log", "w");
    if (log_file == NULL) {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }
}

char* get_sock_name() {
    FILE* conf_f = fopen(CONF, "r");
    char* sock_name = malloc(100);
    fgets(sock_name, 100, conf_f);
    
    if (conf_f == NULL) {
        perror("Error opening config file");
        exit(EXIT_FAILURE);
    }
    
    if (sock_name == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    
    int path_len = snprintf(NULL, 0, "/tmp/%s", sock_name);
    char* sock_path = malloc(path_len+1);
    snprintf(sock_path, path_len+1, "/tmp/%s", sock_name);
    free(sock_name);
    return sock_path;
}

int creat_socket() {
    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Can't create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;

    char* sock_name = get_sock_name();
    strcpy(addr.sun_path, sock_name);
    free(sock_name);

    int option = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1) {
        perror("Can't set socket options");
        exit(EXIT_FAILURE);
    }

    if (bind(server_socket, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        perror("Can't bind socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 150) == -1) {
        perror("Can't listen on socket");
        exit(EXIT_FAILURE);
    }

    return server_socket;
}

int serv(int server_socket) {
    write_log("server started\n");
    fd_set readfds;
    int client_sockets[150];
    for (int i = 0; i < 150; i++) {
        client_sockets[i] = 0;
    }
    
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        int max_fd = server_socket;
        for (int i = 0; i < 150; i++) {
            int sd = client_sockets[i];
            if (client_sockets[i] > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_fd) {
                max_fd = sd;
            }
        }
        int select_res = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (select_res < 0) {
            printf("error during select\n");
        }
        if (FD_ISSET(server_socket, &readfds)) {
            int client_sock = accept(server_socket, NULL, NULL);
            for (int i = 0; i < 150; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_sock;
                    long sbrk_val = (long) sbrk(0);
                    write_log("Accept client socket %d. Current sbrk() = %d\n", client_sockets[i], sbrk_val);
                    break;
                }
            }
        }
        for (int i = 0; i < 150; i++) {
            if (FD_ISSET(client_sockets[i], &readfds)) {
                char read_buf[11] = {0};
                int read = recv(client_sockets[i], read_buf, 11, 0);
                if (read <= 0) {
                    close(client_sockets[i]);
                    client_sockets[i] = 0;
                    continue;
                }

                int recv_num = atoi(read_buf);
                state += recv_num;
                write_log("recv number: %d; send state: %d\n", recv_num, state);
                char write_buf[11] = {0};
                sprintf(write_buf, "%d", state);
                send(client_sockets[i], write_buf, strlen(write_buf), 0);
            }
        }
    }
}

int main() {
    open_log();
    int server_socket = creat_socket();
    serv(server_socket);
    close(server_socket);

    return 0;
}
