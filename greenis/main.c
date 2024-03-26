#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <asm-generic/socket.h>

#define PORT 7379
#define MAX_PROCS 3
#define MAX_BUF 256
#define DEBUG

#define ERROR_REQUEST 0
#define GET_SUCCEEDED 1
#define OK 2

void exit_with_error(const char * msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

typedef struct slot{
    time_t expiry;  //EXPIRATION DATE
    char* key;
    char* value;
    struct slot* next;
} slot;

typedef struct Request{
    enum Req_type {GET, SET, INFO} request_type;
    char* key; 
    char* value;
    int time_to_live;
} Request;

typedef struct Dictionary{
    slot* first;
} Dictionary;

int client_socket;

Request* parse(char* buf);
void set(Dictionary* dict, char* key, char* value);
slot* get(Dictionary* dict, const char* key);
slot* del(Dictionary* dict, slot* slot);
void send_tcp(int socket, int default_ok, slot* slot);
void handle_connection(int fd, int id);
void handle_request(Request* req);


void handle_connection(int fd, int id){
    int ret;
    client_socket = fd;
    Dictionary dict = {0};
    char buf[MAX_BUF] = {0};
    while(1){
        ret = recv(fd, &buf, MAX_BUF, 0);
        if(ret == -1) exit_with_error("Error receiving");
        Request *req = parse(buf);
        handle_request(req);
        free(req);
    }
    _exit(0);
}

//TODO: Finish parser

Request* parse(char* buf){
    Request* ret = calloc(1, sizeof(Request));
    ret->request_type = INFO;
    int index = 0;
    int size_of_next_elem = 0;
    char* token = strtok(buf, "\r\n");
    printf("[CLIENT] Received string: \n");
    while(token){
        printf("%s ", token);
        token = strtok(NULL, "\r\n");
    }
    printf("\n");
    return ret;
}

//TODO: Finish request handler

void handle_request(Request* req){
    if(req->request_type == INFO){
        send_tcp(client_socket, OK, NULL);
    } else{
        send_tcp(client_socket, ERROR_REQUEST, NULL);
    }
 }


void send_tcp(int socket, int request, slot* slot){
    char buf[MAX_BUF] = {0};
    if(request == GET_SUCCEEDED){
        snprintf(buf, MAX_BUF, "$%ld %s\r\n", strlen(slot->value), slot->value);
        int ret = send(socket, buf, strlen(buf)+1, 0);
        if(ret == -1) exit_with_error("error sending GET response");
    } if(request == OK){
        snprintf(buf, MAX_BUF, "+0K\r\n");
        int ret = send(socket, buf, strlen(buf)+1, 0);
        if (ret == -1) exit_with_error("Error sending OK");
    } if(ERROR_REQUEST){
        snprintf(buf, MAX_BUF, "$-1\r\n");
        int ret = send(socket, buf, strlen(buf)+1, 0);
        if(ret == -1) exit_with_error("error sending ERROR response");
    }
    printf("Sent back something\n");

}



// https://redis.io/docs/reference/protocol-spec/

int main(int argc, const char * argv[]) {

    int ret, pid, child_id = 0, opt = 1;

    // Open Socket and receive connections
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) exit_with_error("Error creating socket");


    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) exit_with_error("Error with setting opt");

    struct sockaddr_in server_addr = {0};
    socklen_t addr_len = sizeof(struct sockaddr_in);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    ret = bind(server_fd, (struct sockaddr*) &server_addr, addr_len);
    if(ret < 0) exit_with_error("Error Binding");

    ret = listen(server_fd, MAX_PROCS);
    printf("Listening\n");
    while(1){
        int client_fd = accept(server_fd, (struct sockaddr*) &server_addr, &addr_len);
        if(ret < 0) exit_with_error("Error accepting");

        pid = fork();
        switch(pid){
            case(-1):
                exit_with_error("Error connecting");
                break;
            case(0):
                printf("Accepted connection number %d\n", child_id);
                handle_connection(client_fd, child_id++);
                break;
        }
    }
    
    for(;child_id>0; child_id--){
        pid = wait(NULL);
    }

    return 0;
    
   // Keep a key, value store (you are free to use any data structure you want)

    // Create a process for each connection to serve set and get requested

}
