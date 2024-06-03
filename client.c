#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#define CONF "config"

char* get_sock_name() {
    FILE* conf_f = fopen(CONF, "r");
    if (conf_f == NULL) {
        perror("Error opening config file");
        exit(EXIT_FAILURE);
    }
    
    char* sock_name = malloc(100);
    if (sock_name == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    if (fgets(sock_name, 100, conf_f) == NULL) {
        perror("Error reading from config file");
        exit(EXIT_FAILURE);
    }

    fclose(conf_f);
    

    // Constructing socket path
    int path_len = snprintf(NULL, 0, "/tmp/%s", sock_name);
    char* sock_path = malloc(path_len + 1);
    if (sock_path == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    
    snprintf(sock_path, path_len + 1, "/tmp/%s", sock_name);

    free(sock_name);

    return sock_path;
}

int get_rand(int lower, int upper) {
    return rand() % (upper - lower + 1) + lower;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <client_id> <nums_to_send> [delay]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int client_id = atoi(argv[1]);
    int nums_count = atoi(argv[2]);
    float delay = 0;

    // Parsing optional delay argument
    if (argc == 4) {
        delay = atof(argv[3]);
    }

    char* sock_name = get_sock_name();
    srand(time(NULL));

    float sleep_time = delay * 1000000;

    printf("Client: %d with delay: %.2f seconds\n", client_id, sleep_time / 1000000);

    // Creating socket
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("Can't create socket");
        exit(EXIT_FAILURE);
    }

    // Constructing server address
    struct sockaddr_un sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sun_family = AF_UNIX;
    strcpy(sockaddr.sun_path, sock_name);

    // Connecting to server
    if (connect(sock_fd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) == -1) {
        perror("Can't connect to server");
        exit(EXIT_FAILURE);
    }

    time_t t0 = time(0);

    // Sending numbers to server
    for (int i = 0; i < nums_count; i++) {
        char send_buf[11];
        char recv_buf[11];
        
        if (scanf("%s", send_buf) != 1) {
            perror("Error reading input");
            exit(EXIT_FAILURE);
        }

        usleep(sleep_time);
        
        if (write(sock_fd, send_buf, strlen(send_buf)) == -1) {
            perror("Error sending data to server");
            exit(EXIT_FAILURE);
        }

        if (read(sock_fd, recv_buf, 11) == -1) {
            perror("Error receiving data from server");
            exit(EXIT_FAILURE);
        }
    }

    time_t t1 = time(0);
    double time_in_seconds = difftime(t1, t0);

    printf("Client time: %.2f seconds\n", time_in_seconds);
    
    close(sock_fd);
    free(sock_name);

    return 0;
}
