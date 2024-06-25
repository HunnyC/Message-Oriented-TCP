#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <bits/pthreadtypes.h>
#include <signal.h>
#ifndef MSOCKET_H
#define MSOCKET_H
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <errno.h>
#include <time.h>
#define SHM_KEY_PATH "/tmp"
#define SHM_KEY_ID 'S'
#define MTP_KEY_PATH "/tmp"
#define MTP_KEY_ID 'M'
#define SEM_KEY1_ID 'a'
#define SEM_KEY2_ID 'b'
#define SEM_KEY3_ID 'c'
#define SEM_KEY4_ID 'd'

#include <semaphore.h>

#define T 5

#define SOCK_MTP 5
#define MAX_MESSAGE_SIZE 1030
#define MAX_MTP_SEND_BUFFER_SIZE 10 * MAX_MESSAGE_SIZE
#define MAX_MTP_RECEIVE_BUFFER_SIZE 5 * MAX_MESSAGE_SIZE
#define MAX_MTP_SOCKETS 25
#define p 0.05

// typedef struct
// {
//     // Define your MTP header structure here
// } MTPHeader;

typedef struct
{
    int sock_id;
    char IP[256];
    int port;
    int errorno;
} SOCK_INFO;

typedef struct
{

    bool free;
    pid_t pid;
    int UDPsocID;
    char des_IP[20];
    bool nospace;
    bool sendNospace;
    int des_port;
    char sbuf[10][MAX_MESSAGE_SIZE];
    char rbuf[5][MAX_MESSAGE_SIZE];
    char source_IP[20];
    int current_send; //window size 
    int current_recv;
    int source_port;
    int l_send, f_send; // last first
    char ack_num[3];
    int timeout;
    int seq_num;
    int l_recv, f_recv;
    int send_ack_flag; //Not useful
    // Window swnd;
    //int swnd[5]; // max window size
    int left_swnd, right_swnd;
    int curr_size_swnd;
    //int rwnd[5];
    int left_rwnd, right_rwnd;
    int curr_size_rwnd;
    int left_seq; //seq num for recvwindow
   // int last_seq_num;
   
    // Window rwnd;
    // bool isClosed;

} MTPSocket;

int m_socket(int domain, int type, int protocol);
int m_bind(int sockfd, char source_ip[], int source_port, char dest_ip[], int dest_port);
int m_sendto(int sockfd, char buf[], int len);
int m_recvfrom(int sockfd, char buf[], int len);
int m_close(int sockfd);
int dropMessage(float v);
void semaphore_signal(int semaphore_id);
void semaphore_wait(int semaphore_id);
void semaphore_init(int semaphore_id, int initial_value);
SOCK_INFO *shared_func();



#endif