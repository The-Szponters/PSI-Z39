#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define MAX_BUFFER_SIZE 1000000
#define ACK_MESSAGE "ACK_OK"
#define SERVER_PORT 8000

int main(int argc, char *argv[]) {
    int sockfd;
    char buffer[MAX_BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;


    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Błąd socket()");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);


    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Błąd bind()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP Server C nasłuchuje na porcie %d\n", SERVER_PORT);

    len = sizeof(cliaddr);

    while (1) {
        memset(&cliaddr, 0, sizeof(cliaddr));
        len = sizeof(cliaddr);

        int n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE,
                     0, (struct sockaddr *) &cliaddr, &len);

        if (n < 0) {
            if (errno == EINTR) continue;
            perror("Błąd recvfrom()");
            continue;
        }

        char *client_ip = inet_ntoa(cliaddr.sin_addr);

        printf("\nOdebrano %d bajtów od %s:%d\n",
               n, client_ip, ntohs(cliaddr.sin_port));

        if (sendto(sockfd, (const char *)ACK_MESSAGE, strlen(ACK_MESSAGE),
                   0, (const struct sockaddr *) &cliaddr, len) < 0) {
            perror("Błąd sendto()");
        }
    }

    close(sockfd);
    return 0;
}