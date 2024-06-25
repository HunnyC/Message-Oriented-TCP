#include "msocket.h"

SOCK_INFO *shared_func()
{
    key_t key;
    int shmid;
    SOCK_INFO *shared_info;

    // Generate the same key using ftok
    if ((key = ftok(SHM_KEY_PATH, SHM_KEY_ID)) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Get the identifier for the shared memory segment
    if ((shmid = shmget(key, sizeof(SOCK_INFO), 0666)) == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach the shared memory segment to our data structure
    if ((shared_info = (SOCK_INFO *)shmat(shmid, NULL, 0)) == (SOCK_INFO *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    return shared_info;
}
// Function to initialize semaphore
void semaphore_init(int semaphore_id, int initial_value)
{

    if (semctl(semaphore_id, 0, SETVAL, initial_value) == -1)
    {
        perror("Semaphore initialization failed");
        exit(EXIT_FAILURE);
    }
}

// Function to perform semaphore wait (P) operation
void semaphore_wait(int semaphore_id)
{
    struct sembuf semaphore_operation;
    semaphore_operation.sem_num = 0; // Index of semaphore in semaphore set
    semaphore_operation.sem_op = -1; // Decrement semaphore value by 1
    semaphore_operation.sem_flg = 0; // Undo the operation if the process terminates
    if (semop(semaphore_id, &semaphore_operation, 1) == -1)
    {
        perror("Semaphore wait (P) operation failed");
        exit(EXIT_FAILURE);
    }
}
// Function to perform semaphore signal (V) operation
void semaphore_signal(int semaphore_id)
{
    struct sembuf semaphore_operation;
    semaphore_operation.sem_num = 0; // Index of semaphore in semaphore set
    semaphore_operation.sem_op = 1;  // Increment semaphore value by 1
    semaphore_operation.sem_flg = 0; // Undo the operation if the process terminates
    if (semop(semaphore_id, &semaphore_operation, 1) == -1)
    {
        perror("Semaphore signal (V) operation failed");
        exit(EXIT_FAILURE);
    }
}
// Implement MTP functions here

int dropMessage(float v)
{
    // Seed the random number generator
    srand(time(NULL));

    // Generate a random number between 0 and 1
    float randomNum = (float)rand() / RAND_MAX;

    // Check if the generated number is less than p
    if (randomNum < v)
    {
        return 1; // Message dropped
    }
    else
    {
        return 0; // Message not dropped
    }
}

// Implement other MTP functions as specified in the assignment
int m_socket(int domain, int type, int flag)
{

    if (type != SOCK_MTP)
    {
        perror("socket type was wrong");
        exit(1);
    }

    key_t key1;
    int shmid1;
    MTPSocket *sockM;

    // Generate the same key using ftok
    if ((key1 = ftok(MTP_KEY_PATH, MTP_KEY_ID)) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Get the identifier for the shared memory segment
    if ((shmid1 = shmget(key1, sizeof(MTPSocket) * MAX_MTP_SOCKETS, 0666)) == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach the shared memory segment to our data structure
    if ((sockM = (MTPSocket *)shmat(shmid1, NULL, 0)) == (MTPSocket *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    key_t sem_key1, sem_key2;
    if ((sem_key1 = ftok(SHM_KEY_PATH, SEM_KEY1_ID)) == -1)
    {
        perror("ftok for semaphore1");
        exit(EXIT_FAILURE);
    }

    if ((sem_key2 = ftok(SHM_KEY_PATH, SEM_KEY2_ID)) == -1)
    {
        perror("ftok for semaphore2");
        exit(EXIT_FAILURE);
    }

    int semaphore1;
    if ((semaphore1 = semget(sem_key1, 1, 0666)) == -1)
    {
        perror("semget for semaphore1");
        exit(EXIT_FAILURE);
    }

    int semaphore2;
    if ((semaphore2 = semget(sem_key2, 1, 0666)) == -1)
    {
        perror("semget for semaphore1");
        exit(EXIT_FAILURE);
    }
    key_t sem_key3;
    if ((sem_key3 = ftok(SHM_KEY_PATH, SEM_KEY3_ID)) == -1)
    {
        perror("ftok for semaphore3");
        exit(EXIT_FAILURE);
    }

    int semaphore3;
    if ((semaphore3 = semget(sem_key3, 1, 0666)) == -1)
    {
        perror("semget for semaphore3");
        exit(EXIT_FAILURE);
    }

    semaphore_wait(semaphore3);

    int i = 0;
    for (i = 0; i < MAX_MTP_SOCKETS; ++i)
    {
        if (sockM[i].free)
        {
            sockM[i].free = false;

            break;
        }
    }
    if (i == MAX_MTP_SOCKETS)
    {
        errno = ENOBUFS; // Update the global error variable
        semaphore_signal(semaphore3);
        return -1;
    }

    SOCK_INFO *shared_info = shared_func();
    printf("Socket creation initiated...\n");

    semaphore_signal(semaphore1); // Perform signal operation

    printf("Waiting for socket creation to complete...\n");

    semaphore_wait(semaphore2); // Perform wait operation

    printf("Socket creation completed.\n");

    // Check SOCK_INFO for error after completion
    int r = shared_info->sock_id;
    errno = shared_info->errorno;
    shared_info->sock_id = 0;
    strcpy(shared_info->IP, "");
    shared_info->port = 0;
    shared_info->errorno = 0;
    if (r == -1)
    {

        perror("Socket creation error");
        semaphore_signal(semaphore3);
        return -1;
    }
    else
    {
        printf("Socket created successfully: %d\n", shared_info->sock_id);
    }

    sockM[i].UDPsocID = r;
    sockM[i].pid = getpid();
    printf("Udp socket id : %d \n", r);
    semaphore_signal(semaphore3);
    return i;
    // pthread_mutex_lock(&mutex);
}

int m_bind(int sockfd, char source_ip[], int source_port, char dest_ip[], int dest_port)
{
    key_t key1;
    int shmid1;
    MTPSocket *sockM;

    // Generate the same key using ftok
    if ((key1 = ftok(MTP_KEY_PATH, MTP_KEY_ID)) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Get the identifier for the shared memory segment
    if ((shmid1 = shmget(key1, sizeof(MTPSocket) * MAX_MTP_SOCKETS, 0666)) == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach the shared memory segment to our data structure
    if ((sockM = (MTPSocket *)shmat(shmid1, NULL, 0)) == (MTPSocket *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    key_t sem_key1, sem_key2;
    if ((sem_key1 = ftok(SHM_KEY_PATH, SEM_KEY1_ID)) == -1)
    {
        perror("ftok for semaphore1");
        exit(EXIT_FAILURE);
    }

    if ((sem_key2 = ftok(SHM_KEY_PATH, SEM_KEY2_ID)) == -1)
    {
        perror("ftok for semaphore2");
        exit(EXIT_FAILURE);
    }

    int semaphore1;
    if ((semaphore1 = semget(sem_key1, 1, 0666)) == -1)
    {
        perror("semget for semaphore1");
        exit(EXIT_FAILURE);
    }

    int semaphore2;
    if ((semaphore2 = semget(sem_key2, 1, 0666)) == -1)
    {
        perror("semget for semaphore1");
        exit(EXIT_FAILURE);
    }
    key_t sem_key3;
    if ((sem_key3 = ftok(SHM_KEY_PATH, SEM_KEY3_ID)) == -1)
    {
        perror("ftok for semaphore3");
        exit(EXIT_FAILURE);
    }

    int semaphore3;
    if ((semaphore3 = semget(sem_key3, 1, 0666)) == -1)
    {
        perror("semget for semaphore3");
        exit(EXIT_FAILURE);
    }

    SOCK_INFO *shared_info = shared_func();
    printf("Socket Binding initiated...\n");
    // setting ip and port in sock info
    semaphore_wait(semaphore3);
    if (sockfd == -1 || sockfd >= MAX_MTP_SOCKETS || sockM[sockfd].free != 0 || sockM[sockfd].des_port != 0)
    {
        printf("Socket out of bound or not valid arguments \n");
        semaphore_signal(semaphore3);
        return -1;
    }

    shared_info->sock_id = sockM[sockfd].UDPsocID;
    strcpy(shared_info->IP, source_ip);
    shared_info->port = source_port;

    semaphore_signal(semaphore1); // Perform signal operation

    printf("Waiting for socket binding to complete...\n");

    semaphore_wait(semaphore2); // Perform wait operation
    printf("debugging \n");
    int r = shared_info->sock_id;
    errno = shared_info->errorno;
    shared_info->sock_id = 0;
    strcpy(shared_info->IP, "");
    shared_info->port = 0;
    shared_info->errorno = 0;
    if (r == -1)
    {
        perror("Binding Fail\n");
        semaphore_signal(semaphore3);

        return -1;
    }
    else
    {
        sockM[sockfd].des_port = dest_port;
        strcpy(sockM[sockfd].des_IP, dest_ip);
        sockM[sockfd].source_port = source_port;
        strcpy(sockM[sockfd].source_IP, source_ip);

        printf("Socket created successfully \n");
    }
    semaphore_signal(semaphore3);
    return 1;
}

int m_close(int sockfd)
{
    key_t key1;
    int shmid1;
    MTPSocket *sockM;

    // Generate the same key using ftok
    if ((key1 = ftok(MTP_KEY_PATH, MTP_KEY_ID)) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Get the identifier for the shared memory segment
    if ((shmid1 = shmget(key1, sizeof(MTPSocket) * MAX_MTP_SOCKETS, 0666)) == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach the shared memory segment to our data structure
    if ((sockM = (MTPSocket *)shmat(shmid1, NULL, 0)) == (MTPSocket *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    key_t sem_key1, sem_key2;
    if ((sem_key1 = ftok(SHM_KEY_PATH, SEM_KEY1_ID)) == -1)
    {
        perror("ftok for semaphore1");
        exit(EXIT_FAILURE);
    }

    if ((sem_key2 = ftok(SHM_KEY_PATH, SEM_KEY2_ID)) == -1)
    {
        perror("ftok for semaphore2");
        exit(EXIT_FAILURE);
    }

    int semaphore1;
    if ((semaphore1 = semget(sem_key1, 1, 0666)) == -1)
    {
        perror("semget for semaphore1");
        exit(EXIT_FAILURE);
    }

    int semaphore2;
    if ((semaphore2 = semget(sem_key2, 1, 0666)) == -1)
    {
        perror("semget for semaphore1");
        exit(EXIT_FAILURE);
    }
    key_t sem_key3;
    if ((sem_key3 = ftok(SHM_KEY_PATH, SEM_KEY3_ID)) == -1)
    {
        perror("ftok for semaphore3");
        exit(EXIT_FAILURE);
    }

    int semaphore3;
    if ((semaphore3 = semget(sem_key3, 1, 0666)) == -1)
    {
        perror("semget for semaphore3");
        exit(EXIT_FAILURE);
    }

    SOCK_INFO *shared_info = shared_func();
    printf("Socket Closing initiated...\n");

    semaphore_wait(semaphore3);
    if (sockfd == -1 || sockfd >= MAX_MTP_SOCKETS || sockM[sockfd].free != 0)
    {
        printf("Invalid argument in close\n");
        semaphore_signal(semaphore3);
        return -1;
    }

    // setting ip and port in sock info
    shared_info->sock_id = sockM[sockfd].UDPsocID;
    strcpy(shared_info->IP, "");
    shared_info->port = 0;

    semaphore_signal(semaphore1); // Perform signal operation

    printf("Waiting for socket closing to complete...\n");

    semaphore_wait(semaphore2); // Perform wait operation

    int r = shared_info->sock_id;
    errno = shared_info->errorno;
    shared_info->sock_id = 0;
    strcpy(shared_info->IP, "");
    shared_info->port = 0;
    shared_info->errorno = 0;
    if (r == -1)
    {
        perror("Closing Fail\n");
        semaphore_signal(semaphore3);

        return -1;
    }
    else
    {

        printf("Socket closed successfully \n");
    }

    int i = sockfd;
    sockM[i].free = true;
    sockM[i].pid = 0;
    sockM[i].UDPsocID = -1;
    memset(sockM[i].des_IP, 0, sizeof(sockM[i].des_IP));
    sockM[i].des_port = 0;
    memset(sockM[i].source_IP, 0, sizeof(sockM[i].source_IP));
    sockM[i].source_port = 0;
    memset(sockM[i].sbuf, 0, sizeof(sockM[i].sbuf));
    memset(sockM[i].rbuf, 0, sizeof(sockM[i].rbuf));
    sockM[i].nospace = false;
    strcpy(sockM[i].ack_num, "");
    sockM[i].sendNospace = false;
    sockM[i].l_send = 0;
    sockM[i].f_send = 0;
    sockM[i].l_recv = 0;
    sockM[i].f_recv = 0;
    sockM[i].current_send = 0;
    sockM[i].current_recv = 0;
    sockM[i].timeout = 0;
    sockM[i].send_ack_flag = 0;

    sockM[i].seq_num = 1;
    sockM[i].left_swnd = 0;
    sockM[i].right_swnd = 5;
    sockM[i].curr_size_swnd = 5;
    sockM[i].left_rwnd = 1;
    sockM[i].right_rwnd = 6;
    sockM[i].curr_size_rwnd = 5;
    // int last_seq_num = 0;
    sockM[i].left_seq = 15;

    // Initialize other fields as needed...
    semaphore_signal(semaphore3);
    return 1;
}

int m_sendto(int sockfd, char buf[], int len)
{
    key_t key1;
    int shmid1;
    MTPSocket *sockM;

    // Generate the same key using ftok
    if ((key1 = ftok(MTP_KEY_PATH, MTP_KEY_ID)) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Get the identifier for the shared memory segment
    if ((shmid1 = shmget(key1, sizeof(MTPSocket) * MAX_MTP_SOCKETS, 0666)) == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach the shared memory segment to our data structure
    if ((sockM = (MTPSocket *)shmat(shmid1, NULL, 0)) == (MTPSocket *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    key_t sem_key3, sem_key4;
    if ((sem_key3 = ftok(SHM_KEY_PATH, SEM_KEY3_ID)) == -1)
    {
        perror("ftok for semaphore1");
        exit(EXIT_FAILURE);
    }

    if ((sem_key4 = ftok(SHM_KEY_PATH, SEM_KEY4_ID)) == -1)
    {
        perror("ftok for semaphore2");
        exit(EXIT_FAILURE);
    }

    int semaphore3;
    if ((semaphore3 = semget(sem_key3, 1, 0666)) == -1)
    {
        perror("semget for semaphore3");
        exit(EXIT_FAILURE);
    }

    int semaphore4;
    if ((semaphore4 = semget(sem_key4, 1, 0666)) == -1)
    {
        perror("semget for semaphore4");
        exit(EXIT_FAILURE);
    }

    // printf("here in msocket.c\n");
    // printf("Semaphore 3 value %d",semctl(semaphore3,0,GETVAL,0));
    semaphore_wait(semaphore3);
    // printf("here in after msocket.c\n");
    // printf("Semaphore 3 value %d",semctl(semaphore3,0,GETVAL,0));
    //////checking if socket is bind or not
    if ((sockM[sockfd].free != 0) || sockM[sockfd].source_port == 0 || sockM[sockfd].des_port == 0)
    {
        errno = ENOTCONN;
        perror("Unable to send\n");
        semaphore_signal(semaphore3);
        return -1;
    }

    int k = -1;
    //////checking dest source is bind with this socket or not for it find source id for that socket

    for (int i = 0; i < MAX_MTP_SOCKETS; i++)
    {
        if (sockM[i].des_port == sockM[sockfd].source_port && strcmp(sockM[i].des_IP, sockM[sockfd].source_IP) == 0)
        {
            k = i;
        }
    }

    // return -1 if dest is not bind with socket
    /////////////////////////////////////////////////////
    if (k == -1)
    {
        errno = ENOTCONN;
        perror("Unable to send\n");
        semaphore_signal(semaphore3);
        return -1;
    }

    if (sockM[sockfd].current_send == 10 )
    {
        errno = ENOBUFS;
        perror("Unable to send\n");
        semaphore_signal(semaphore3);
        return -1;
    }

    strcpy(sockM[sockfd].sbuf[sockM[sockfd].l_send], "");
    strncpy(sockM[sockfd].sbuf[sockM[sockfd].l_send], buf, len);
    sockM[sockfd].sbuf[sockM[sockfd].l_send][len] = '\0';

    char str[4];
    int nus = sockM[sockfd].seq_num;
    if (nus < 10)
        strcpy(str, "0");
    else
        strcpy(str, "");
    char st[3];
    sprintf(st, "%d", sockM[sockfd].seq_num);
    strcat(str, st);
    strcat(sockM[sockfd].sbuf[sockM[sockfd].l_send], str);
    sockM[sockfd].seq_num++;
    if (sockM[sockfd].seq_num > 15)
        sockM[sockfd].seq_num %= 15;

    printf("( %ld ) From msocket.c\n", strlen(sockM[sockfd].sbuf[sockM[sockfd].l_send]));
    sockM[sockfd].l_send += 1;
    sockM[sockfd].l_send %= 10;
    sockM[sockfd].current_send++;
    /////signal to semaphore in s thread

    semaphore_signal(semaphore3);

    /////wait for signal from s thread
    return EXIT_SUCCESS;
}

int m_recvfrom(int sockfd, char buf[], int len)
{
    key_t key1;
    int shmid1;
    MTPSocket *sockM;
    key_t sem_key3, sem_key4;
    if ((sem_key3 = ftok(SHM_KEY_PATH, SEM_KEY3_ID)) == -1)
    {
        perror("ftok for semaphore1");
        exit(EXIT_FAILURE);
    }

    if ((sem_key4 = ftok(SHM_KEY_PATH, SEM_KEY4_ID)) == -1)
    {
        perror("ftok for semaphore2");
        exit(EXIT_FAILURE);
    }

    int semaphore3;
    if ((semaphore3 = semget(sem_key3, 1, 0666)) == -1)
    {
        perror("semget for semaphore3");
        exit(EXIT_FAILURE);
    }

    int semaphore4;
    if ((semaphore4 = semget(sem_key4, 1, 0666)) == -1)
    {
        perror("semget for semaphore4");
        exit(EXIT_FAILURE);
    }

    // Generate the same key using ftok
    if ((key1 = ftok(MTP_KEY_PATH, MTP_KEY_ID)) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Get the identifier for the shared memory segment
    if ((shmid1 = shmget(key1, sizeof(MTPSocket) * MAX_MTP_SOCKETS, 0666)) == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach the shared memory segment to our data structure
    if ((sockM = (MTPSocket *)shmat(shmid1, NULL, 0)) == (MTPSocket *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    //////checking if socket is bind or not

    semaphore_wait(semaphore3);
    if ((sockM[sockfd].free != 0) || sockM[sockfd].source_port == 0)
    {
        errno = ENOTCONN;
        perror("Receive not possible socket is not bind or sockfd not exitst\n");
        semaphore_signal(semaphore3);
        return -1;
    }

    if (sockM[sockfd].current_recv == 0)
    {
        errno = ENOMSG;
        printf("No message in recieve buffer \n ");
        semaphore_signal(semaphore3);
        return -1;
    }

    /////signal to semaphore in r thread
    int si = strlen(sockM[sockfd].rbuf[sockM[sockfd].f_recv]);

    if (len > si)
    {
        len = si;
    }

    // strncpy(buf, sockM[sockfd].rbuf[sockM[sockfd].f_recv],si);

    strcpy(buf, sockM[sockfd].rbuf[sockM[sockfd].f_recv]);

    printf("In m_recv (  %d )\n",  si);
    strcpy(sockM[sockfd].rbuf[sockM[sockfd].f_recv], "");
    sockM[sockfd].f_recv += 1;
    sockM[sockfd].f_recv %= 5;
    int vt = sockM[sockfd].right_rwnd;
    vt++;
    if (vt > 15)
    {
        vt %= 15;
    }
    sockM[sockfd].right_rwnd = vt;

    sockM[sockfd].curr_size_rwnd++;

    if (sockM[sockfd].current_recv != 0)
        sockM[sockfd].current_recv--;

    semaphore_signal(semaphore3);

    /////wait for signal from s thread
    return EXIT_SUCCESS;
}
