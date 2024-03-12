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
#define PORT 1234
#define MAXRAND 16

typedef struct{
    int seq_num;
    int data;
} Packet;

int main(int argc, const char** argv){
    if(argc < 2 || (argc == 3 && memcmp(argv[2], "leader", sizeof(argv[2]))  != 0)){
        printf("USAGE: ./client.o <num> [leader]");
    }
    
    int curr_seq = 0;
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

    int bytes_read = 0, to_read = sizeof(Packet);

    if(argc == 3 && memcmp(argv[2], "leader", sizeof(argv[2])) == 0){
        //TODO: implement leader sending first packet
        int rand_num = rand() % MAXRAND, addr_len = sizeof(struct sockaddr_in);
        Packet packet = {.seq_num = curr_seq, .data = rand_num};
        int write_bytes = 0, to_send = sizeof(packet);
        while(to_send > 0){
            status = sendto(sock, &packet+write_bytes, to_send, 0, (struct sockaddr *) &brdc_addr, (socklen_t*) &addr_len);
            if(status == -1){
                if(errno == EINTR) continue;
                perror("Error sending");
                close(sock);
                exit(1);
            }
        }
    }

    do{
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
                status = sendto(sock, &packet+write_bytes, to_send, 0, (struct sockaddr *) &brdc_addr, (socklen_t*) &addr_len);
                if(status == -1){
                    if(errno == EINTR) continue;
                    perror("Error sending");
                    close(sock);
                    exit(1);
                }
            }
        }else{
            fprintf(stdout,"Sequence number same or lower, ignoring...\n");
        }
    } while(0);
    
    close(sock);
    return 0;   
}