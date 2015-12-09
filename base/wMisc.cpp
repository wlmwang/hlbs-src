
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>		//atoi random srandom
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>	//int flock(int fd,int operation);

#include <signal.h>
#include "wType.h"
#include "wLog.h"

#include "wMisc.h"


//变为守护进程
int InitDaemon(const char *filename)
{
	//打开需要锁定的文件
	int lock_fd = open(filename, O_RDWR|O_CREAT, 0640);
	if (lock_fd < 0) 
	{
		cout << "Open lock file failed when init daemon" <<endl;
		return -1;
	}
	//独占式锁定文件，防止有相同程序的进程已经启动
	int ret = flock(lock_fd, LOCK_EX | LOCK_NB);
	if (ret < 0) 
	{
		cout << "Lock file failed, server is already running" <<endl;
		return -1;
	}

	//获取当前的目录信息
	char dir_path[256] = {0};
	getcwd(dir_path, sizeof(dir_path));

	pid_t pid;

	//第一次fork
	if ((pid = fork()) != 0) exit(0);

	//将该进程设置一个新的进程组的首进程
	setsid();

	//忽略以下信号
	signal(SIGINT,  SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	//再次fork
	if ((pid = fork()) != 0) exit(0);

	if (chdir(dir_path)) 
	{
		cout << "Can not change run dir to "<< dir_path << ", init daemon failed" << strerror(errno) <<endl;
		return -1;		
	}
	umask(0);

	return 0;
}

void itoa(unsigned long val, char *buf, unsigned radix) 
{
	char *p; /* pointer to traverse string */ 
	char *firstdig; /* pointer to first digit */ 
	char temp; /* temp char */ 
	unsigned digval; /* value of digit */ 

	p = buf; 
	firstdig = p; /* save pointer to first digit */ 

	do { 
		digval = (unsigned) (val % radix); 
		val /= radix; /* get next digit */ 

		/* convert to ascii and store */ 
		if (digval > 9) 
			*p++ = (char ) (digval - 10 + 'a'); /* a letter */ 
		else 
			*p++ = (char ) (digval + '0'); /* a digit */ 
	} while (val > 0); 

	/* We now have the digit of the number in the buffer, but in reverse 
	   order. Thus we reverse them now. */ 

	*p-- = '\0'; /* terminate string; p points to last digit */ 

	do { 
		temp = *p; 
		*p = *firstdig; 
		*firstdig = temp; /* swap *p and *firstdig */ 
		--p; 
		++firstdig; /* advance to next two digits */ 
	} while (firstdig < p); /* repeat until halfway */ 
}
