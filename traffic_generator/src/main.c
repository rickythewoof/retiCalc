#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>

int src_num, seq_num;

typedef struct Payload
{
    int value;
} Payload;

typedef struct Message
{
    int source;
    int sequence;
    int payload_lenght;
    Payload* payload;
} Message;

Message *prepare_message(Payload *payload);

//---------------
// Traffic Generator
//---------------

// Some sistem that triggers handler callback every x milliseconds


//---------------
// Broadcaster
//---------------

Message *prepare_message(Payload *payload)
{
    Message *msg = calloc(1, sizeof(Message));
    msg->source = src_num;
    msg->sequence = seq_num;
    msg->payload_lenght = sizeof(int);
    msg->payload = payload;

    return msg;
}

typedef void (*BroadcasterHandler)(void *broadcaster, Payload *payload);

typedef struct Broadcaster
{
    // .. Local data to manage broadcast
    // e.g. socket opened, network interfaces,
    // sequence of other nodes...
    int sock_fd;
    struct sockaddr_in *addr;
    socklen_t size_sockaddr;
    int seq_sent;

    BroadcasterHandler handler;
} Broadcaster;

void register_handler(Broadcaster *broadcaster, BroadcasterHandler handler)
{
    broadcaster->handler = handler;
}

void send_broadcast(Broadcaster *broadcaster, Payload *payload)
{

    int ret = sendto(broadcaster->sock_fd, payload, sizeof(Payload), 0, (struct sockaddr*) broadcaster->addr, broadcaster->size_sockaddr);
    if (ret == -1)
    {
        perror("Error sending payload!");
        exit(1);
    }
}

void process_broadcaster(Broadcaster *broadcaster)
{
    // In case of packet reception, notify the handler
    // Discard already seen packets

    // TODO pipe to traffic 
}

//---------------
// Traffic Analyzer
//---------------

typedef struct TrafficAnalyzer
{
    int node_id;
    time_t datetime;
    struct TrafficAnalyzer* next;

} TrafficAnalyzer;

void received_pkt(TrafficAnalyzer *analyzer, int source)
{
    

}

void dump(TrafficAnalyzer *analyzer)
{
    // Dump information about the thoughput of all packets received
}

//-------------------------
// Utility
// ------------------------

/**
 * Bind the given socket to all interfaces (one by one)
 * and invoke the handler with same parameter
 */
void bind_to_all_interfaces(int sock, void *context, void (*handler)(int, void *))
{
    struct ifaddrs *addrs, *tmp;
    getifaddrs(&addrs);
    tmp = addrs;
    while (tmp)
    {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_PACKET)
        {
            setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, tmp->ifa_name, sizeof(tmp->ifa_name));
            handler(sock, context);
        }
        tmp = tmp->ifa_next;
        
    }
    freeifaddrs(addrs);
}

/**
 * Sleep a given amount of milliseconds
 */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

int main()
{
    //Broadcaster on main!
    //TODO open pipe brdc->trfc
    //               gnrt->brdc


    // Autoflush stdout for docker
    setvbuf(stdout, NULL, _IONBF, 0);

    // Traffic generator
    pid_t generator_pid = fork();
    switch(generator_pid){
        case(-1):
            perror("Fork() failed");
            return -1;
        case(0):
            //TODO: 
            //  - Enable traffic generator
            break;
        default:
            break;
    }
    // Traffic analyzer
    pid_t traffic_pid = fork();
    switch(traffic_pid){
        case(-1):
            perror("Fork() failed");
            return -1;
        case(0):
            //TODO: 
            //  - Enable traffic analyzer
            //  - Create comm-pipe from broadcaster to traffica analyzer
            break;
        default:
            break;
    }
}