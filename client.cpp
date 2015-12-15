
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

/*
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "RouterCommand.h"

#define DEST_IP   "192.168.8.13"
#define DEST_PORT 10007


int main()
{
    int res;
    int sockfd;
    struct sockaddr_in dest_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) 
    {
        perror("socket()");
        exit(1);
    }

    dest_addr.sin_family = AF_INET;                 
    dest_addr.sin_port = htons(DEST_PORT);          
    dest_addr.sin_addr.s_addr = inet_addr(DEST_IP); 
    bzero(&(dest_addr.sin_zero), 8);                

    res = connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));
    if (res == -1) 
    {
        perror("connect()");
        exit(1);
    }
    
    wHeadCmd *pHeadCmd = new LoginFromS_t();
    //pHeadCmd->wdServerID = 1;
    
    int len = sizeof(*pHeadCmd);
    char *buff = 0;
    *(int *)buff = len;
    buff += sizeof(int); 

    strncpy(buff,(char *)pHeadCmd,len);

    int bytes_sent = send(sockfd , buff , len + sizeof(int) , 0);
    
    close(sockfd);
}
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

const int MAXLINE = 8196;
int main()
{
    char vBuff[MAXLINE];
    pid_t pid;
    int status;

    printf("%% ");
    while(fgets(vBuff, MAXLINE,stdin) != NULL)   //ctrl-D
    {
        vBuff[MAXLINE - 1] = 0;

        if((pid = fork()) < 0)
        {
            printf("fork error: %s\n", strerror(errno));
            exit(-1);
        }
        else if(pid == 0)
        {
            //execlp(vBuff,vBuff,(char*) 0);
            //printf("buff: %s\n", vBuff);
            exit(127);
        }
        
        //parent
        if ((pid = waitpid(pid, &status, 0)) < 0)
        {
            printf("waitpid error: %s\n", strerror(errno));
            exit(-1);
        }
        printf("%% ");
    }
    return 0;
}