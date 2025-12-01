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
#define MAX_FILE_SIZE 20000
#define ACK_MESSAGE "ACK_OK"
#define SERVER_PORT 8000
#define FLAG_START 1
#define FLAG_IN_PROGRESS 0
#define FLAG_FIN 2

typedef struct __attribute__((packed)) {
    uint32_t payload_size;  // 4 B
    uint32_t seq_id;        // 4 B
    uint16_t status;        // 2 B
} udp_header_t;

/* hash DJB2 */
unsigned long compute_hash(const unsigned char *data, size_t len) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + data[i]; // hash * 33 + c
    }
    return hash;
}

int main(int argc, char *argv[]) {
    int sockfd;
    char buffer[MAX_BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char file_buf[MAX_FILE_SIZE];
    size_t file_len = 0;
    unsigned int expected_seq = 0;
    int started = 0;


    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Error socket()");
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

    printf("UDP Server listens on %d\n", SERVER_PORT);

    len = sizeof(cliaddr);

    while (1) {
        memset(&cliaddr, 0, sizeof(cliaddr));
        len = sizeof(cliaddr);

        int n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE,
                     0, (struct sockaddr *) &cliaddr, &len);

        if (n < 0) {
            if (errno == EINTR) continue;
            perror("Error recvfrom()");
            continue;
        }

        char *client_ip = inet_ntoa(cliaddr.sin_addr);

        printf("\nReceived %d bytes from %s:%d\n",
               n, client_ip, ntohs(cliaddr.sin_port));

        if (n < (int)sizeof(udp_header_t)) {
            fprintf(stderr, "Packet too short: %d bytes\n", n);
            continue;
        }

        udp_header_t hdr_net;
        memcpy(&hdr_net, buffer, sizeof(hdr_net));

        unsigned int payload_size = ntohl(hdr_net.payload_size);
        unsigned int seq_id = ntohl(hdr_net.seq_id);
        unsigned short status = ntohs(hdr_net.status);

        if ((int)(sizeof(udp_header_t) + payload_size) > n) {
            fprintf(stderr, "Malformed packet: header says %u payload, got %d bytes\n", payload_size, n);
            continue;
        }

        unsigned char *payload = (unsigned char *)buffer + sizeof(udp_header_t);

        unsigned char ack_buf[sizeof(ACK_MESSAGE) - 1 + 4];
        memcpy(ack_buf, ACK_MESSAGE, sizeof(ACK_MESSAGE) - 1);
        unsigned int ack_seq_net = htonl(seq_id);
        memcpy(ack_buf + (sizeof(ACK_MESSAGE) - 1), &ack_seq_net, 4);

        if (status == FLAG_START) {
            started = 1;
            expected_seq = seq_id;
            file_len = 0;
            printf("START received. seq=%u, payload=%u\n", seq_id, payload_size);
            if (payload_size > 0) {
                if (file_len + payload_size <= MAX_FILE_SIZE) {
                    memcpy(file_buf + file_len, payload, payload_size);
                    file_len += payload_size;
                } else {
                    fprintf(stderr, "File buffer overflow on START. Dropping payload.\n");
                }
            }
        } else if (status == FLAG_IN_PROGRESS) {
            if (!started) {
                fprintf(stderr, "Received IN_PROGRESS before START. Ignoring.\n");
            } else if (seq_id == expected_seq) {
                if (file_len + payload_size <= MAX_FILE_SIZE) {
                    memcpy(file_buf + file_len, payload, payload_size);
                    file_len += payload_size;
                    expected_seq++;
                } else {
                    fprintf(stderr, "File buffer overflow. Dropping payload seq=%u.\n", seq_id);
                }
            } else if (seq_id < expected_seq) {
                printf("Duplicate packet seq=%u, re-ACKing.\n", seq_id);
            } else {
                printf("Out-of-order packet seq=%u, expected=%u. Ignoring but ACKing.\n", seq_id, expected_seq);
            }
        } else if (status == FLAG_FIN) {
            if (!started) {
                fprintf(stderr, "Received FIN before START. Ignoring.\n");
            } else if (seq_id == expected_seq) {
                if (file_len + payload_size <= MAX_FILE_SIZE) {
                    memcpy(file_buf + file_len, payload, payload_size);
                    file_len += payload_size;
                } else {
                    fprintf(stderr, "File buffer overflow on FIN. Dropping payload.\n");
                }
                unsigned long hash = compute_hash(file_buf, file_len);
                printf("FIN received. Total bytes=%zu, server hash=%lu\n", file_len, hash);
                started = 0;
                expected_seq = 0;
                file_len = 0;
            } else if (seq_id < expected_seq) {
                printf("Duplicate FIN seq=%u, re-ACKing.\n", seq_id);
            } else {
                printf("Out-of-order FIN seq=%u, expected=%u. Ignoring but ACKing.\n", seq_id, expected_seq);
            }
        } else {
            fprintf(stderr, "Unknown status flag: %u\n", status);
        }

        if (sendto(sockfd, (const char *)ack_buf, sizeof(ack_buf),
                   0, (const struct sockaddr *) &cliaddr, len) < 0) {
            perror("Error sendto()");
        }
    }

    close(sockfd);
    return 0;
}