
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <sys/file.h>	//int flock(int fd,int operation);
#include <signal.h>
#include "wCore.h"
#include "wLog.h"

#include "wMisc.h"

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

vector<string> Split(string sStr, string sPattern, bool bRepeat)  
{  
    string::size_type iPos, iNextPos;
    vector<string> vResult;
    sStr += sPattern;  
    int iSize = sStr.size();
  
    for(int i = 0; i < iSize; i++)  
    {  
        iPos = iNextPos = sStr.find(sPattern, i);
        if(iPos < iSize)
        {
            string s = sStr.substr(i, iPos - i);
            vResult.push_back(s);
            i = iPos + sPattern.size() - 1;
        }
    }
    return vResult;
}

u_char *Cpystrn(u_char *dst, u_char *src, size_t n)
{
    if (n == 0) 
    {
        return dst;
    }

    while (--n) 
    {
        *dst = *src;
        if (*dst == '\0') 
        {
            return dst;
        }
        dst++;
        src++;
    }

    *dst = '\0';
    return dst;
}

int Gcd(int a, int b)
{
	if (a < b)
	{
		int tmp = a;
		a = b;
		b = tmp;
	}

	if (b == 0)
	{
		return a;
	}
	else
	{
		return Gcd(b, a % b);
	}
}

int Ngcd(int *arr, int n)
{
	if (n == 1)  return *arr;
	return Gcd(arr[n-1], Ngcd(arr, n-1));
}

int InitDaemon(const char *filename)
{
	//打开需要锁定的文件
	int lock_fd = open(filename, O_RDWR|O_CREAT, 0640);
	if (lock_fd < 0) 
	{
		LOG_ERROR(ELOG_KEY, "[startup] Open lock file failed when init daemon");
		return -1;
	}
	//独占式锁定文件，防止有相同程序的进程已经启动
	int ret = flock(lock_fd, LOCK_EX | LOCK_NB);
	if (ret < 0) 
	{
		LOG_ERROR(ELOG_KEY, "[startup] Lock file failed, server is already running");
		return -1;
	}

	//获取当前的目录信息
	char dir_path[256] = {0};
	getcwd(dir_path, sizeof(dir_path));

	pid_t pid;

	//第一次fork
	if ((pid = fork()) != 0) exit(0);

	setsid();

	//忽略以下信号
	wSignal stSig(SIG_IGN);
	stSig.AddSigno(SIGINT);
	stSig.AddSigno(SIGHUP);
	stSig.AddSigno(SIGQUIT);
	stSig.AddSigno(SIGTERM);
	stSig.AddSigno(SIGCHLD);
	stSig.AddSigno(SIGPIPE);
	stSig.AddSigno(SIGTTIN);
	stSig.AddSigno(SIGTTOU);

	//再次fork
	if ((pid = fork()) != 0) exit(0);

	if (chdir(dir_path)) 
	{
		int err = errno;
		LOG_ERROR(ELOG_KEY, "[startup] Can not change run dir to %s , init daemon failed: %s", dir_path, strerror(err));
		return -1;		
	}
	umask(0);
	
	unlink(filename);
	return 0;
}
