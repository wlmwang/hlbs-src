
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>		//atoi random srandom
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>	//int flock(int fd,int operation);

#include <signal.h>

#include "wType.h"
#include "wLog.h"
#include "wFunction.h"

/**
 * 创建共享内存
 * @param  filename [共享内存基于的文件名]
 * @param  pipe_id  [ftok附加参数]
 * @param  size     [共享内存大小]
 * @return          [共享内存在进程中映射地址]
 */
char *CreateShareMemory(const char *filename, int pipe_id, size_t size)
{
	LOG_DEBUG("default", "Try to alloc %lld bytes of share memory", size);

	//把需要申请共享内存的key值申请出来
	key_t key = ftok(filename, pipe_id);
	if (key < 0) 
	{
		printf("Initailize memory (ftok) failed: %s", strerror(errno));
		exit(1);
	}

	//申请共享内存
	int shm_id = shmget(key, size, IPC_CREAT|IPC_EXCL|0666);

	//如果申请内存失败
	if (shm_id < 0) 
	{
		if (errno != EEXIST) 
		{
			LOG_ERROR("default", "Alloc share memory failed: %s", strerror(errno));
			exit(1);
		}

		LOG_DEBUG("default", "Share memory is exist now, try to attach it");

		//如果该内存已经被申请，则申请访问控制它
		shm_id = shmget(key, size, 0666);

		//如果失败
		if (shm_id < 0) 
		{
			LOG_DEBUG("default", "Attach to share memory failed: %s, try to touch it", strerror(errno));
			
			//猜测是否是该内存大小太小，先获取内存ID
			shm_id = shmget(key, 0, 0666);
			
			//如果失败，则无法操作该内存，只能退出
			if (shm_id < 0) 
			{
				LOG_ERROR("default", "Touch to share memory failed: %s", strerror(errno));
				exit(1);
			}
			else 
			{
				LOG_DEBUG("default", "Remove the exist share memory %d", shm_id);

				//如果成功，则先删除原内存
				if (shmctl(shm_id, IPC_RMID, NULL) < 0) 
				{
					LOG_ERROR("default", "Remove share memory failed: %s", strerror(errno));
					exit(1);
				}

				//再次申请该ID的内存
				shm_id = shmget(key, size, IPC_CREAT|IPC_EXCL|0666);
				if (shm_id < 0) 
				{
					LOG_ERROR("default", "Alloc share memory failed again: %s", strerror(errno));
					exit(1);
				}
			}
		}
		else
		{
			LOG_DEBUG("default", "Attach to share memory succeed");
		}
	}

	LOG_INFO("default", "Alloc %lld bytes of share memory succeed", size);

	return (char *)shmat(shm_id, NULL, 0);
}

/**
 * 变为守护进程
 * @param  filename [互斥文件名]
 */
void InitDaemon(const char *filename)
{
	//打开需要锁定的文件
	int lock_fd = open(filename, O_RDWR|O_CREAT, 0640);
	if (lock_fd < 0) 
	{
		printf("Open lock file failed when init daemon\n");
		exit(1);
	}
	//独占式锁定文件，防止有相同程序的进程已经启动
	int ret = flock(lock_fd, LOCK_EX | LOCK_NB);
	if (ret < 0) 
	{
		printf("Lock file failed, server is already running\n");
		exit(1);
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
		printf("Can not change run dir to %s, init daemon failed:%s\n", dir_path, strerror(errno));
		exit(1);
	}

	umask(0);
}
