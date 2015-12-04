#include <strings.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/file.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "log.h"
#include "server_ctrl.h"
#include "login_server.h"
#include "tinyxml.h"
#include "login_config.h"

char *CServerCtrl::mShmPtr = NULL;
template<> CServerCtrl *CSingleton<CServerCtrl>::mSingletonPtr = NULL;
template<> CLoginConfig *CSingleton<CLoginConfig>::mSingletonPtr = NULL;
char *CLoginServer::mOutBufferPtr = NULL;
char *CLoginServer::mInBufferPtr = NULL;

//--------------------------------------------------
// 对于SIGUSR1信号的处理，用于服务器关闭
//-------------------------------------------------- 
void Sigusr1Handle(int sig_val)
{
	CServerCtrl::GetSingletonPtr()->mStatus = SERVER_STATUS_QUIT;
	signal(SIGUSR1, Sigusr1Handle);
}

//--------------------------------------------------
// 对于SIGUSR2信号的处理，暂时没什么用，保留
//-------------------------------------------------- 
void Sigusr2Handle(int sig_val)
{
	signal(SIGUSR2, Sigusr2Handle);
}

//--------------------------------------------------
// 让服务器变为守护进程
//-------------------------------------------------- 
void InitDaemon()
{
	// 打开需要锁定的文件
	int lock_fd = open("./loginserver.lock", O_RDWR|O_CREAT, 0640);
	if (lock_fd < 0) 
	{
		printf("open lock file failed when loginserver init\n");
		exit(1);
	}

	// 独占式锁定文件，防止有相同程序的进程已经启动
	int ret = flock(lock_fd, LOCK_EX | LOCK_NB);
	if (ret < 0) 
	{
		printf("lock file failed, loginserver is already running\n");
		exit(1);
	}

	// 获取当前的目录信息
	char dir_path[ 256 ] = { 0 };
	getcwd(dir_path, sizeof(dir_path));

	pid_t pid;

	// 第一次fork
	if ((pid = fork()) != 0) exit(0);

	// 将该进程设置一个新的进程组的首进程
	setsid();

	// 忽略以下信号
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	// 再次fork
	if ((pid = fork()) != 0) exit(0);

	if (chdir(dir_path)) 
	{
		printf("can not change run dir to %s, init failed:%s\n", dir_path, strerror(errno));
		exit(1);
	}

	umask(0);
}

//--------------------------------------------------
// 获取需要申请的服务器的内存大小
//-------------------------------------------------- 
size_t GetAllocMemorySize()
{
	return CServerCtrl::GetMemorySize() + CLoginServer::GetMemorySize() + CLoginConfig::GetMemorySize();
}

//--------------------------------------------------
// 向gate获取消息队列
//-------------------------------------------------- 
char *AttachShareMemory(const char *filename, int pipe_id, size_t size)
{
	// 把需要申请共享内存的key值申请出来
	key_t key = ftok(filename, pipe_id);
	if (key < 0) 
	{
		printf("initailize memory (ftok) failed: %s\n", strerror(errno));
		exit(1);
	}

	// 尝试获取
	int shm_id = shmget(key, size, 0666);
	if( shm_id < 0 ) 
	{
		printf("attach to share memory failed: %s\n", strerror(errno));
		exit(1);
	}

	return (char *)shmat(shm_id, NULL, 0);
}

//--------------------------------------------------
// 向系统申请共享内存
//-------------------------------------------------- 
char *CreateShareMemory(const char *filename, int pipe_id, size_t size)
{
	LOG_DEBUG("default", "try to alloc %lld bytes of share memory", size);

	// 把需要申请共享内存的key值申请出来
	key_t key = ftok(filename, pipe_id);
	if (key < 0) 
	{
		printf("initailize memory (ftok) failed: %s", strerror(errno));
		exit(1);
	}

	// 申请共享内存
	int shm_id = shmget(key, size, IPC_CREAT|IPC_EXCL|0666);
	// 如果申请内存失败
	if (shm_id < 0) 
	{
		if (errno != EEXIST) 
		{
			LOG_ERROR("default", "Alloc share memory failed: %s", strerror(errno));
			exit(1);
		}

		LOG_DEBUG("default", "share memory is exist now, try to attach it");

		// 如果该内存已经被申请，则申请访问控制它
		shm_id = shmget(key, size, 0666);

		// 如果失败
		if (shm_id < 0) 
		{
			LOG_DEBUG("default", "attach to share memory failed: %s, try to touch it", strerror(errno));
			
			// 猜测是否是该内存大小太小，先获取内存ID
			shm_id = shmget(key, 0, 0666);
			
			// 如果失败，则无法操作该内存，只能退出
			if (shm_id < 0) 
			{
				LOG_ERROR("default", "touch to share memory failed: %s", strerror(errno));
				exit(1);
			}
			else 
			{
				LOG_DEBUG("default", "remove the exist share memory %d", shm_id);

				// 如果成功，则先删除原内存
				if (shmctl(shm_id, IPC_RMID, NULL) < 0) 
				{
					LOG_ERROR("default", "remove share memory failed: %s", strerror(errno));
					exit(1);
				}

				// 再次申请该ID的内存
				shm_id = shmget(key, size, IPC_CREAT|IPC_EXCL|0666);
				if (shm_id < 0) 
				{
					LOG_ERROR("default", "alloc share memory failed again: %s", strerror(errno));
					exit(1);
				}
			}
		}
		else
		{
			LOG_DEBUG("default", "attach to share memory succeed");
		}
	}

	LOG_INFO("default", "alloc %lld bytes of share memory succeed", size);

	return (char *)shmat(shm_id, NULL, 0);
}

//--------------------------------------------------
// 初始化服务器内存
//-------------------------------------------------- 
void InitailizeMemory(int init_flag)
{
	// 如果没有loginpipe就先创建一个
	system("touch loginpipe");

	size_t alloc_memory_size = GetAllocMemorySize();

	// 申请共享内存
	char *mem_ptr = CreateShareMemory("loginpipe", 'z', alloc_memory_size);
	if (mem_ptr == NULL)
	{
		printf("create share memory failed\n");
		exit(1);
	}

	// 尝试连接消息队列
	const char *p_filename = "./gatepipe";
	CLoginServer::mInBufferPtr = AttachShareMemory(p_filename, 'o', MSG_QUEUE_LEN);
	CLoginServer::mOutBufferPtr = AttachShareMemory(p_filename, 'i', MSG_QUEUE_LEN);
	if( CLoginServer::mInBufferPtr == NULL
			|| CLoginServer::mOutBufferPtr == NULL ) 
	{
		printf("attach to msg queue failed\n");
		exit(1);
	}

	// 创建server_ctrl单例
	CServerCtrl::mShmPtr = mem_ptr;
	CServerCtrl *p_server_ctrl = new CServerCtrl(init_flag, alloc_memory_size);

	if( p_server_ctrl == NULL )
	{
		printf("new server_ctrl failed\n");
		exit(1);
	}

	// 配置文件的实体
	CLoginConfig *p_config = new CLoginConfig();

	if( p_config == NULL )
	{
		printf("new CLoginConfig failed\n");
		exit(1);
	}

	TiXmlDocument stDoc;
	stDoc.LoadFile("../config/loginserver.xml");
	TiXmlElement *pElement = NULL;
	TiXmlElement *pChildElm = NULL;
	TiXmlElement *pRoot = stDoc.FirstChildElement();

	// 读取需要连接的IP地址
	pElement = pRoot->FirstChildElement("IPADDRESS");
	if( pElement != NULL )
	{
		const char *szIPAdress;
		const char *szPort;
		pChildElm = pElement->FirstChildElement("LOGIN_SERVER");
		if( pChildElm != NULL )
		{
			szIPAdress = pChildElm->Attribute("IP");
			szPort = pChildElm->Attribute("PORT");
			memcpy(CLoginConfig::GetSingletonPtr()->mLoginIPAddress, szIPAdress, MAX_IP_ADDR_LEN);
			CLoginConfig::GetSingletonPtr()->mLoginPort = atoi(szPort);
		}
		else
		{
			printf("get login ip and port from config file failed\n");
			exit(1);
		}
	}
	else
	{
		printf("get connect ip and port from config file failed\n");
		exit(1);
	}
	
	// 读取需要日志配置
	pElement = pRoot->FirstChildElement("LOG");
	if( pElement != NULL )
	{
		for( pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement() )
		{
			const char *szKey = pChildElm->Attribute("KEY");
			const char *szFile = pChildElm->Attribute("FILE");
			const char *szLevel = pChildElm->Attribute("LEVEL");

			// 初始化日志
			INIT_ROLLINGFILE_LOG(szKey, szFile, (LogLevel)atoi(szLevel), 10*1024*1024, 20);
		}
	}
	else
	{
		printf("get log config from config file failed\n");
		exit(1);
	}
}

//--------------------------------------------------
// 主函数
//-------------------------------------------------- 
int main(int argc, const char *argv[])
{
	// 是否启动为守护进程
	int nDaemonFlag = 0;
	// 这个参数用于以后服务器在崩溃后被拉起
	int nInitFlag = 1;
	
	for (int i = 1; i < argc; i++) 
	{
		if (strcasecmp((const char *)argv[i], "-V") == 0) 
		{
#ifdef _DEBUG_
			printf("loginserver debug version\n");
#else
			printf("loginserver release version\n");
#endif
			exit(0);
		}

		if (strcasecmp((const char *)argv[i], "-D") == 0) 
		{
			nDaemonFlag = 1;
		}

		if (strcasecmp((const char *)argv[i], "-R") == 0)
		{
			nInitFlag = 0;
		}
	}

	if (nDaemonFlag) 
	{
		// 初始化守护进程
		InitDaemon();
	}

	// 初始化内存，读取配置文件，初始化日志
	InitailizeMemory(nInitFlag);

	// 创建真正的服务器实体
	CLoginServer *pServer = new CLoginServer;

	if( pServer == NULL ) 
	{
		LOG_ERROR("default", "new CLoginServer failed");
		exit(1);
	}

	// 准备工作
	pServer->PrepareToRun();

	printf("--------------------------------------------------\n");
	printf("		  login server startup OK!\n");
	printf("--------------------------------------------------\n");

	// 消息处理函数的注册
	signal(SIGUSR1, Sigusr1Handle);
	signal(SIGUSR2, Sigusr2Handle);

	// 服务器开始运行
	pServer->Run();

	LOG_SHUTDOWN_ALL;

	return 0;
}
