// include all the headers
#include "msocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "msocket.h"
#define BUFFER_SIZE 1024
int main(int argc, char *argv[])
{

    if (argc <= 3)
    {
        printf("give port for source ,dest ports,file name please\n ");
        return 0;
    }
    int sock_mtpfd = m_socket(AF_INET, SOCK_MTP, 0);

    printf("Created mtpsocket %d id\n", sock_mtpfd);

    if (m_bind(sock_mtpfd, "127.0.0.1", atoi(argv[1]), "127.0.0.1", atoi(argv[2])) > 0)
    {
        printf("success\n");
    }

    char buffer[BUFFER_SIZE];
    strcpy(buffer, "");
    // Open a new file for writing
  int file = open(argv[3], O_RDWR|O_CREAT , 0666);
    if (file == -1)
    {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // Receive data and write to the file
    
    int bytes_received;
    int t=0;
    int r=0;
    while (1) {
        bytes_received = m_recvfrom(sock_mtpfd, buffer, 1024);
        if (bytes_received == -1) {
            //perror("Error receiving data");
            r++;
            sleep(2);
         
        } else if (buffer[strlen(buffer)-1] == EOF) {
            // End-of-file received, break the loop
             printf("i break here\n");
            break;
        } else {
          

           r=0;
           
           // buffer[strlen(buffer)-1]='\0';
            buffer[strlen(buffer)]='\0';
            char buf[1024]="";
            int i=0;
            
            printf("%s %ld\n",buf,strlen(buffer));

            write(file,buffer, strlen(buffer));
            if(buffer[strlen(buffer)-2]=='*')break;
          
            // printf("hi\n");
            
           

        }

         if(buffer[strlen(buffer)-1]=='*')break;
         if(r>=10)break;
        
        t++;
        if(t>1e9)break;
         strcpy(buffer,"");

    }


    // Close the file and the socket
    close(file);



    //  int g;
    // while(1)
    // {

    //     // printf("Enter Message to send: ");
    //     // scanf("%s",buff);

    //     // if (m_sendto(sock_mtpfd, buff, strlen(buff)) >= 0)
    //     // {
    //     //     printf("!!!!!!!!!!!Sended success %s\n", buff);
    //     // }
    //     // printf("Enter 1 to get message or 0 to continue to send : ");
    //     scanf("%d",&g);
    //     if(1)
    //     {
    //         strcpy(buff,"");
    //         if(m_recvfrom(sock_mtpfd,buff,100)>=0)
    //         {
    //             printf("------->Have Received value %s \n",buff);
    //         }
    //         else

    //         {
    //             perror("Error in Received\n");
    //         }

    //     }
    //   //  if(g !=1 && g != 0) break;

    // }

    //   m_close(sock_mtpfd);

    return 0;
}