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
#define SERVER_PORT 8000

#define FLAG_START 1
#define FLAG_IN_PROGRESS 0
#define FLAG_FIN 2
#define FLAG_ACK 3

typedef struct __attribute__((packed)) {
    uint32_t payload_size;
    uint32_t seq_id;
    uint16_t status;
} udp_header_t;

unsigned long compute_hash(const unsigned char *data, size_t len) {
    unsigned long hash = 5381;
    size_t i;
    for (i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + data[i];
    }
    return hash;
}

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

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

        if (n < (int)sizeof(udp_header_t)) {
            continue;
        }

        udp_header_t hdr_net;
        memcpy(&hdr_net, buffer, sizeof(hdr_net));

        unsigned int payload_size = ntohl(hdr_net.payload_size);
        unsigned int seq_id = ntohl(hdr_net.seq_id);
        unsigned short status = ntohs(hdr_net.status);

        if ((int)(sizeof(udp_header_t) + payload_size) > n) {
            continue;
        }

        unsigned char *payload = (unsigned char *)buffer + sizeof(udp_header_t);
        int send_ack = 0;

        if (status == FLAG_START) {
            started = 1;
            expected_seq = 0; 
            file_len = 0;
            if (payload_size > 0 && file_len + payload_size <= MAX_FILE_SIZE) {
                memcpy(file_buf + file_len, payload, payload_size);
                file_len += payload_size;
            }
            send_ack = 1;
        } 
        else if (status == FLAG_IN_PROGRESS) {
            if (!started) {
                send_ack = 0;
            } else if (seq_id == expected_seq) {
                if (file_len + payload_size <= MAX_FILE_SIZE) {
                    memcpy(file_buf + file_len, payload, payload_size);
                    file_len += payload_size;
                    expected_seq++;
                    send_ack = 1;
                } else {
                    send_ack = 0;
                }
            } else if (seq_id < expected_seq) {
                send_ack = 1;
            } else {
                send_ack = 0;
            }
        } 
        else if (status == FLAG_FIN) {
            if (!started) {
                send_ack = 0;
            } else if (seq_id == expected_seq) {
                if (file_len + payload_size <= MAX_FILE_SIZE) {
                    memcpy(file_buf + file_len, payload, payload_size);
                    file_len += payload_size;
                }
                unsigned long hash = compute_hash((unsigned char*)file_buf, file_len);
                
                printf("SERVER HASH: %lu\n", hash);
                
                started = 0;
                expected_seq = 0;
                file_len = 0;
                send_ack = 1;
            } else if (seq_id < expected_seq) {
                send_ack = 1;
            } else {
                send_ack = 0;
            }
        }
        
        if (send_ack) {
            udp_header_t ack_pkt;
            ack_pkt.payload_size = 0;
            ack_pkt.seq_id = htonl(seq_id);
            ack_pkt.status = htons(FLAG_ACK);

            if (sendto(sockfd, (const char *)&ack_pkt, sizeof(ack_pkt),
                       0, (const struct sockaddr *) &cliaddr, len) < 0) {
                perror("Error sendto()");
            }
        }
    }

    close(sockfd);
    return 0;
}