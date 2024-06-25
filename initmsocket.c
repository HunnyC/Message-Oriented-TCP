#include "msocket.h"
#include <pthread.h>
#include <sys/select.h>
#include "msocket.h"
#include <pthread.h>
#include <sys/select.h>

#define SEM_OP1_INDEX 0
#define SEM_OP2_INDEX 1

int semaphore1, semaphore2;
int semaphore3, semaphore4;

int cnt_transmission=0;
int shmid, shmid1;
SOCK_INFO *shared_info;
MTPSocket *sockM;
pthread_t tid_R, tid_S, tid_G;
// signal handler CTRL+C
pthread_mutex_t sr_mutex = PTHREAD_MUTEX_INITIALIZER;

/// ////////////////////////  initializing mutex ///////////
void init_mutex()
{
    pthread_mutex_trylock(&sr_mutex);
    pthread_mutex_unlock(&sr_mutex);
}
void end()
{
   
    printf("\nDeleting Semaphores..........\n");
    semctl(semaphore1, IPC_RMID, 0);
    semctl(semaphore2, IPC_RMID, 0);
    semctl(semaphore3, IPC_RMID, 0);
    semctl(semaphore4, IPC_RMID, 0);
    printf("Deleting shared memory SOCK_INFO and shared_info.......\n");
    // Detach the shared memory segment
    if (shmdt(shared_info) == -1)
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    // Detach the shared memory segment
    if (shmdt(sockM) == -1)
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        exit(1);
    }
    if (shmctl(shmid1, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        exit(1);
    }
    pthread_mutex_destroy(&sr_mutex);
    exit(0);
}
// Function to initialize semaphore

void *garbage_collector(void *arg)
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
        printf("Fail here ... \n");

        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach the shared memory segment to our data structure
    if ((sockM = (MTPSocket *)shmat(shmid1, NULL, 0)) == (MTPSocket *)-1)
    {
        perror("shmat");
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

    printf("Garbage Collecting Thread G  initiated...\n");
    // setting ip and port in sock info
    while (1)
    {   
        pthread_mutex_lock(&sr_mutex);
        semaphore_wait(semaphore3);
        int i = 0;
        for (i = 0; i < MAX_MTP_SOCKETS; i++)
        {    
            int v=0;
            int jd=0;
            if(sockM[i].free == false)
            {
                for(int j=0;j<MAX_MTP_SOCKETS;j++)
                {
                    if(sockM[i].source_port==sockM[j].des_port)
                    {   
                        jd=j;
                        break;
                    }
                   
                }
            }
            // for(int j=0;j< 10;j++)
            // {
            //     if(strcmp(sockM[i].sbuf[j],"")==0)v++;
            //     if(sockM[i].free == false)
            //     {
            //         if(strcmp(sockM[jd].sbuf[j],"")==0)v++;
            //     }


            // }
            if ((sockM[i].free == true && kill(sockM[i].pid, 0) != 0 ) ||(sockM[i].free == false && kill(sockM[i].pid, 0) != 0 && kill(sockM[jd].pid, 0) != 0))
            {
                printf("Socket Process has been terminated\n");
                close(sockM[i].UDPsocID);
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

                sockM[i].left_seq = 15;


                //////////////////////////
                close(sockM[jd].UDPsocID);
                sockM[jd].free = true;
                sockM[jd].pid = 0;
                sockM[jd].UDPsocID = -1;
                memset(sockM[jd].des_IP, 0, sizeof(sockM[jd].des_IP));
                sockM[jd].des_port = 0;
                memset(sockM[jd].source_IP, 0, sizeof(sockM[jd].source_IP));
                sockM[jd].source_port = 0;
                
                memset(sockM[jd].sbuf, 0, sizeof(sockM[jd].sbuf));
                memset(sockM[jd].rbuf, 0, sizeof(sockM[jd].rbuf));
                sockM[jd].nospace = false;
                strcpy(sockM[jd].ack_num, "");
                sockM[jd].sendNospace = false;
                sockM[jd].l_send = 0;
                sockM[jd].f_send = 0;
                sockM[jd].l_recv = 0;
                sockM[jd].f_recv = 0;
                sockM[jd].current_send = 0;
                sockM[jd].current_recv = 0;
                sockM[jd].timeout = 0;
                sockM[jd].send_ack_flag = 0;

                sockM[jd].seq_num = 1;
                sockM[jd].left_swnd = 0;
                sockM[jd].right_swnd = 5;
                sockM[jd].curr_size_swnd = 5;
                sockM[jd].left_rwnd = 1;
                sockM[jd].right_rwnd = 6;
                sockM[jd].curr_size_rwnd = 5;

                sockM[jd].left_seq = 15;
            }
        }
        semaphore_signal(semaphore3);
        pthread_mutex_unlock(&sr_mutex);
    }
}

// Thread S Function
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *thread_S(void *arg)
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

    while (1)
    {
        // printf("Semaphore 3 value %d in s thread\n", semctl(semaphore3, 0, GETVAL, 0));
        pthread_mutex_lock(&sr_mutex);
        // printf("Here in init :\n");
        printf("in S:\n");
        semaphore_wait(semaphore3);
        // printf("Here in after semaphore3 init :\n");

        for (int i = 0; i < MAX_MTP_SOCKETS; i++)
        {

            if (sockM[i].free == false && sockM[i].current_send != 0)
            {
                // printf("INIT_S_THREAD----------------->>\n");

                if (sockM[i].timeout <= 0)
                {

                    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                    // printf("Message --> starting sending ............\n");
                    sockM[i].timeout = T; // set timeout
                    struct sockaddr_in addr;
                    memset(&addr, 0, sizeof(addr));
                    addr.sin_family = AF_INET;
                    addr.sin_port = htons(sockM[i].des_port);
                    inet_aton(sockM[i].des_IP, &addr.sin_addr);

                    int l = sockM[i].left_swnd;
                    int r = sockM[i].right_swnd;
                    int size = sockM[i].curr_size_swnd;
                    if (r >= l)
                    {

                        if (r - l < size)
                        {
                            r += (size - (r - l));
                            r %= 10;
                            sockM[i].right_swnd = r;
                        }
                    }
                    else
                    {
                        int t = 10 + r;

                        if (t - l < size)
                        {
                            t += (size - (t - l));
                            t %= 10;
                            r = t;
                            sockM[i].right_swnd = r;
                        }
                    }
                    printf("(S : L %d , R %d )\n", l, r);
                    printf("INIT: <S-THREAD> &&&& NEW LEFT_swnd %d AND NEW RIGHT_swnd %d Send Window size %d Current Messages %d \n", l, r, size, sockM[i].current_send);
                    if (r < l)
                    {
                        r = 10 + r;
                    }

                    char str[2];
                    char buf[1024];
                    /*
                         head at the end of frame message
                         message frame type ---->   <message>< 2 sequence-num><1bit to identify b/w message (1) and ack (0) >
                         ack  frame type -----> <seq numb><size of rwnd ><1bit to identify duplicate (1)or simple ack (0)><1 bit to identify b/w message and ack>
                    */

                    int k = l;
                    for (int j = 0; j < size; j++)
                    {
                        if (l >= r)
                            break;
                        if (strcmp(sockM[i].sbuf[k], "") == 0)
                            break;
                        strcpy(buf, "");
                        strcpy(buf, sockM[i].sbuf[k]);
                        // sprintf(str, "%d", k);
                        // strcat(buf, str);
                        strcat(buf, "1");

                        if (sendto(sockM[i].UDPsocID, buf, strlen(buf) + 1, 0, (struct sockaddr *)&addr, sizeof(addr)) >= 0)
                        {
                            //printf("INIT<S-THREAD>::Sending..... Message frame = %s   length = %ld to Udp recvfrom\n", buf, strlen(buf));
                            l++;
                            k++;
                            k %= 10;
                            cnt_transmission++;
                            printf("----\nTransmission count %d\n---",cnt_transmission);
                        }
                        else
                        {
                            perror("Send\n");
                        }
                    }
                }
                else
                {
                    // printf("decreasing__Timeout T/2\n");
                    sockM[i].timeout -= T / 2;
                }

                ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            }
        }
        semaphore_signal(semaphore3);
        pthread_mutex_unlock(&sr_mutex);
        sleep(T / 2);
    }

    pthread_exit(NULL);
}
int nfs(char st[])
{

    char num[3];
    int lenth = strlen(st);
    num[0] = st[lenth - 2];
    num[1] = st[lenth - 1];
    num[2] = '\0';
    int nu = atoi(num);

    return nu;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Thread R Function
void *thread_R(void *arg)
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

    fd_set read_fds, write_fds;
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    int socks[25];
    for (int i = 0; i < 25; i++)
        socks[i] = 0;
    while (1)
    {
        // while(pthread_mutex_trylock(&sr_mutex)!=0);

        pthread_mutex_lock(&sr_mutex);

        //  printf("Semaphore 3 value %d in 1 r\n",semctl(semaphore3,0,GETVAL,0));
        semaphore_wait(semaphore3);

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        timeout.tv_sec = 3;
        int t = -1;
        int mt = 26;
        int mxt = -1;
        for (int i = 0; i < MAX_MTP_SOCKETS; i++)
        {

            if (sockM[i].free == false && sockM[i].des_port != 0 && strcmp(sockM[i].des_IP, "") != 0)
            {

                FD_SET(sockM[i].UDPsocID, &read_fds);
                FD_SET(sockM[i].UDPsocID, &write_fds);
                if (sockM[i].UDPsocID > t)
                {
                    t = sockM[i].UDPsocID;
                }
                if (mxt < i)
                    mxt = t;

                if (i < mt)
                    mt = i;
                socks[i] = 1;
                /////////////////////////////////////////////// sending dup ack for nospace///////////
                if (sockM[i].nospace == 1 && sockM[i].current_recv != 5)
                {
                    char buf[5];
                    char v[3];
                    int g = sockM[i].left_seq;
                    if (g < 10)
                    {
                        strcpy(buf, "0");
                    }
                    sprintf(v, "%d", g);
                    strcat(buf, v);
                    strcpy(v, "");
                    sprintf(v, "%d", 5 - sockM[i].current_recv);

                    strcat(buf, v);
                    strcat(buf, "10");

                    sockM[i].nospace = 0;
                    struct sockaddr_in addr;
                    memset(&addr, 0, sizeof(addr));
                    addr.sin_family = AF_INET;
                    addr.sin_port = htons(sockM[i].des_port);
                    inet_aton(sockM[i].des_IP, &addr.sin_addr);

                    if (sendto(sockM[i].UDPsocID, buf, strlen(buf) + 1, 0, (struct sockaddr *)&addr, sizeof(addr)) >= 0)
                    {
                        printf("~~~~~~~~~~~~~~~~~INIT<R-THREAD>****Sending..... Dup Ack  = %s   length = %ld to Udp recvfrom\n", buf, strlen(buf));
                    }
                }
            }
        }
        semaphore_signal(semaphore3);
        //  printf("Semaphore 3 value %d in  after signal  1 r\n",semctl(semaphore3,0,GETVAL,0));
        pthread_mutex_unlock(&sr_mutex);

        if (t >= 0)
        {
            int n = select(t + 1, &read_fds, &write_fds, NULL, &timeout);

            pthread_mutex_lock(&sr_mutex);
            // printf("Semaphore 3 value %d in   2 r\n",semctl(semaphore3,0,GETVAL,0));
            semaphore_wait(semaphore3);
            if (n > 0)
            {
                //////////////////////////////////////// checking in read_fds //////////////////////////////

                for (int i = 0; i < MAX_MTP_SOCKETS; i++)
                {

                    struct sockaddr_in addr;
                    socklen_t len = sizeof(addr);

                    char buf[1024];

                    char buff[1024];
                    char seqb[3];
                    strcpy(seqb, "");

                    strcpy(buff, "");
                    strcpy(buf, "");
                    int flag = 1;
                    if (FD_ISSET(sockM[i].UDPsocID, &read_fds))
                    {

                        recvfrom(sockM[i].UDPsocID, buf, 2000, 0, (struct sockaddr *)&addr, &len);

                        printf("INIT:<R-THREAD> !!!! Received MTP frame in UDP socket Frame  Len = %ld\n",strlen(buf));
                        if (dropMessage(p) == 1)
                        {
                            flag = 0;
                        }
                        if (buf[strlen(buf) - 1] == '0' && buf[strlen(buf) - 2] == '1')
                            flag = 1;

                        if (flag == 0)
                        {
                            printf("@@@@@@@@@ Droping @@@@@@@@@@ \n");
                        }
                        else
                        {

                            if (buf[strlen(buf) - 1] == '1')
                            {
                               printf("INIT:<R-THREAD> !!!! Received MESSAGE frame in UDP socket Frame Len = %ld\n", strlen(buf));

                                strncpy(seqb, buf + strlen(buf) - 3, 2);
                                seqb[2] = '\0';

                                int sno = atoi(seqb);
                                int check_in_window = 0;
                                strncpy(buff, buf, strlen(buf) - 3);
                                buff[strlen(buf) - 3] = '\0';
                                int l = sockM[i].left_rwnd;
                                int r = sockM[i].right_rwnd;
                                if (r < l)
                                    r = 15 + r;
                                int it = l;
                                int dup_msg = 0;
                                for (int j = 0; j < sockM[i].curr_size_rwnd; j++)
                                {
                                    int vt = (it - 1) % 5; // to find position in recv buf
                                    int cmp = it;
                                    if (it > 15)
                                        cmp = it % 15;
                                    if (sno == cmp)
                                    {
                                        check_in_window = 1;
                                        if (strcmp(sockM[i].rbuf[vt], "") != 0)
                                        {
                                            dup_msg = 1;
                                        }
                                        else

                                        {
                                            strcpy(sockM[i].rbuf[vt], buff);
                                        }
                                        break;
                                    }
                                    it++;
                                    if (it >= r)
                                        break;
                                }

                                // message seq not exist in recv wnd
                                if (check_in_window == 1)
                                {
                                    if (dup_msg == 1)
                                    {
                                        // Sending ack for duplicate message
                                        int lst = sockM[i].left_seq;
                                        char Ackst[10] = ""; // ack
                                        char temp[3];

                                        strcpy(temp, "");
                                        sprintf(temp, "%d", lst);
                                        if (lst < 10)
                                        {
                                            strcat(Ackst, "0");
                                            strcat(Ackst, temp);
                                        }
                                        else
                                        {
                                            strcat(Ackst, temp);
                                        }
                                        strcpy(temp, "");
                                        sprintf(temp, "%d", sockM[i].curr_size_rwnd);
                                        strcat(Ackst, temp);
                                        strcat(Ackst, "00");
                                        if (sockM[i].current_recv == 5)
                                        {
                                            sockM[i].nospace = 1;
                                        }

                                        sendto(sockM[i].UDPsocID, Ackst, strlen(Ackst) + 1, 0, (struct sockaddr *)&addr, len);
                                        printf("(L= %d , R= %d )\n", sockM[i].left_rwnd, sockM[i].right_rwnd);
                                        printf("INIT<R-THREAD> **Duplicate Message**.....   Ack  = %s  Last_recv = %d length = %ld to Udp recvfrom\n", Ackst, sockM[i].left_seq, strlen(Ackst));
                                    }
                                    else
                                    {
                                        l = sockM[i].left_rwnd;
                                        r = sockM[i].right_rwnd;
                                        if (r < l)
                                            r = 15 + r;
                                        it = l;
                                        int j;
                                        int k = 0;
                                        for (j = 0; j < sockM[i].curr_size_rwnd; j++)
                                        {
                                            int vt = (it - 1) % 5;
                                            if (strcmp(sockM[i].rbuf[vt], "") == 0)
                                            {
                                                break;
                                            }
                                            k++;
                                            sockM[i].l_recv++;
                                            sockM[i].l_recv %= 5;
                                            sockM[i].current_recv++;
                                            it++;
                                            if (it >= r)
                                                break;
                                        }
                                        int out_order = 0;
                                        if (k == 0)
                                            out_order = 1;
                                        if (out_order == 0)
                                        {

                                            l = it;
                                            if (l > 15)
                                                l %= 15;
                                            sockM[i].left_rwnd = l;
                                            sockM[i].curr_size_rwnd -= k;
                                            it--;
                                            if (it > 15)
                                                it %= 15;
                                            sockM[i].left_seq = it;
                                            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                            int lst = sockM[i].left_seq;
                                            char Ackst[10] = ""; // ack
                                            char temp[3];
                                            strcpy(temp, "");
                                            sprintf(temp, "%d", lst);
                                            if (lst < 10)
                                            {
                                                strcat(Ackst, "0");
                                                strcat(Ackst, temp);
                                            }
                                            else
                                            {
                                                strcat(Ackst, temp);
                                            }
                                            strcpy(temp, "");
                                            sprintf(temp, "%d", sockM[i].curr_size_rwnd);
                                            strcat(Ackst, temp);
                                            strcat(Ackst, "00");
                                            if (sockM[i].current_recv == 5)
                                            {
                                                sockM[i].nospace = 1;
                                            }

                                            sendto(sockM[i].UDPsocID, Ackst, strlen(Ackst) + 1, 0, (struct sockaddr *)&addr, len);
                                            printf("(L= %d , R= %d )\n", sockM[i].left_rwnd, sockM[i].right_rwnd);
                                            printf("INIT<R-THREAD>Sending.....   Ack  = %s  Last_recv = %d length = %ld to Udp recvfrom\n", Ackst, sockM[i].left_seq, strlen(Ackst));

                                            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                        }
                                        else
                                        {
                                            /// handling outof order
                                            int lst = sockM[i].left_seq;
                                            char Ackst[10] = ""; // ack
                                            char temp[3];
                                            strcpy(temp, "");
                                            sprintf(temp, "%d", lst);
                                            if (lst < 10)
                                            {
                                                strcat(Ackst, "0");
                                                strcat(Ackst, temp);
                                            }
                                            else
                                            {
                                                strcat(Ackst, temp);
                                            }
                                            strcpy(temp, "");
                                            sprintf(temp, "%d", sockM[i].curr_size_rwnd);
                                            strcat(Ackst, temp);
                                            strcat(Ackst, "00");
                                            if (sockM[i].current_recv == 5)
                                            {
                                                sockM[i].nospace = 1;
                                            }

                                            sendto(sockM[i].UDPsocID, Ackst, strlen(Ackst) + 1, 0, (struct sockaddr *)&addr, len);
                                            printf("(L= %d , R= %d )\n", sockM[i].left_rwnd, sockM[i].right_rwnd);
                                            printf("INIT<R-THREAD> **Out of order** .....   Ack  = %s  Last_recv = %d length = %ld to Udp recvfrom\n", Ackst, sockM[i].left_seq, strlen(Ackst));
                                        }
                                    }
                                }
                            }

                            else if (buf[strlen(buf) - 1] == '0')
                            {

                                printf("INIT:<R-THREAD> ^^^^^^^ Received ACK frame in UDP socket Frame = ACK%s Len = %ld\n", buf, strlen(buf));
                                int le = strlen(buf);
                                if (buf[le - 2] == '0')
                                {
                                    // Normal ack
                                    // len of buf
                                    int r_ws = buf[le - 3] - '0'; // window size
                                    char seqn[3];
                                    seqn[0] = buf[0];
                                    seqn[1] = buf[1];
                                    seqn[2] = '\0';
                                    int sqn = atoi(seqn); // seq num last receive in order

                                    sockM[i].curr_size_swnd = r_ws; // changing the window size of sender
                                    sockM[i].current_send--;
                                    int mt = sockM[i].left_swnd;

                                    // int nu = nfs(sockM[i].sbuf[mt]);
                                    int qw = 0;
                                    for (int j = mt; nfs(sockM[i].sbuf[mt]) != sqn; j++)
                                    {
                                        strcpy(sockM[i].sbuf[mt], "");
                                        mt++;
                                        mt %= 10;
                                        qw++;
                                        sockM[i].current_send--;
                                    }
                                    strcpy(sockM[i].sbuf[mt], "");
                                    mt++;
                                    mt %= 10;
                                    sockM[i].left_swnd = mt;
                                    int r = sockM[i].right_swnd;
                                    // if(r>=mt)
                                    // {
                                    //     if(r-mt<r_ws)
                                    //     {

                                    //     }

                                    // }
                                    // else
                                    // {

                                    // }

                                    printf("INIT: <R-THREAD> &&&& NEW WINDOW SIZE %d LEFT_swnd %d  current_send_messages %d \n", r_ws, mt, sockM[i].current_send);
                                }
                                else
                                {
                                    int r_ws = buf[le - 3] - '0'; // window size
                                    char seqn[3];
                                    seqn[0] = buf[0];
                                    seqn[1] = buf[1];
                                    seqn[2] = '\0';
                                    int sqn = atoi(seqn); // seq num last receive in order
                                    // int v = (sqn) % 10;
                                    sockM[i].curr_size_swnd = r_ws;

                                    printf("INIT: <R-THREAD> &&&& After dup ack (NOSPACE) NEW WINDOW SIZE %d  current_send_messages %d \n", r_ws, sockM[i].current_send);
                                }
                                // sockM[i].current_send
                            }
                        }
                        // else
                        // {
                        //     printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&Detection : %s len %ld",buf,strlen(buf));
                        // }
                    }
                    // else if (FD_ISSET(sockM[i].UDPsocID, &write_fds))
                    // {
                    // }
                }
            }
            semaphore_signal(semaphore3);
            //  printf("Semaphore 3 value %d in  after signal  2 r\n",semctl(semaphore3,0,GETVAL,0));
            pthread_mutex_unlock(&sr_mutex);
        }
    }

    pthread_exit(NULL);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{

 
        init_mutex();
        
        
        //////////////////////////////////////semaphore creation/////////////////////////
        key_t sem_key1, sem_key2;
        key_t sem_key3, sem_key4;
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
        if ((sem_key3 = ftok(SHM_KEY_PATH, SEM_KEY3_ID)) == -1)
        {
            perror("ftok for semaphore3");
            exit(EXIT_FAILURE);
        }

        if ((sem_key4 = ftok(SHM_KEY_PATH, SEM_KEY4_ID)) == -1)
        {
            perror("ftok for semaphore4");
            exit(EXIT_FAILURE);
        }

        // Create semaphore 1

        if ((semaphore1 = semget(sem_key1, 1, IPC_CREAT | 0666)) == -1)
        {
            perror("semget for semaphore1");
            exit(EXIT_FAILURE);
        }

        // Create semaphore 2

        if ((semaphore2 = semget(sem_key2, 1, IPC_CREAT | 0666)) == -1)
        {
            perror("semget for semaphore2");
            exit(EXIT_FAILURE);
        }
        // Create semaphore 3
        if ((semaphore3 = semget(sem_key3, 1, IPC_CREAT | 0666)) == -1)
        {
            perror("semget for semaphore3");
            exit(EXIT_FAILURE);
        }

        // Create semaphore 4

        if ((semaphore4 = semget(sem_key4, 1, IPC_CREAT | 0666)) == -1)
        {
            perror("semget for semaphore4");
            exit(EXIT_FAILURE);
        }

        semaphore_init(semaphore1, 0); // Initializing semaphore value to 0
        semaphore_init(semaphore2, 0); // Initializing semaphore value to 0
        semaphore_init(semaphore3, 1); // Initializing semaphore value to 1
        semaphore_init(semaphore4, 0); // Initializing semaphore value to 0
        printf("Semaphores created and initialized successfully.\n");
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Initialize MTP sockets, bind, etc.
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        key_t key, key1;

        // Generate key using ftok
        if ((key = ftok(SHM_KEY_PATH, SHM_KEY_ID)) == -1)
        {
            perror("ftok");
            exit(EXIT_FAILURE);
        }

        // Create the shared memory segment
        if ((shmid = shmget(key, sizeof(SOCK_INFO), IPC_CREAT | 0666)) == -1)
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
        // Initialize the shared structure
        shared_info->sock_id = 0;
        strcpy(shared_info->IP, "");
        shared_info->port = 0;
        shared_info->errorno = 0;

        printf("Shared memory segment created and initialized successfully.\n");

        // Generate key using ftok
        if ((key1 = ftok(MTP_KEY_PATH, MTP_KEY_ID)) == -1)
        {
            perror("ftok");
            exit(EXIT_FAILURE);
        }

        // Create the shared memory segment
        if ((shmid1 = shmget(key1, sizeof(MTPSocket) * MAX_MTP_SOCKETS, IPC_CREAT | 0666)) == -1)
        {
            perror("shmget");
            exit(EXIT_FAILURE);
        }

        // Attach the shared memory segment to our MTPSocket array
        if ((sockM = (MTPSocket *)shmat(shmid1, NULL, 0)) == (MTPSocket *)-1)
        {
            perror("shmat");
            exit(EXIT_FAILURE);
        }

        // Initialize the shared MTPSocket array
        for (int i = 0; i < MAX_MTP_SOCKETS; ++i)
        {
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
            // sockM[i].ack_num = "05";
            strcpy(sockM[i].ack_num, "05");
            sockM[i].sendNospace = false;
            sockM[i].l_send = 0;
            sockM[i].seq_num = 1;
            sockM[i].f_send = 0;
            sockM[i].l_recv = 0;
            sockM[i].f_recv = 0;
            sockM[i].current_send = 0;
            sockM[i].current_recv = 0;
            sockM[i].timeout = 0;
            sockM[i].send_ack_flag = 0;
            // int last_seq_num = 0;

            // for (int j = 0; j < 5; j++)
            // {
            //     sockM[i].swnd[j] = j + 1;
            //     sockM[i].rwnd[j] = j + 1;
            // }

            sockM[i].left_swnd = 0;
            sockM[i].right_swnd = 5;
            sockM[i].curr_size_swnd = 5;
            sockM[i].left_rwnd = 1;
            sockM[i].right_rwnd = 6;
            sockM[i].curr_size_rwnd = 5;
            sockM[i].left_seq = 15;
            // int last_seq_num = 0;
            //  Initialize other fields as needed...
        }

        pthread_create(&tid_R, NULL, thread_R, NULL);
        pthread_create(&tid_S, NULL, thread_S, NULL);
        pthread_create(&tid_G,NULL,garbage_collector,NULL);

        signal(SIGINT, end);

        while (1)
        {

            printf("waiting for signal from msocket.c\n");
            // Wait for a signal on semaphore 1
            semaphore_wait(semaphore1); // Perform wait operation
                                        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if (shared_info->sock_id == 0 && shared_info->errorno == 0)
            {
                // SOCK_INFO indicates a m_socket call, proceed to create a UDP socket
                int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
                if (sockfd == -1)
                {
                    // If error, update SOCK_INFO
                    shared_info->sock_id = -1;
                    shared_info->errorno = errno;
                }
                else
                {
                    // If successful, update SOCK_INFO with the socket id
                    shared_info->sock_id = sockfd;
                    int optval = 1;
                    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
                    {
                        perror("ERROR setting socket option");
                        exit(1);
                    }
                    printf("SOcket is created\n");
                }
            }
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            else if (shared_info->port == 0 && shared_info->errorno == 0)
            {

                if (close(shared_info->sock_id) >= 0)
                {
                    printf("Close sucess\n");
                }
                else
                {
                    shared_info->sock_id = -1;
                    shared_info->errorno = errno;
                    perror("close");
                }
            }
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            else if (shared_info->port != 0 && shared_info->errorno == 0)
            {

                // SOCK_INFO indicates a m_bind call, proceed to make a bind() call
                struct sockaddr_in addr;
                memset(&addr, 0, sizeof(addr));
                addr.sin_family = AF_INET;
                addr.sin_port = htons(shared_info->port);
                inet_aton(shared_info->IP, &addr.sin_addr);

                int result = bind(shared_info->sock_id, (struct sockaddr *)&addr, sizeof(addr));

                if (result == -1)
                {
                    // If error, update SOCK_INFO
                    shared_info->sock_id = -1;
                    shared_info->errorno = errno;
                }
                else
                {
                    printf("Bind Successfull\n");
                }
            }

            // Signal semaphore 2 to indicate completion
            semaphore_signal(semaphore2); // Perform signal operation
        }
    

    return 0;
}