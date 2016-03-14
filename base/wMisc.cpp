
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

	//将该进程设置一个新的进程组的首进程
	setsid();

	//忽略以下信号
	/*
	signal(SIGINT,  SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	*/

	//再次fork
	if ((pid = fork()) != 0) exit(0);

	if (chdir(dir_path)) 
	{
		int err = errno;
		LOG_ERROR(ELOG_KEY, "[startup] Can not change run dir to %s , init daemon failed: %s", dir_path, strerror(err));
		return -1;		
	}
	umask(0);
	
	//TODO.
	unlink(filename);
	return 0;
}

/*
void GetProcessStatus(void)
{
    int		status;
    char	*process;
    pid_t	pid;
    int		err;
    int		i;
    int		one;

    one = 0;
    while (true) 
	{
        pid = waitpid(-1, &status, WNOHANG);

        if (pid == 0) 
		{
            return;
        }

        if (pid == -1) 
		{
            err = errno;
            if (err == EINTR) 
			{
                continue;
            }

            if (err == ECHILD && one) 
			{
                return;
            }

            if (err == ECHILD) 
			{
				LOG_ERROR(ELOG_KEY, "waitpid() failed: %s", strerror(err));				
                return;
            }
			
			LOG_ERROR(ELOG_KEY, "waitpid() failed: %s", strerror(err));	
            return;
        }
		
        one = 1;
        process = "unknown process";

        for (i = 0; i < ngx_last_process; i++) 
		{
            if (ngx_processes[i].pid == pid) {
                ngx_processes[i].status = status;
                ngx_processes[i].exited = 1;
                process = ngx_processes[i].name;
                break;
            }
        }

        if (WTERMSIG(status)) 
		{
            ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0,
                          "%s %P exited on signal %d%s",
                          process, pid, WTERMSIG(status),
                          WCOREDUMP(status) ? " (core dumped)" : "");
        } 
		else 
		{
            ngx_log_error(NGX_LOG_NOTICE, ngx_cycle->log, 0,
                          "%s %P exited with code %d",
                          process, pid, WEXITSTATUS(status));
        }

        if (WEXITSTATUS(status) == 2 && ngx_processes[i].respawn) 
		{
            ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0,
                          "%s %P exited with fatal code %d "
                          "and cannot be respawned",
                          process, pid, WEXITSTATUS(status));
            ngx_processes[i].respawn = 0;
        }

        ngx_unlock_mutexes(pid);
    }
}
*/

