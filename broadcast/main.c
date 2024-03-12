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

#define BRDCAST_ADDR "255.255.255.255"
#define PORT 12345
#define MAXRAND 16

typedef struct{
    int seq_num;
    int data;
} Packet;

int main(int argc, const char** argv){
    if(argc < 2 || argc > 3){
        printf("USAGE: ./node.o <num> [leader]\n");
        exit(1);
    }
    
    int curr_seq = 0;
    int yes = 1;
    int status, node_num = atoi(argv[1]), addr_len;
    struct sockaddr_in brdc_addr;
    memset(&brdc_addr, 0, sizeof(struct sockaddr_in));

    //Socket creation
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == -1){
        perror("Error creating socket");
        exit(1);
    }

    brdc_addr.sin_family = AF_INET;
    brdc_addr.sin_addr.s_addr = inet_addr(BRDCAST_ADDR);
    brdc_addr.sin_port = htons(PORT);

    status = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&yes, sizeof(yes));
    if (status == -1)
    {
        perror("setsockopt error");
        return 0;
    }

    int bytes_read = 0, to_read = sizeof(Packet);

    if(argc == 3){
        fprintf(stdout, "You're the leader, yay!\n");
        //TODO: implement leader sending first packet
        int rand_num = rand() % MAXRAND, addr_len = sizeof(struct sockaddr_in);
        Packet packet = {.seq_num = curr_seq, .data = rand_num};
        int write_bytes = 0, to_send = sizeof(packet);
        while(to_send > 0){
            status = sendto(sock, &packet+write_bytes, to_send, 0, (struct sockaddr *) &brdc_addr, (socklen_t) addr_len);
            if(status == -1){
                if(errno == EINTR) continue;
                perror("Error sending");
                close(sock);
                exit(1);
            }
            to_send -=status;
            write_bytes += status;
        }
        fprintf(stdout,"Broadcast sent!\n");
    }

    do{
        fprintf(stdout,"wating for packet to forward\n");
        Packet packet = {};
        while(to_read > 0){
            status = recvfrom(sock, &packet-bytes_read, to_read, 0, NULL, NULL);
            if(status == -1){
                if(errno == EINTR) continue;
                perror("Error reading");
                close(sock);
                exit(1);
            } if(status == 0){
                perror("End of file!");
                exit(1);
            }
            bytes_read += status;
            bytes_read -= status;
        }
        fprintf(stdout, "[node #%d]: Received message %d\n", node_num, packet.data);
        if(packet.seq_num > curr_seq ){
            curr_seq = packet.seq_num;
            int write_bytes = 0, to_send = sizeof(packet);
            while(to_send > 0){
                status = sendto(sock, &packet+write_bytes, to_send, 0, (struct sockaddr *) &brdc_addr, (socklen_t) addr_len);
                if(status == -1){
                    if(errno == EINTR) continue;
                    perror("Error sending");
                    close(sock);
                    exit(1);
                }
                to_send -=status;
                write_bytes += status;
            }
        }else{
            fprintf(stdout,"Sequence number same or lower, ignoring...\n");
        }
    } while(0);
    
    close(sock);
    return 0;   
}