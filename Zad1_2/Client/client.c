#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>

#define SERVER_HOST "z39_server_app2"
#define SERVER_PORT 8000
#define CHUNK_SIZE 100
#define N_CHUNKS 100
#define FILENAME "random.bin"

// Flagi protokołu
#define START_TRANSMISSION 1
#define TRANSMISSION_IN_PROGRESS 0
#define END_TRANSMISSION 2
#define ACK_FLAG 3

#pragma pack(push, 1)
typedef struct {
    uint32_t payload_size;
    uint32_t seq_id;
    uint16_t status_flag;
} udp_header_t;
#pragma pack(pop)

#define HEADER_SIZE sizeof(udp_header_t)

unsigned long djb2_hash(unsigned char *data, size_t len) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + data[i];
    }
    return hash;
}

void prepare_header(udp_header_t *header, uint32_t payload_size, uint32_t seq_id, uint16_t status_flag) {
    header->payload_size = htonl(payload_size);
    header->seq_id = htonl(seq_id);
    header->status_flag = htons(status_flag);
}

int main() {
    FILE *fp = fopen(FILENAME, "rb");
    if (!fp) {
        perror("Error opening file");
        exit(1);
    }

    unsigned char *file_data = malloc(CHUNK_SIZE * N_CHUNKS);
    if (fread(file_data, 1, CHUNK_SIZE * N_CHUNKS, fp) != CHUNK_SIZE * N_CHUNKS) {
        fprintf(stderr, "Error reading file or file too short\n");
    }
    fclose(fp);

    unsigned long hash = djb2_hash(file_data, CHUNK_SIZE * N_CHUNKS);
    printf("CLIENT HASH: %lu\n", hash);

    int sockfd;
    struct sockaddr_in servaddr;
    struct hostent *server;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Error creating socket");
        exit(1);
    }

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error setting timeout");
        exit(1);
    }

    server = gethostbyname(SERVER_HOST);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host as %s\n", SERVER_HOST);
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    memcpy(&servaddr.sin_addr.s_addr, server->h_addr, server->h_length);
    servaddr.sin_port = htons(SERVER_PORT);

    socklen_t len = sizeof(servaddr);
    udp_header_t send_hdr, recv_hdr;
    char buffer[1024];

    while (1) {
        prepare_header(&send_hdr, 0, 0, START_TRANSMISSION);
        sendto(sockfd, &send_hdr, HEADER_SIZE, 0, (const struct sockaddr *)&servaddr, len);

        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (n >= HEADER_SIZE) {
            memcpy(&recv_hdr, buffer, HEADER_SIZE);
            if (ntohs(recv_hdr.status_flag) == ACK_FLAG) {
                printf("INFO: Established connection with %s:%d\n", SERVER_HOST, SERVER_PORT);
                break;
            }
        } else {
            printf("Timeout or invalid packet (Handshake)\n");
        }
    }

    for (int i = 0; i < N_CHUNKS; i++) {
        char packet[HEADER_SIZE + CHUNK_SIZE];
        prepare_header((udp_header_t*)packet, CHUNK_SIZE, i, TRANSMISSION_IN_PROGRESS);
        memcpy(packet + HEADER_SIZE, file_data + (i * CHUNK_SIZE), CHUNK_SIZE);

        while (1) {
            sendto(sockfd, packet, HEADER_SIZE + CHUNK_SIZE, 0, (const struct sockaddr *)&servaddr, len);

            int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
            if (n >= HEADER_SIZE) {
                memcpy(&recv_hdr, buffer, HEADER_SIZE);
                uint32_t ack_seq = ntohl(recv_hdr.seq_id);
                uint16_t ack_flag = ntohs(recv_hdr.status_flag);

                if (ack_flag == ACK_FLAG && ack_seq == i) {
                    printf("INFO: Got ACK form %dth chunk\n", i);
                    break;
                }
            }
            printf("Timeout or invalid ACK for chunk %d\n", i);
        }
    }

    while (1) {
        prepare_header(&send_hdr, 0, 0, END_TRANSMISSION);
        sendto(sockfd, &send_hdr, HEADER_SIZE, 0, (const struct sockaddr *)&servaddr, len);

        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (n >= HEADER_SIZE) {
            memcpy(&recv_hdr, buffer, HEADER_SIZE);
            if (ntohs(recv_hdr.status_flag) == ACK_FLAG) {
                printf("INFO: Finalized connection with %s:%d\n", SERVER_HOST, SERVER_PORT);
                break;
            }
        } else {
            printf("Timeout or invalid packet (Finalize)\n");
        }
    }

    free(file_data);
    close(sockfd);
    return 0;
}