#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_BUFFER_SIZE 1000000
#define SERVER_PORT 8000

/* hash DJB2 */
unsigned long compute_hash(const unsigned char *data, size_t len) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + data[i]; // hash * 33 + c
    }
    return hash;
}

void reap_children(int sig) {
    (void)sig;
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0) {
    }
}

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    
    int sockfd;
    char buffer[MAX_BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = reap_children;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Error sigaction()");
        exit(EXIT_FAILURE);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error socket()");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error setsockopt()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Error bind()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, SOMAXCONN) < 0) {
        perror("Error listen()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("TCP server listens on %d\n", SERVER_PORT);

    while (1) {
        memset(&cliaddr, 0, sizeof(cliaddr));
        len = sizeof(cliaddr);

        int connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
        if (connfd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Error accept()");
            continue;
        }

        char *client_ip = inet_ntoa(cliaddr.sin_addr);
        printf("\nNew connection from %s:%d\n",
               client_ip, ntohs(cliaddr.sin_port));

        pid_t pid = fork();
        if (pid < 0) {
            perror("Error fork()");
            close(connfd);
            continue;
        }

        if (pid == 0) {
            /* CHILD */
            close(sockfd);

            memset(buffer, 0, MAX_BUFFER_SIZE);
            int n = recv(connfd, buffer, MAX_BUFFER_SIZE - 1, 0);
            if (n < 0) {
                perror("Error recv()");
                close(connfd);
                exit(EXIT_FAILURE);
            } else if (n == 0) {
                printf("Client %s:%d disconnected without data\n",
                       client_ip, ntohs(cliaddr.sin_port));
                close(connfd);
                exit(EXIT_SUCCESS);
            }

            printf("Received %d bytes from %s:%d\n",
                   n, client_ip, ntohs(cliaddr.sin_port));

            unsigned long hash = compute_hash((unsigned char *)buffer, (size_t)n);

            char response[128];
            snprintf(response, sizeof(response), "Hash: %lu", hash);

            if (send(connfd, response, strlen(response), 0) < 0) {
                perror("Error send()");
                close(connfd);
                exit(EXIT_FAILURE);
            }

            printf("Sent response '%s' to %s:%d\n",
                   response, client_ip, ntohs(cliaddr.sin_port));

            close(connfd);
            exit(EXIT_SUCCESS);
        } else {
            /* PARENT */
            close(connfd);
        }
    }

    close(sockfd);
    return 0;
}