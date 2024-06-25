// include all the headers
#include "msocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
int main(int argc, char *argv[])
{

    if (argc <= 3)
    {
        printf("give port for source,dest  ports , file name please \n ");
        return 0;
    }

    int sock_mtpfd = m_socket(AF_INET, SOCK_MTP, 0);

    printf("Created %d id\n", sock_mtpfd);

    if (m_bind(sock_mtpfd, "127.0.0.1", atoi(argv[1]), "127.0.0.1", atoi(argv[2])) > 0)
    {
        printf("success\n");
    }

  
    int file = open(argv[3], O_RDWR|O_CREAT , 0666);
    if (file == -1)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    sleep(4);
    char buffer[BUFFER_SIZE];
    strcpy(buffer,"");
    int bytes_read;
    int m;
    int no_of_send_count=0;
    
    while ((bytes_read = read(file,buffer, BUFFER_SIZE) > 0))
    {
        while(m_sendto(sock_mtpfd, buffer,strlen(buffer)) == -1)
        {
            perror("Error sending data");
            // exit(EXIT_FAILURE);
            sleep(3);
            
        }

        strcpy(buffer,"");
       // scanf("%d",&m);



    }
    close(file);

    

    return 0;
}