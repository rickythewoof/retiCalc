#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 12345
#define MAXRAND 16

typedef struct
{
    int seq_num;
    int data;
} Packet;

int main(int argc, const char **argv)
{
    if (argc < 2 || argc > 3)
    {
        printf("USAGE: ./node.o <num> [leader]\n");
        exit(1);
    }

    int curr_seq = 0;
    int yes = 1;
    int status, node_num = atoi(argv[1]), addr_len = sizeof(struct sockaddr_in);

    struct sockaddr_in brdc_addr;
    memset(&brdc_addr, 0, sizeof(struct sockaddr_in));

    brdc_addr.sin_family = AF_INET;
    brdc_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    brdc_addr.sin_port = htons(PORT);

    int is_leader = (strcmp(argv[2], "leader") == 0);

    if (is_leader)
    {
        fprintf(stdout, "LEADER\n");
        fflush(stdout);
        // Socket creation
        int srv_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (srv_sock == -1)
        {
            perror("Error creating socket");
            exit(1);
        }

        status = setsockopt(srv_sock, SOL_SOCKET, SO_BROADCAST, (char *)&yes, sizeof(yes));
        if (status == -1)
        {
            perror("setsockopt error");
            return 0;
        }
        // TODO make server and bind
        struct sockaddr_in srv_addr;
        memset(&srv_addr, 0, sizeof(struct sockaddr_in));

        srv_addr.sin_family = AF_INET;
        srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        srv_addr.sin_port = htons(PORT);

        status = bind(srv_sock, (struct sockaddr *)&srv_addr, (socklen_t)addr_len);

        sleep(10);

        int rand_num = rand() % MAXRAND, addr_len = sizeof(struct sockaddr_in);
        Packet packet = {.seq_num = curr_seq, .data = rand_num};
        int write_bytes = 0, to_send = sizeof(packet);
        while (to_send > 0)
        {
            status = sendto(srv_sock, &packet + write_bytes, to_send, 0, (struct sockaddr *)&brdc_addr, (socklen_t)addr_len);
            if (status == -1)
            {
                if (errno == EINTR)
                    continue;
                perror("Error sending");
                close(srv_sock);
                exit(1);
            }
            to_send -= status;
            write_bytes += status;
        }
        fprintf(stdout, "Broadcast sent!\n");
    }
    else
    {
        fprintf(stdout, "CLIENT!\n");
        // Socket creation
        int cln_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (cln_sock == -1)
        {
            perror("Error creating socket");
            exit(1);
        }

        status = setsockopt(cln_sock, SOL_SOCKET, SO_BROADCAST, (char *)&yes, sizeof(yes));
        if (status == -1)
        {
            perror("setsockopt error");
            return 0;
        }
        fd_set readfd;

        FD_ZERO(&readfd);
        FD_SET(cln_sock, &readfd);
        status = select(cln_sock + 1, &readfd, NULL, NULL, 0);
        if (status > 0)
        {
            if (FD_ISSET(cln_sock, &readfd))
            {
                int bytes_read = 0, to_read = sizeof(Packet);
                Packet packet = {};
                while (to_read > 0)
                {
                    status = recvfrom(cln_sock, (&packet) + bytes_read, to_read, 0, NULL, NULL);
                    if (status == -1)
                    {
                        if (errno == EINTR)
                            continue;
                        perror("Error receiving");
                        close(cln_sock);
                        exit(1);
                    }
                    if (status == 0)
                    {
                        perror("EOF");
                        close(cln_sock);
                        exit(1);
                    }
                    bytes_read += status;
                    to_read -= status;
                }
                fprintf(stdout, "[Node #%d] Received message %d", node_num, packet.data);
                if (packet.seq_num > curr_seq)
                {
                    int bytes_written = 0, to_write = sizeof(Packet);
                    while (to_write > 0)
                    {
                        status = sendto(cln_sock, (&packet) + bytes_written, to_write, 0, (struct sockaddr *)&brdc_addr, (socklen_t)addr_len);
                        if (status == -1)
                        {
                            if (errno == EINTR)
                                continue;
                            perror("Error sending");
                            close(cln_sock);
                            exit(1);
                        }
                    }
                    fprintf(stdout, "Sent back to broadcast!\n");
                }
                else
                {
                    fprintf(stdout, "Discarting...\n");
                }
            }
        }
    }
    return 0;
}